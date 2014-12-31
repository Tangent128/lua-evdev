
evdev/core.so: evdev/core.c
	gcc $(CFLAGS) -shared -o evdev/core.so -fPIC -Wall -Wextra -pedantic -std=c99 evdev/core.c $(LDFLAGS)
	
clean:
	rm evdev/core.so
