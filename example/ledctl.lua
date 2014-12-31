local evdev = require "evdev"

-- usage:
-- lua ledctl.lua /dev/path/to/device LED_CONSTANT {0 | 1}

-- example:
-- lua ledctl.lua /dev/input/event1 LED_NUML 0
-- lua ledctl.lua /dev/input/event1 LED_NUML 1

-- example LED constants:
-- LED_NUML
-- LED_CAPSL
-- LED_SCROLLL
local path, led, state = ...

dev = evdev.Device(path, true)

ledID = evdev[led]

state = tonumber(state)

dev:write(evdev.EV_LED, ledID, state)
dev:write(evdev.EV_SYN, evdev.SYN_REPORT, 0)
