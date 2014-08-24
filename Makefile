
evdev/core.so: evdev/core.c
	gcc -shared -o evdev/core.so -fPIC -Wall -Wextra -pedantic -std=c99 evdev/core.c
	
