-- script for generating constants.lua from the linux/input.h header

local capture = {
	{"Event Types", "^EV_"},
	{"Synchronization", "^SYN_"},
	{"Keys", "^KEY_"},
	{"Buttons", "^BTN_"},
	{"Relative Axes", "^REL_"},
	{"Absolute Axes", "^ABS_"},
	{"LEDs", "^LED_"},
}

local header = [[

local _ENV = {}

if setfenv then
	setfenv(1, _ENV)
end

-- Constants for the Linux evdev API
-- created by the gen-constants.lua script from the <linux/input.h> header
]]

local footer = [[
return _ENV
]]

local headerFiles = { ... }

if #headerFiles == 0 then
	print "Please provide header file path(s) as arguments"
	return 1
end

local sections = {}
for _, record in pairs(capture) do
	sections[record[1]] = "-- " .. record[1] .. "\n"
end

for headerIndex = 1, #headerFiles do
	for line in io.lines(headerFiles[headerIndex]) do
		local name, value = line:match "^#define%s+(%S+)%s+(%S+)"
		if name then
			for _, record in pairs(capture) do
				if name:match(record[2]) then
					local defineLine = name .. " = " .. value .. "\n"
					sections[record[1]] = sections[record[1]] .. defineLine
				end
			end
		end
	end
end

print(header)
for _, record in pairs(capture) do
	print(sections[record[1]])
end
print(footer)
