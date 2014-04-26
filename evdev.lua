
local c = require "_evdev"
local constants = require "evdev.constants"
local setmetatable = setmetatable

local _ENV = setmetatable({}, {
	__index = constants
})

local dev = {}
local evdev_mt = {
	__index = dev
}

function open(devpath)
	local dev = setmetatable({}, evdev_mt)
	dev.fd = c.open(devpath)
	return dev
end

-- return: timestamp, event type, event code, event value
function dev:read()
	return c.read(self.fd)
end

function dev:close()
	if self.fd then
		c.close(self.fd)
		self.fd = nil
	end
end

evdev_mt.__gc = dev.close

return _ENV

