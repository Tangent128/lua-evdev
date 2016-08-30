# Configuration

BUILD ?= ./build

COMPAT_CFLAGS ?= -D _GNU_SOURCE -I ./compat53 -include compat-5.3.h
O_CFLAGS      ?= -fPIC -Wall -Wextra -pedantic -std=c99 $(COMPAT_CFLAGS)
SO_LDFLAGS    ?= -shared
DEPFILES      != [ -d $(BUILD)/deps ] && find $(BUILD)/deps -name *.d

all: $(BUILD)/evdev/core.so

.PHONY: all clean print

# Build Rules

$(BUILD)/%.o: %.c
	mkdir -p $(BUILD)/$(*D) $(BUILD)/deps/$(*D)
	$(CC) $(O_CFLAGS) $(CFLAGS) -c $< -o $@ -MMD -MP -MF $(BUILD)/deps/$*.d

%.so:
	mkdir -p $(@D)
	$(LD) $(SO_LDFLAGS) $(LDFLAGS) $^ -o $@

# Utility

clean:
	-rm -r $(BUILD)

print:
	@echo BUILD=$(BUILD)
	@echo COMPAT_CFLAGS=$(COMPAT_CFLAGS)
	@echo O_CFLAGS=$(O_CFLAGS)
	@echo SO_LDFLAGS=$(SO_LDFLAGS)
	@echo DEPFILES=$(DEPFILES)
	@echo
	@echo CC=$(CC)
	@echo LD=$(LD)
	@echo CFLAGS=$(CFLAGS)
	@echo LDFLAGS=$(LDFLAGS)

# Dependencies

-include $(DEPFILES)

$(BUILD)/evdev/core.so: $(BUILD)/evdev/core.o
