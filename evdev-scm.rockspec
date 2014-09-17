package = "evdev"
version = "scm-1"
source = {
   url = "git://github.com/Tangent128/lua-evdev"
}
description = {
   summary = "Lua library for using Linux evdev & uinput interfaces",
   detailed = [[
A Lua 5.2 module for interacting with input devices on Linux. You can
read events with a Device object, or create a virtual input device
with a Uinput object.]],
   homepage = "http://github.com/Tangent128/lua-evdev",
   license = "MIT/X11"
}
supported_platforms = {
   "linux"
}
dependencies = {
   "lua >= 5.2"
}
build = {
   type = "builtin",
   modules = {
      evdev = "evdev.lua",
      ['evdev.constants'] = "evdev/constants.lua",
      ['evdev.core'] = {
         sources = "evdev/core.c"
      }
   }
}
