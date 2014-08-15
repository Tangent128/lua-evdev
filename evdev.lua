
local c = require "_evdev"
local constants = require "evdev.constants"
local setmetatable = setmetatable

local _ENV = setmetatable({}, {
	__index = constants
})

Device = c.Device
Uinput = c.Uinput

return _ENV

