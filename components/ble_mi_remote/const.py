"""BleMiRemote component const."""

# pylint: disable=line-too-long

from __future__ import annotations

from typing import Final

import esphome.config_validation as cv
from esphome.components import binary_sensor
from esphome.components.number import NumberMode
from esphome.const import (
    CONF_ICON,
    CONF_ID,
    CONF_INITIAL_VALUE,
    CONF_MAX_VALUE,
    CONF_MIN_VALUE,
    CONF_MODE,
    CONF_NAME,
    CONF_OPTIMISTIC,
    CONF_RESTORE_VALUE,
    CONF_STEP,
    CONF_TYPE,
    CONF_UNIT_OF_MEASUREMENT,
    CONF_VALUE,
    UNIT_MILLISECOND,
    UNIT_PERCENT,
)

DOMAIN: Final = "ble_mi_remote"

CONF_TEXT: Final = "text"
CONF_KEYS: Final = "keys"
CONF_RECONNECT: Final = "reconnect"
CONF_BUTTONS: Final = "buttons"
CONF_USE_DEFAULT_LIBS: Final = "use_default_libs"

COMPONENT_CLASS: Final = "BleMiRemote"
COMPONENT_NUMBER_CLASS: Final = "BleMiRemoteNumber"
COMPONENT_BUTTON_CLASS: Final = "BleMiRemoteButton"

ACTION_START_CLASS: Final = "BleMiRemoteStartAction"
ACTION_STOP_CLASS: Final = "BleMiRemoteStopAction"
ACTION_PRINT_CLASS: Final = "BleMiRemotePrintAction"
ACTION_PRESS_CLASS: Final = "BleMiRemotePressAction"
ACTION_RELEASE_CLASS: Final = "BleMiRemoteReleaseAction"
ACTION_COMBINATION_CLASS: Final = "BleMiRemoteCombinationAction"

"""Libraries"""
LIBS_ADDITIONAL: Final = [
    (
        "h2zero/NimBLE-Arduino",
        "1.4.0",
        None,
    )
]

"""Special buttons"""
SPECIAL_KEY: Final = [
    {
        CONF_NAME: "Enter",
        CONF_ID: "key_enter",
        CONF_ICON: "mdi:play",
        CONF_VALUE: 0
    },{   
        CONF_NAME: "Up",
        CONF_ID: "key_up",
        CONF_ICON: "mdi:arrow-up-drop-circle-outline",
        CONF_VALUE: 1
  
    },{
        CONF_NAME: "Down",
        CONF_ID: "key_down",
        CONF_ICON: "mdi:arrow-down-drop-circle-outline",
        CONF_VALUE: 2
    },{
        CONF_NAME: "Left",
        CONF_ID: "key_left",
        CONF_ICON: "mdi:arrow-left-drop-circle-outline",
        CONF_VALUE: 3
    },{
        CONF_NAME: "Right",
        CONF_ID: "key_right",
        CONF_ICON: "mdi:arrow-right-drop-circle-outline",
        CONF_VALUE: 4
    },{        
        CONF_NAME: "Power",
        CONF_ID: "key_power",
        CONF_ICON: "mdi:power-cycle",
        CONF_VALUE: 5
    },{
        CONF_NAME: "Voice",
        CONF_ID: "key_voice",
        CONF_ICON: "mdi:microphone",
        CONF_VALUE: 6
    },{
        CONF_NAME: "Volume Up",
        CONF_ID: "key_volumeup",
        CONF_ICON: "mdi:volume-plus",
        CONF_VALUE: 7
    },{
        CONF_NAME: "Volume Down",
        CONF_ID: "key_volumedown",
        CONF_ICON: "mdi:volume-minus",
        CONF_VALUE: 8
    },{
        CONF_NAME: "Select",
        CONF_ID: "key_select",
        CONF_ICON: "mdi:form-select",
        CONF_VALUE: 9
    },{
        CONF_NAME: "Movie",
        CONF_ID: "key_movie",
        CONF_ICON: "mdi:multimedia",
        CONF_VALUE: 10
    },{
        CONF_NAME: "Green",
        CONF_ID: "key_green",
        CONF_ICON: "mdi:radiobox-marked",
        CONF_VALUE: 11
    },{
        CONF_NAME: "Blue",
        CONF_ID: "key_blue",
        CONF_ICON: "mdz:radiobox-marked",
        CONF_VALUE: 12
    },{
        CONF_NAME: "MI HOME",
        CONF_ID: "key_mihome",
        CONF_ICON: "mdi:alpha-m-circle-outline",
        CONF_VALUE: 13
    },{
        CONF_NAME: "Email",
        CONF_ID: "key_email",
        CONF_ICON: "mdi:email-outline",
        CONF_VALUE: 14
    },{
        CONF_NAME: "Calculator",
        CONF_ID: "key_calculator",
        CONF_ICON: "mdi:calculator",
        CONF_VALUE: 15
    },{
        CONF_NAME: "Files",
        CONF_ID: "key_files",
        CONF_ICON: "mdi:folder-file-outline",
        CONF_VALUE: 16
    },{ 
        CONF_NAME: "Voice2",
        CONF_ID: "key_voice2",
        CONF_ICON: "mdi:account-voice",
        CONF_VALUE: 17 
    },{ 
        CONF_NAME: "Android TV",
        CONF_ID: "key_androidtv",
        CONF_ICON: "mdi:android",
        CONF_VALUE: 18 
    },{
        CONF_NAME: "Back",
        CONF_ID: "key_back",
        CONF_ICON: "mdi:arrow-left-bold-outline",
        CONF_VALUE: 19
    },{
        CONF_NAME: "Forward",
        CONF_ID: "key_forward",
        CONF_ICON: "mdi:arrow-right-bold-outline",
        CONF_VALUE: 20
    },{
        CONF_NAME: "Stop",
        CONF_ID: "key_stop",
        CONF_ICON: "mdi:stop-circle-outline",
        CONF_VALUE: 21
    },{
        CONF_NAME: "Refresh",
        CONF_ID: "key_refresh",
        CONF_ICON: "mdi:refresh",
        CONF_VALUE: 22
    },{
        CONF_NAME: "Bookmarks",
        CONF_ID: "key_bookmarks",
        CONF_ICON: "mdi:bookmark-box-outline",
        CONF_VALUE: 23
    },{
        CONF_NAME: "Menu",
        CONF_ID: "key_menu",
        CONF_ICON: "mdi:bookmark-box-outline",
        CONF_VALUE: 24
    }
]

