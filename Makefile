
evdev/core.so: evdev/core.c
	gcc -shared -o evdev/core.so -fPIC -Wall evdev/core.c
	
