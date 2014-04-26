
local evdev = require "evdev"

local path = ...

local dev = evdev.open(path)

while true do
	print(dev:read())
end

