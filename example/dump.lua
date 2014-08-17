
-- Usage: (note that you need read premission for the device node)
-- lua5.2 dump.lua /dev/input/event???

local evdev = require "evdev"

local path = ...

local dev = evdev.Device(path)

while true do
	print(dev:read())
end

