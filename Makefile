
# Configuration

SRC= .

COMPAT_CFLAGS= -D _GNU_SOURCE -I $(SRC)/compat53 -include compat-5.3.h
CFLAGS= -shared -fPIC -Wall -Wextra -pedantic -std=c99 $(MYCFLAGS) $(COMPAT_CFLAGS)

CORE_SO= evdev/core.so

# Filepaths

CORE_C= $(SRC)/evdev/core.c

# Rules

default: $(CORE_SO)

$(CORE_SO): $(CORE_C)
	gcc $(CFLAGS) -o $(CORE_SO) $(CORE_C) $(LDFLAGS)
	
clean:
	-rm $(CORE_SO)
