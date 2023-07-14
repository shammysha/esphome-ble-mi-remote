"""MiRemote component."""

from __future__ import annotations

from typing import Final

import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import automation
from esphome.automation import maybe_simple_id
from esphome.components import binary_sensor, button, number
from esphome.const import (
    CONF_BATTERY_LEVEL,
    CONF_CODE,
    CONF_DELAY,
    CONF_ID,
    CONF_INITIAL_VALUE,
    CONF_MANUFACTURER_ID,
    CONF_MAX_VALUE,
    CONF_MIN_VALUE,
    CONF_NAME,
    CONF_STEP,
    CONF_TYPE,
    CONF_VALUE,
)
from esphome.core import CORE, ID
from esphome.cpp_generator import LambdaExpression, MockObj, TemplateArguments

from .const import (
    ACTION_COMBINATION_CLASS,
    ACTION_PRESS_CLASS,
    ACTION_PRINT_CLASS,
    ACTION_RELEASE_CLASS,
    ACTION_START_CLASS,
    ACTION_STOP_CLASS,
    BINARY_SENSOR_STATE,
    SPECIAL_KEY,
    COMPONENT_BUTTON_CLASS,
    COMPONENT_CLASS,
    CONF_RECONNECT,
    CONF_TEXT,
    DOMAIN,
    LIBS_ADDITIONAL
)

CODEOWNERS: Final = ["@shammysha"]
AUTO_LOAD: Final = ["binary_sensor", "button"]

mi_remote_ns = cg.esphome_ns.namespace(DOMAIN)

MiRemote = mi_remote_ns.class_(COMPONENT_CLASS, cg.PollingComponent)
MiRemoteButton = mi_remote_ns.class_(COMPONENT_BUTTON_CLASS, cg.Component)

CONFIG_SCHEMA: Final = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(MiRemote),
        cv.Optional(CONF_NAME, default=COMPONENT_CLASS): cv.Length(min=1),
        cv.Optional(CONF_MANUFACTURER_ID, default=COMPONENT_CLASS): cv.Length(min=1),
        cv.Optional(CONF_BATTERY_LEVEL, default=100): cv.int_range(min=0, max=100),
        cv.Optional(CONF_RECONNECT, default=True): cv.boolean
    }
)


async def to_code(config: dict) -> None:
    """Generate component

    :param config: dict
    """

    if not CORE.is_esp32:
        raise cv.Invalid("The component only supports ESP32.")

    if not CORE.using_arduino:
        raise cv.Invalid("The component only supports the Arduino framework.")

    var = cg.new_Pvariable(
        config[CONF_ID],
        config[CONF_NAME],
        config[CONF_MANUFACTURER_ID],
        config[CONF_BATTERY_LEVEL],
        config[CONF_RECONNECT],
    )

    await cg.register_component(var, config)

    await adding_binary_sensors(var)

    await adding_special_keys(var)

    for lib in LIBS_ADDITIONAL:  # type: ignore
        cg.add_library(*lib)


async def adding_special_keys(var: MockObj) -> None:
    """Adding buttons

    :param var: MockObj
    """

    for key in SPECIAL_KEY:
        new_key: MockObj = await button.new_button(
            key | {CONF_ID: cv.declare_id(MiRemoteButton)(key[CONF_ID])}
        )
        cg.add(new_key.set_parent(var))

        if CONF_VALUE not in key:
            continue

        cg.add(new_key.set_value(key[CONF_VALUE]))


async def adding_binary_sensors(var: MockObj) -> None:
    """Adding binary sensor

    :param var: MockObj
    """

    cg.add(
        var.set_state_sensor(await binary_sensor.new_binary_sensor(BINARY_SENSOR_STATE))
    )


OPERATION_BASE_SCHEMA: Final = cv.Schema(
    {
        cv.Required(CONF_ID): cv.use_id(MiRemote),
    }
)

MiRemoteReleaseAction = mi_remote_ns.class_(
    ACTION_RELEASE_CLASS, automation.Action
)


@automation.register_action(
    f"{DOMAIN}.release",
    MiRemoteReleaseAction,
    maybe_simple_id(OPERATION_BASE_SCHEMA),
)
async def mi_remote_release_to_code(
    config: dict, action_id: ID, template_arg: TemplateArguments, args: list
) -> MockObj:
    """Action release

    :param config: dict
    :param action_id: ID
    :param template_arg: TemplateArguments
    :param args: list
    :return: MockObj
    """

    paren: MockObj = await cg.get_variable(config[CONF_ID])

    return cg.new_Pvariable(action_id, template_arg, paren)


MiRemotePressAction = mi_remote_ns.class_(ACTION_PRESS_CLASS, automation.Action)


@automation.register_action(
    f"{DOMAIN}.press",
    MiRemotePressAction,
    OPERATION_BASE_SCHEMA.extend(
        {
            cv.Required(CONF_CODE): cv.Any(
                cv.templatable(cv.uint8_t),
                cv.templatable(cv.string)
            )
        }
    )
)

async def mi_remote_press_to_code(
    config: dict, action_id: ID, template_arg: TemplateArguments, args: list
) -> MockObj:
    """Action press

    :param config: dict
    :param action_id: ID
    :param template_arg: TemplateArguments
    :param args: list
    :return: MockObj
    """

    paren: MockObj = await cg.get_variable(config[CONF_ID])
    var: MockObj = cg.new_Pvariable(action_id, template_arg, paren)


    template_: LambdaExpression = await cg.templatable(config[CONF_CODE], args, string)
    
    is_number = true;

    try:
        config[CONF_CODE] = int(template_)
    except:
        try:
            config[CONF_CODE] = int(template_, 16)
        except:
            is_number = false
    
    if is_number:
        cg.add(var.set_key(template_))
    else:
        template_ = template_.lower()
        for i, k in enumerate(SPECIAL_KEY):
            if k[CONF_NAME].lower() == template_:
                cg.add(var.set_special(k[CONF_VALUE]))
                break
    return var


MiRemoteStartAction = mi_remote_ns.class_(ACTION_START_CLASS, automation.Action)


@automation.register_action(
    f"{DOMAIN}.start",
    MiRemoteStartAction,
    maybe_simple_id(OPERATION_BASE_SCHEMA),
)
async def mi_remote_start_to_code(
    config: dict, action_id: ID, template_arg: TemplateArguments, args: list
) -> MockObj:
    """Action start

    :param config: dict
    :param action_id: ID
    :param template_arg: TemplateArguments
    :param args: list
    :return: MockObj
    """

    paren: MockObj = await cg.get_variable(config[CONF_ID])

    return cg.new_Pvariable(action_id, template_arg, paren)


MiRemoteStopAction = mi_remote_ns.class_(ACTION_STOP_CLASS, automation.Action)


@automation.register_action(
    f"{DOMAIN}.stop",
    MiRemoteStopAction,
    maybe_simple_id(OPERATION_BASE_SCHEMA),
)
async def mi_remote_stop_to_code(
    config: dict, action_id: ID, template_arg: TemplateArguments, args: list
) -> MockObj:
    """Action stop

    :param config: dict
    :param action_id: ID
    :param template_arg: TemplateArguments
    :param args: list
    :return: MockObj
    """

    paren: MockObj = await cg.get_variable(config[CONF_ID])

    return cg.new_Pvariable(action_id, template_arg, paren)
