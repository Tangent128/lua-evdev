
local evdev = require "evdev"

local path = ...

local dev = evdev.Device(path)

local x,y = 0,0
local btn0,btn1,btn2 = 0,0,0

local function stat()
	print(x,y, "buttons:", btn0,btn1,btn2 )
end

while true do
	local timestamp, type, code, value = dev:read()
	if type == evdev.EV_ABS then
		if code == evdev.ABS_X then
			x = value
		elseif code == evdev.ABS_Y then
			y = value
		end
		stat()
	elseif type == evdev.EV_KEY then
		if code == evdev.BTN_TOUCH then
			btn0 = value
		elseif code == evdev.BTN_STYLUS then
			btn1 = value
		elseif code == evdev.BTN_STYLUS2 then
			btn2 = value
		end
		stat()
	end
end

