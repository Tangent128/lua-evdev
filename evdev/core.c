/*
 * Lua 5.2 module for interacting with input devices on Linux.
 * 
 * Supports reading events from device nodes and creating virtual
 * devices via uinput.
 * 
Copyright © 2015 Tangent 128

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
 */

/* Needed for O_CLOEXEC */
#if !defined(_GNU_SOURCE)
#define _GNU_SOURCE
#endif

#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <linux/input.h>
#include <linux/uinput.h>

#include <lua.h>
#include <lauxlib.h>

/* Evdev wrappers */

#define EVDEV_USERDATA "us.tropi.evdev.struct.inputDevice"
struct inputDevice {
	int fd; /* file descriptor */
};

#define CHECK_EVDEV(dev, index) \
struct inputDevice *dev = luaL_checkudata(L, index, EVDEV_USERDATA); \
if(dev->fd == -1) { \
	return luaL_error(L, "Trying to use closed device event node."); \
}

static int evdev_open(lua_State *L) {
	const char *path = luaL_checkstring(L, 1);
	int writeMode = lua_toboolean(L, 2);

	int fd = -1;
	
	if(writeMode) {
		// if requested, attempt opening for writing so we can send LED events and such
		fd = open(path, O_RDWR | O_CLOEXEC);
	}
	
	if(fd < 0) {
		// writing mode not requested or not allowed,
		// try falling back to reading events only
		fd = open(path, O_RDONLY | O_CLOEXEC);
	}

	if(fd < 0) {
		// still couldn't open, nothing to be done.
		return luaL_error(L, "Couldn't open device node.");
	}
	
	/* create userdata */
	struct inputDevice *dev = lua_newuserdata(L, sizeof(struct inputDevice));
	dev->fd = fd;

	luaL_setmetatable(L, EVDEV_USERDATA);

	return 1;
}

static int evdev_tryRead(lua_State *L) {
	CHECK_EVDEV(dev, 1);

	struct input_event evt;
	const size_t evt_size = sizeof(struct input_event);
	memset(&evt, 0, evt_size);

	int count = read(dev->fd, &evt, evt_size);

	if(count < 0) {
		/* device was presumably unplugged */
		return 0;
	} else if((unsigned int) count < evt_size) {
		return luaL_error(L, "Failure reading input event.");
	}

	/* return: timestamp, event type, event code, event value */
	lua_pushnumber(L, evt.time.tv_sec + evt.time.tv_usec/1000000.0);
	lua_pushinteger(L, evt.type);
	lua_pushinteger(L, evt.code);
	lua_pushinteger(L, evt.value);
	
	return 4;
}

static int evdev_read(lua_State *L) {
	int count = evdev_tryRead(L);

	if(count == 0) {
		return luaL_error(L, "End of input event stream.");
	}

	return count;
}

static int evdev_grab(lua_State *L) {
	CHECK_EVDEV(dev, 1);

	int wantGrab = lua_isnone(L, 2) || lua_toboolean(L, 2);

	if(ioctl(dev->fd, EVIOCGRAB, wantGrab) < 0) {
		/* Presuming any error here means a failed grab;
		 * not sure if any error besides EBUSY is possible. */
		lua_pushboolean(L, 0);
		return 1;
	}
	
	lua_pushboolean(L, 1);
	return 1;
}

static int evdev_write(lua_State *L) {
	CHECK_EVDEV(dev, 1);
	
	struct input_event evt;
	memset(&evt, 0, sizeof(struct input_event));
	
	evt.type = luaL_checkinteger(L, 2);
	evt.code = luaL_checkinteger(L, 3);
	evt.value = luaL_checkinteger(L, 4);
	
	write(dev->fd, &evt, sizeof(struct input_event));
	
	return 0;
}

static int evdev_close(lua_State *L) {
	struct inputDevice *dev = luaL_checkudata(L, 1, EVDEV_USERDATA);
	
	if(dev->fd != -1) {
		close(dev->fd);
		dev->fd = -1;
	}

	return 0;
}

static int evdev_pollfd(lua_State *L) {
	CHECK_EVDEV(dev, 1);

	lua_pushinteger(L, dev->fd);

	return 1;
}

/* Uinput wrappers */

#define UINPUT_USERDATA "us.tropi.evdev.struct.userdev"
struct userdev {
       int fd; // file descriptor
       int init; // 0 if not initialized, 1 if initialized
       struct uinput_user_dev dev;
};

#define CHECK_UINPUT(dev, index, isInit) \
struct userdev *dev = luaL_checkudata(L, index, UINPUT_USERDATA); \
if(dev->fd == -1) { \
	return luaL_error(L, "Trying to use closed uinput device node."); \
} \
if(isInit == 1) { \
	if(dev->init == 0 ) \
		return luaL_error(L, "Trying to use uninitialized uinput device node."); \
} else if(isInit == 0) { \
	if(dev->init == 1) \
		return luaL_error(L, "Trying to configure initialized uinput device node."); \
} else if(isInit == 2) { \
	/* Don't care about initialization state */ \
} else { \
	/* Should never happen */ \
	return luaL_error(L, "Logic error in CHECK_UINPUT"); \
}


static int uinput_open(lua_State *L) {
	const char *path = luaL_optstring(L, 1, "/dev/uinput");
	
	int fd = open(path, O_WRONLY | O_NONBLOCK | O_CLOEXEC);
	if(fd < 0) {
		return luaL_error(L, "Couldn't open uinput device node.");
	}

	/* create userdata */
	struct userdev *dev = lua_newuserdata(L, sizeof(struct userdev));
	memset(dev, 0, sizeof(struct userdev));
	dev->fd = fd;
	
	luaL_setmetatable(L, UINPUT_USERDATA);
	
	/* init dummy device description */
	dev->dev.id.bustype = BUS_VIRTUAL;
	
	return 1;
}

#define BIT_TYPES(action, axisAction) \
action(useEvent, UI_SET_EVBIT) \
action(useKey, UI_SET_KEYBIT) \
axisAction(useRelAxis, UI_SET_RELBIT) \
axisAction(useAbsAxis, UI_SET_ABSBIT)


#define DECLARE_BIT_SETTER(name, type) \
static int uinput_ ## name (lua_State *L) { \
	CHECK_UINPUT(dev, 1, 0) \
	int bit = luaL_checkinteger(L, 2); \
	ioctl(dev->fd, type, bit); \
	return 0; \
}

#define DECLARE_AXIS_BIT_SETTER(name, type) \
static int uinput_ ## name (lua_State *L) { \
	CHECK_UINPUT(dev, 1, 0) \
	int bit = luaL_checkinteger(L, 2); \
	int minVal = luaL_checkinteger(L, 3); \
	int maxVal = luaL_checkinteger(L, 4); \
	ioctl(dev->fd, type, bit); \
	dev->dev.absmin[bit] = minVal; \
	dev->dev.absmax[bit] = maxVal; \
	return 0; \
}

BIT_TYPES(DECLARE_BIT_SETTER, DECLARE_AXIS_BIT_SETTER)

static int uinput_init(lua_State *L) {
	
	CHECK_UINPUT(dev, 1, 0)
	
	/* Give device human-friendly description */
	const char *name = luaL_optstring(L, 2, "Lua-Powered Virtual Input Device");
	strncpy(dev->dev.name, name, UINPUT_MAX_NAME_SIZE);
	
	// register device
	write(dev->fd, &dev->dev, sizeof(struct uinput_user_dev));
	
	if(ioctl(dev->fd, UI_DEV_CREATE)) {
		return luaL_error(L, "Couldn't create uinput device node.");
	}
	
	dev->init = 1;
	
	return 0;
}

static int uinput_write(lua_State *L) {
	CHECK_UINPUT(dev, 1, 1)
	
	struct input_event evt;
	memset(&evt, 0, sizeof(struct input_event));
	
	evt.type = luaL_checkinteger(L, 2);
	evt.code = luaL_checkinteger(L, 3);
	evt.value = luaL_checkinteger(L, 4);
	
	write(dev->fd, &evt, sizeof(struct input_event));
	
	return 0;
}

static int uinput_close(lua_State *L) {

	struct userdev *dev = luaL_checkudata(L, 1, UINPUT_USERDATA);
	
	/* explicit double-closes are poor practice,
	 * but don't make them errors because we always close on __gc  */
	if(dev->fd == -1) {
		return 0;
	}
	
	/* uinput device destroys on close anyways, but being explicit: */
	ioctl(dev->fd, UI_DEV_DESTROY);
	close(dev->fd);
	
	/* mark resource released */
	dev->fd = -1;
	
	return 0;
}

/* Expose to Lua */

static const luaL_Reg evdevFuncs[] = {
	{ "Device", &evdev_open },
	{ "Uinput", &uinput_open },
	{ NULL, NULL }
};

static const luaL_Reg evdev_mtFuncs[] = {
	{ "read", &evdev_read },
	{ "tryRead", &evdev_tryRead },
	{ "write", &evdev_write},
	{ "close", &evdev_close },
	{ "grab", &evdev_grab },
	{ "pollfd", &evdev_pollfd },
	{ NULL, NULL }
};

#define REGISTER_BIT_SETTER(name, type) \
{ #name, &uinput_ ## name },

static const luaL_Reg uinput_mtFuncs[] = {
	{ "init", &uinput_init },
	{ "close", &uinput_close },
	{ "write", &uinput_write},
	BIT_TYPES(REGISTER_BIT_SETTER, REGISTER_BIT_SETTER)
	{ NULL, NULL }
};

int luaopen_evdev_core(lua_State *L) {
	
	/* Evdev metatable */
	luaL_newmetatable(L, EVDEV_USERDATA);
	
	lua_pushstring(L, "__index");
	luaL_newlib(L, evdev_mtFuncs);
	
		lua_pushstring(L, "events");
		lua_pushstring(L, "r");
		lua_settable(L, -3);
	
	lua_settable(L, -3);

	lua_pushstring(L, "__gc");
	lua_pushcfunction(L, &evdev_close);
	lua_settable(L, -3);
	
	
	/* Uinput metatable */
	luaL_newmetatable(L, UINPUT_USERDATA);
	
	lua_pushstring(L, "__index");
	luaL_newlib(L, uinput_mtFuncs);
	lua_settable(L, -3);

	lua_pushstring(L, "__gc");
	lua_pushcfunction(L, &uinput_close);
	lua_settable(L, -3);
	
	/* Base library */
	luaL_newlib(L, evdevFuncs);
	
	return 1;
}

