-- script for generating constants.lua from the linux/input.h header

local capture = {
	{"Event Types", "^EV_"},
	{"Synchronization", "^SYN_"},
	{"Keys", "^KEY_"},
	{"Buttons", "^BTN_"},
	{"Relative Axes", "^REL_"},
	{"Absolute Axes", "^ABS_"},
}

local header = [[

local _ENV = {}

-- Constants for the Linux evdev API
-- created by the gen-constants.lua script from the <linux/input.h> header
]]

local footer = [[
return _ENV
]]

local headerFile = ...

if not headerFile then
	print "Please provide the header file path as an argument"
	return 1
end

local sections = {}
for _, record in pairs(capture) do
	sections[record[1]] = "-- " .. record[1] .. "\n"
end

for line in io.lines(headerFile) do
	local name, value = line:match "^#define%s+(%S+)%s+(%S+)"
	if name then
		for _, record in pairs(capture) do
			if name:match(record[2]) then
				sections[record[1]] = sections[record[1]] .. name .. " = " .. value .. "\n"
			end
		end
	end
end

print(header)
for _, record in pairs(capture) do
	print(sections[record[1]])
end
print(footer)
