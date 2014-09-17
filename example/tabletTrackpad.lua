
-- turn a tablet (absolute pointing device)
-- into a trackpad (relative pointing device)

local e = require "evdev"
local constants = require "evdev.constants"
local cqueues = require "cqueues"

local dev = e.Device(...)
dev:grab()

------

local fakeMouse = e.Uinput()

fakeMouse:useEvent(e.EV_KEY)
fakeMouse:useEvent(e.EV_REL)
fakeMouse:useKey(e.BTN_0)
fakeMouse:useKey(e.BTN_1)
fakeMouse:useRelAxis(e.REL_X,-1000,1000)
fakeMouse:useRelAxis(e.REL_Y,-1000,1000)
fakeMouse:init "Lua Mouse"

------

local loop = cqueues.new()

loop:wrap(function()

	local touching, rx, ry, x, y = false, 0, 0, 0, 0

	while true do
		cqueues.poll(dev)
		local timestamp, class, event, state = dev:read()
		
		if class == e.EV_KEY and event == e.BTN_TOUCH then
			touching = state == 1
		end

		if class == e.EV_ABS and event == e.ABS_X then
			x = state
		end
		if class == e.EV_ABS and event == e.ABS_Y then
			y = state
		end
		
		if class == e.EV_SYN then
			if touching then
				local dx, dy = x-rx, y-ry
				local scale = 1
				dx, dy = dx*scale, dy*scale
				fakeMouse:write(e.EV_REL, e.REL_X, dx)
				fakeMouse:write(e.EV_REL, e.REL_Y, dy)
				fakeMouse:write(e.EV_SYN, e.SYN_REPORT, 0)
			end
			rx, ry = x, y
		end

		
		if class == e.EV_KEY and event == e.BTN_STYLUS then
			fakeMouse:write(e.EV_KEY, e.BTN_2, state)
			fakeMouse:write(e.EV_SYN, e.SYN_REPORT, 0)
		end		
		if class == e.EV_KEY and event == e.BTN_STYLUS2 then
			fakeMouse:write(e.EV_KEY, e.BTN_0, state)
			fakeMouse:write(e.EV_SYN, e.SYN_REPORT, 0)
		end		
	end
end)

print(loop:loop())
