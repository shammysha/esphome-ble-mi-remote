"""BleMiRemote component."""

from __future__ import annotations

from typing import Final

import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import automation
from esphome.automation import maybe_simple_id
from esphome.components import binary_sensor, button, number
from esphome.components.esp32 import add_idf_component, add_idf_sdkconfig_option
from esphome.const import (
    CONF_DEVICE_CLASS,
    DEVICE_CLASS_CONNECTIVITY,
    CONF_BATTERY_LEVEL,
    CONF_CODE,
    CONF_ID,
    CONF_MANUFACTURER_ID,
    CONF_NAME,
    CONF_VALUE,
    CONF_ICON,
    CONF_DISABLED_BY_DEFAULT,
    CONF_RESTORE_MODE    
)
from esphome.core import CORE, ID
from esphome.cpp_generator import LambdaExpression, MockObj, TemplateArguments, TemplateArgsType

from .const import (
    ACTION_COMBINATION_CLASS,
    ACTION_CONNECT_WAKE_CLASS,
    ACTION_PLAIN_ADVERT_CLASS,
    ACTION_PRESS_CLASS,
    ACTION_PRINT_CLASS,
    ACTION_RELEASE_CLASS,
    ACTION_START_CLASS,
    ACTION_STOP_CLASS,
    SPECIAL_KEY,
    COMPONENT_BUTTON_CLASS,
    COMPONENT_CLASS,
    CONF_RECONNECT,
    CONF_TARGET_MAC_ADDRESS,
    CONF_TEXT,
    DOMAIN,
    NIMBLE_CPP_COMPONENT,
    NIMBLE_CPP_COMPONENT_REPO,
    NIMBLE_CPP_COMPONENT_REF
)

CODEOWNERS: Final = ["@shammysha"]
AUTO_LOAD: Final = ["binary_sensor", "button"]

ble_mi_remote_ns = cg.esphome_ns.namespace(DOMAIN)

BleMiRemote = ble_mi_remote_ns.class_(COMPONENT_CLASS, cg.PollingComponent)
BleMiRemoteButton = ble_mi_remote_ns.class_(COMPONENT_BUTTON_CLASS, cg.Component)

CONFIG_SCHEMA: Final = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(BleMiRemote),
        cv.Optional(CONF_NAME, default=COMPONENT_CLASS): cv.Length(min=1),
        cv.Optional(CONF_MANUFACTURER_ID, default=COMPONENT_CLASS): cv.Length(min=1),
        cv.Optional(CONF_BATTERY_LEVEL, default=100): cv.int_range(min=0, max=100),
        cv.Optional(CONF_RECONNECT, default=True): cv.boolean,
        cv.Optional(CONF_TARGET_MAC_ADDRESS): cv.mac_address
    }
)


async def to_code(config: dict) -> None:
    """Generate component

    :param config: dict
    """

    if not CORE.is_esp32:
        raise cv.Invalid("The component only supports ESP32.")

    var = cg.new_Pvariable(
        config[CONF_ID],
        config[CONF_NAME],
        config[CONF_MANUFACTURER_ID],
        config[CONF_BATTERY_LEVEL],
        config[CONF_RECONNECT]
    )

    await cg.register_component(var, config)

    await adding_binary_sensors(var, config)

    await adding_special_keys(var, config)

    if CONF_TARGET_MAC_ADDRESS in config:
        cg.add(var.set_target_mac(config[CONF_TARGET_MAC_ADDRESS].as_hex))

    add_idf_sdkconfig_option("CONFIG_BT_ENABLED", True)
    add_idf_sdkconfig_option("CONFIG_BT_NIMBLE_ENABLED", True)
    add_idf_sdkconfig_option("CONFIG_BT_NIMBLE_ROLE_CENTRAL", True)
    # Without this, NimBLE's own bond store (LTK/IRK) does not survive a
    # reboot - confirmed the hard way via a gapless serial capture: the box
    # DOES auto-reconnect via the HD-burst (~2s after boot), but
    # onAuthenticationComplete then reports bonded=false and the link drops
    # (reason=0x213) because our side no longer has the LTK the box already
    # remembers from the prior bonding. This is ESP-IDF's own documented
    # requirement for "reconnect without rebonding after reset/power-cycle".
    add_idf_sdkconfig_option("CONFIG_BT_NIMBLE_NVS_PERSIST", True)
    # Ruled out this session (2026-09-02), all via direct code comparison
    # against the known-working NimBLE-Arduino 1.4.0 baseline, real
    # hardware tests, or both: BT modem sleep, GATT/HID profile content,
    # connParams/timeout values, security key-distribution config (incl.
    # sm_sc - matches after our own setSecurityAuth() call), GAP service
    # characteristic set, TV-side cache (full TV reboot before re-pairing),
    # NimBLE host debug logging (genuinely silent, confirmed via live
    # serial capture, not a log-transport artifact), ATT Read Blob/offset
    # handling in ble_gatts.c (verified correct by tracing the actual
    # source).
    #
    # The legacy Arduino framework's own baked-in sdkconfig.h links
    # CONFIG_BTDM_CTRL_MODE_BTDM=1 (dual-mode controller, Classic BT + BLE,
    # even though Classic is never used) where this build defaulted to
    # CONFIG_BTDM_CTRL_MODE_BLE_ONLY - a genuinely different compiled
    # controller binary, not just a runtime value. First attempt at
    # flipping this Kconfig choice crashed real hardware in a hard
    # boot-loop (LoadProhibited inside NimBLEDevice::init()) - root-caused
    # by tracing actual ESP-IDF source, not assumption: nimble_port_init()
    # (ESP-IDF's own controller bring-up helper for IDF>=5.0) hardcodes
    # esp_bt_controller_enable(ESP_BT_MODE_BLE), which
    # esp_bt_controller_enable() (components/bt/controller/esp32/bt.c)
    # rejects outright whenever it doesn't exactly match the mode the
    # controller was actually built for - and our fork's
    # NimBLEDevice::init() never checked nimble_port_init()'s return value,
    # so the host task got spawned over never-initialized state instead of
    # a clean failure. Fixed directly in our fork (shammysha/esp-nimble-cpp,
    # commit 7319ea1): for IDF>=5.0 we now do the same controller bring-up
    # ourselves, matching bt_cfg.mode instead of hardcoding BLE, checking
    # every step. Re-enabling BTDM now that the actual blocker is fixed.
    add_idf_sdkconfig_option("CONFIG_BTDM_CTRL_MODE_BTDM", True)

    add_idf_component(name=NIMBLE_CPP_COMPONENT, repo=NIMBLE_CPP_COMPONENT_REPO, ref=NIMBLE_CPP_COMPONENT_REF)


async def adding_special_keys(var: MockObj, config: dict) -> None:
    """Adding buttons

    :param var: MockObj
    """
    
    for key in SPECIAL_KEY:
        new_key: MockObj = await button.new_button(
            {
                CONF_ID: cv.declare_id(BleMiRemoteButton)(key[CONF_ID]),
                CONF_NAME: (config[CONF_NAME] or DOMAIN.replace("_", " ")) + " " + key[CONF_NAME],
                CONF_ICON: key[CONF_ICON],
                CONF_DISABLED_BY_DEFAULT: False
            }        
        )
        cg.add(new_key.set_parent(var))

        if CONF_VALUE not in key:
            continue

        cg.add(new_key.set_value(key[CONF_VALUE]))


async def adding_binary_sensors(var: MockObj, config: dict) -> None:
    """Adding binary sensor

    :param var: MockObj
    """

    cg.add(
        var.set_state_sensor(await binary_sensor.new_binary_sensor(
            {
                CONF_ID: cv.declare_id(binary_sensor.BinarySensor)("connected"),
                CONF_NAME: (config[CONF_NAME] or DOMAIN.replace("_", " ")) + "-connected",
                CONF_DEVICE_CLASS: DEVICE_CLASS_CONNECTIVITY,
                CONF_DISABLED_BY_DEFAULT: False
            }            
        ))
    )


OPERATION_BASE_SCHEMA: Final = cv.Schema(
    {
        cv.Required(CONF_ID): cv.use_id(BleMiRemote),
    }
)

BleMiRemoteReleaseAction = ble_mi_remote_ns.class_(
    ACTION_RELEASE_CLASS, automation.Action
)


@automation.register_action(
    f"{DOMAIN}.release",
    BleMiRemoteReleaseAction,
    maybe_simple_id(OPERATION_BASE_SCHEMA),
    synchronous=True,
)
async def ble_mi_remote_release_to_code(
    config: dict, action_id: ID, template_arg: TemplateArguments, args: TemplateArgsType
) -> MockObj:
    """Action release

    :param config: dict
    :param action_id: ID
    :param template_arg: TemplateArguments
    :param args: TemplateArgsType
    :return: MockObj
    """

    paren: MockObj = await cg.get_variable(config[CONF_ID])

    return cg.new_Pvariable(action_id, template_arg, paren)


BleMiRemotePressAction = ble_mi_remote_ns.class_(ACTION_PRESS_CLASS, automation.Action)


@automation.register_action(
    f"{DOMAIN}.press",
    BleMiRemotePressAction,
    OPERATION_BASE_SCHEMA.extend(
        {
            cv.Required(CONF_CODE): cv.Any(
                cv.templatable(cv.int_),
                cv.templatable(cv.string)
            )
        }
    ),
    synchronous=True,
)

async def ble_mi_remote_press_to_code(
    config: dict, action_id: ID, template_arg: TemplateArguments, args: TemplateArgsType
) -> MockObj:
    """Action press

    :param config: dict
    :param action_id: ID
    :param template_arg: TemplateArguments
    :param args: TemplateArgsType
    :return: MockObj
    """

    paren: MockObj = await cg.get_variable(config[CONF_ID])
    var: MockObj = cg.new_Pvariable(action_id, template_arg, paren)


    template_ = await cg.templatable(config[CONF_CODE], args, cg.std_string)
    
    is_number = True;
    
    try:
        config[CONF_CODE] = int(template_)
    except:
        try:
            config[CONF_CODE] = int(template_, 16)
        except:
            is_number = False
    
    if is_number:
        cg.add(var.set_key(config[CONF_CODE]))
    else:
        template_ = template_.lower()
        for i, k in enumerate(SPECIAL_KEY):
            if k[CONF_NAME].lower() == template_:
                cg.add(var.set_special(k[CONF_VALUE]))
                break
    return var


BleMiRemoteStartAction = ble_mi_remote_ns.class_(ACTION_START_CLASS, automation.Action)


@automation.register_action(
    f"{DOMAIN}.start",
    BleMiRemoteStartAction,
    maybe_simple_id(OPERATION_BASE_SCHEMA),
    synchronous=True,    
)
async def ble_mi_remote_start_to_code(
    config: dict, action_id: ID, template_arg: TemplateArguments, args: TemplateArgsType
) -> MockObj:
    """Action start

    :param config: dict
    :param action_id: ID
    :param template_arg: TemplateArguments
    :param args: TemplateArgsType
    :return: MockObj
    """

    paren: MockObj = await cg.get_variable(config[CONF_ID])

    return cg.new_Pvariable(action_id, template_arg, paren)


BleMiRemoteStopAction = ble_mi_remote_ns.class_(ACTION_STOP_CLASS, automation.Action)


@automation.register_action(
    f"{DOMAIN}.stop",
    BleMiRemoteStopAction,
    maybe_simple_id(OPERATION_BASE_SCHEMA),
    synchronous=True,    

)
async def ble_mi_remote_stop_to_code(
    config: dict, action_id: ID, template_arg: TemplateArguments, args: TemplateArgsType
) -> MockObj:
    """Action stop

    :param config: dict
    :param action_id: ID
    :param template_arg: TemplateArguments
    :param args: TemplateArgsType
    :return: MockObj
    """

    paren: MockObj = await cg.get_variable(config[CONF_ID])

    return cg.new_Pvariable(action_id, template_arg, paren)


BleMiRemoteConnectWakeAction = ble_mi_remote_ns.class_(
    ACTION_CONNECT_WAKE_CLASS, automation.Action
)


@automation.register_action(
    f"{DOMAIN}.connect_wake",
    BleMiRemoteConnectWakeAction,
    maybe_simple_id(OPERATION_BASE_SCHEMA),
    synchronous=True,
)
async def ble_mi_remote_connect_wake_to_code(
    config: dict, action_id: ID, template_arg: TemplateArguments, args: TemplateArgsType
) -> MockObj:
    """Action connect_wake

    Attempts a direct BLE central connection to the configured
    target_mac_address, mirroring the `bluetoothctl pair <mac>` wake
    trick used by github.com/DenizOner/MiPower. Standalone action for
    testing independently of the manufacturer-data advertise sequence.

    :param config: dict
    :param action_id: ID
    :param template_arg: TemplateArguments
    :param args: TemplateArgsType
    :return: MockObj
    """

    paren: MockObj = await cg.get_variable(config[CONF_ID])

    return cg.new_Pvariable(action_id, template_arg, paren)


BleMiRemotePlainAdvertAction = ble_mi_remote_ns.class_(
    ACTION_PLAIN_ADVERT_CLASS, automation.Action
)


@automation.register_action(
    f"{DOMAIN}.plain_advert",
    BleMiRemotePlainAdvertAction,
    maybe_simple_id(OPERATION_BASE_SCHEMA),
    synchronous=True,
)
async def ble_mi_remote_plain_advert_to_code(
    config: dict, action_id: ID, template_arg: TemplateArguments, args: TemplateArgsType
) -> MockObj:
    """Action plain_advert

    Manual test/escape-hatch action: force plain undirected advertising,
    bypassing the HD-burst/retry logic in start_reconnect_advert_() - for
    A/B testing whether the box responds to bare undirected advertising on
    its own.

    :param config: dict
    :param action_id: ID
    :param template_arg: TemplateArguments
    :param args: TemplateArgsType
    :return: MockObj
    """

    paren: MockObj = await cg.get_variable(config[CONF_ID])

    return cg.new_Pvariable(action_id, template_arg, paren)
