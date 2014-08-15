
local evdev = require "evdev"

local path = ...

local dev = evdev.Device(path)

while true do
	print(dev:read())
end

