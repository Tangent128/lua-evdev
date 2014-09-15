Evdev Lua module
===

A Lua 5.2 module for interacting with input devices on Linux. You can
read events with a `Device` object, or create a virtual input device
with a `Uinput` object.

The `evdev.constants` module contains most of the relevant constants
from `linux/input.h` (EV_KEY, BTN_0, KEY_A, ABS_X, etc), and is used
as an `__index` for the `evdev` module itself for convenience.

Example: Reading events from /dev/input/event* nodes:
---

```lua
local evdev = require "evdev"
-- (assuming event0 is the keyboard, which in practice easily varies)
local keyboard = evdev.Device "/dev/input/event0"
while true do
    local timestamp, eventType, eventCode, value = keyboard:read()
    if eventType == evdev.EV_KEY then
		if eventCode == evdev.KEY_ESC then
			break
		end
		if value == 0 then
			print("Key Released:", eventCode)
		else
			print("Key Pressed:", eventCode)
		end
    end
end
```

Example: Creating a fake mouse:
---

```lua
local e = require "evdev"
local fakeMouse = e.Uinput "/dev/uinput"
-- register supported events
fakeMouse:useEvent(e.EV_KEY)
fakeMouse:useEvent(e.EV_REL)
fakeMouse:useKey(e.BTN_0)
fakeMouse:useKey(e.BTN_1)
fakeMouse:useRelAxis(e.REL_X,-100,100)
fakeMouse:useRelAxis(e.REL_Y,-100,100)
fakeMouse:init()
-- inject a few events; mouse move, button press, button release
fakeMouse:write(e.EV_REL, e.REL_X, -50)
fakeMouse:write(e.EV_REL, e.REL_Y, 0)
fakeMouse:write(e.EV_SYN, e.SYN_REPORT, 0)
fakeMouse:write(e.EV_KEY, e.BTN_0, 1)
fakeMouse:write(e.EV_SYN, e.SYN_REPORT, 0)
fakeMouse:write(e.EV_KEY, e.BTN_0, 0)
fakeMouse:write(e.EV_SYN, e.SYN_REPORT, 0)
-- dispose
fakeMouse:close()
```

API
===

Device - read input events
---

`evdev.Device(path)` - open the device event node at `path`, returning
a `Device` object. File permissions to read `path` are necessary.

`Device:read()` - read a single event, returning 4 values:

1. the floating-point timestamp of the event
2. the general type of event (EV_SYN, EV_KEY, EV_REL, EV_ABS, etc.)
3. the event code (KEY_A, REL_X, ABS_Y, etc.)
4. the event value (axis value, or 0/1 for button state)

The read will block if no events are available, and throw an error if
the device reaches EOF (such as if unplugged).

`Device:tryRead()` - like `Device:read()`, but returns nil on EOF.

`Device:grab(enable)` - if the argument is true or not given, grab the
device, ensuring all input events for it are exclusively delivered to
this handle. Returns true if the grab suceeded.

If the argument is false, release any existing grab.

`Device:close()` - close the file descriptor; further reads will be
errors. `Device` objects are automatically closed on garbage-collection.

`Device:pollfd()` - return the numeric fd, which can be used with
external event loops or polling libraries to avoid blocking.

`Device.events` - "r"; provided to allow a `Device` object to be
compatable with cqueues.poll(), if you use the [cqueues][cqueues] library.

Uinput - submit virtual input events
---

`evdev.Uinput(path)` - open the uinput device node at `path`, returning
a `Uinput` object. File permissions to read `path` are necessary. If
path is not given, a default of "/dev/uinput" is used.

`Uinput:useEvent(type)` - declare that this virtual device will emit
events of the given type (such as EV_KEY, EV_REL, EV_ABS, etc.);
must be called before `:init()`

`Uinput:useKey(code)` - declare that this virtual device can emit
key events of the given keycode (such as KEY_A, KEY_ESC, KEY_LEFT, etc.);
must be called before `:init()`; the EV_KEY event should be declared.

`Uinput:useAbsAxis(axis, min, max)`, `Uinput:useRelAxis(axis, min, max)` -
declare that this virtual device can emit absolute or relative axis events
on the given axis (such as ABS_X, REL_Y, REL_WHEEL, ABS_PRESSURE, etc.);
must be called before `:init()`; the EV_ABS or EV_REL event should be declared.

`Uinput:init(title)` - create the virtual device. `title` will be used
for the human-friendly name visible to the system if given.

`Uinput:write(type, code, value)` - feed the given event to the input
system via this virtual device; must be called after `:init()`.

`Uinput:close()` - close the file descriptor; further writes will be
errors. `Uinput` objects are automatically closed on garbage-collection.


[cqueues]: http://25thandclement.com/~william/projects/cqueues.html
