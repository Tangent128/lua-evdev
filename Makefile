
_evdev.so: _evdev.c
	gcc -shared -o _evdev.so -fPIC -Wall _evdev.c
	
