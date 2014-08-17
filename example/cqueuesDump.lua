
-- example of using Device objects as cqueues-pollable objects
-- cqueues needs to be installed; see:
-- https://github.com/wahern/cqueues

-- Usage: (note that you need read premission for the device nodes)
-- lua5.2 dump.lua /dev/input/event??? /dev/input/event???

local evdev = require "evdev"
local cqueues = require "cqueues"

local kbDev, mouseDev = ...

local kb = evdev.Device(kbDev)
local mouse = evdev.Device(mouseDev)

local loop = cqueues.new()

loop:wrap(function()
	while true do
		cqueues.poll(kb)
		print("KEYBOARD", kb:read())
	end
end)

loop:wrap(function()
	while true do
		cqueues.poll(mouse)
		print("MOUSE", mouse:read())
	end
end)

print(loop:loop())
