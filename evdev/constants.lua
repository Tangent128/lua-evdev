
local _ENV = {}

-- Constants for the Linux evdev API
-- currently nowhere near exhaustive 

-- Event Types
EV_KEY	=	0x01

EV_ABS	=	0x03

-- Buttons
BTN_0	=	0x100
BTN_1	=	0x101
BTN_2	=	0x102

BTN_LEFT	=	0x110
BTN_RIGHT	=	0x111
BTN_MIDDLE	=	0x112

BTN_TOOL_PEN	=	0x140

BTN_TOUCH	=	0x14a
BTN_STYLUS	=	0x14b
BTN_STYLUS2	=	0x14c

-- Axes
ABS_X	=	0x00
ABS_Y	=	0x01
ABS_Z	=	0x02
ABS_RX	=	0x03
ABS_RY	=	0x04
ABS_RZ	=	0x05
ABS_PRESSURE	=	0x18

return _ENV
