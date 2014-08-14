
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <linux/input.h>
#include <linux/uinput.h>

#include <lua.h>
#include <lauxlib.h>

/* Evdev open/read/close wrappers */

static int evdev_open(lua_State *L) {
	const char *path = luaL_checkstring(L, 1);
	
	int fd = open(path, O_RDONLY);
	if(fd < 0) {
		return luaL_error(L, "Couldn't open device node.");
	}

	/* Don't leave device open for children should process fork */
	fcntl(fd, F_SETFD, FD_CLOEXEC);
	
	lua_pushinteger(L, fd);
	
	return 1;
}

static int evdev_read(lua_State *L) {
	int fd = luaL_checkinteger(L, 1);

	struct input_event evt;
	const size_t evt_size = sizeof(struct input_event);

	int count = read(fd, &evt, evt_size);
	if(count < evt_size) {
		return luaL_error(L, "Failure reading input event.");
	}

	/* return: timestamp, event type, event code, event value */
	lua_pushnumber(L, evt.time.tv_sec + evt.time.tv_usec/1000000.0);
	lua_pushinteger(L, evt.type);
	lua_pushinteger(L, evt.code);
	lua_pushinteger(L, evt.value);
	
	return 4;
}

static int evdev_close(lua_State *L) {
	int fd = luaL_checkinteger(L, 1);
	
	close(fd);
	
	return 0;
}

/* Uinput open/write/close wrappers */

#define USERDATA_NAME "lua.evdev.uinput.struct.uinput_user_dev"

static int uinput_open(lua_State *L) {
	const char *path = luaL_checkstring(L, 1);
	
	int fd = open(path, O_WRONLY | O_NONBLOCK);
	if(fd < 0) {
		return luaL_error(L, "Couldn't open uinput device node.");
	}

	/* Don't leave device open for children should process fork */
	fcntl(fd, F_SETFD, FD_CLOEXEC);
	
	/* push results */
	lua_pushinteger(L, fd);

	struct uinput_user_dev *dev = lua_newuserdata(L, sizeof(struct uinput_user_dev));
	luaL_setmetatable(L, USERDATA_NAME);
	
	/* init dummy device description */
	memset(dev, 0, sizeof(struct uinput_user_dev));
	strncpy(dev->name, "Lua-Powered Virtual Input Device", UINPUT_MAX_NAME_SIZE);
	
	
	return 2;
}

//static int uinput_setBit(lua_State *L) {

//static int uinput_setAxis(lua_State *L) {

static int uinput_init(lua_State *L) {
	
	int fd = luaL_checkinteger(L, 1);
	
	struct uinput_user_dev *dev = luaL_checkudata(L, 2, USERDATA_NAME);
	
	// register device
	write(fd, dev, sizeof(struct uinput_user_dev));
	
	if(ioctl(fd, UI_DEV_CREATE)) {
		return luaL_error(L, "Couldn't create uinput device node.");
	}
	
	return 0;
}

static int uinput_close(lua_State *L) {
	int fd = luaL_checkinteger(L, 1);
	
	/* should destroy on close anyways, but being explicit: */
	ioctl(fd, UI_DEV_DESTROY);
	close(fd);
	
	return 0;
}

/* Expose to Lua */

static const luaL_Reg evdevFuncs[] = {
	{ "open", &evdev_open },
	{ "read", &evdev_read },
	{ "close", &evdev_close },
	{ "uinput_open", &uinput_open },
	{ "uinput_init", &uinput_init },
	{ "uinput_close", &uinput_close },
	{ NULL, NULL }
};

int luaopen__evdev(lua_State *L) {
	luaL_newmetatable(L, USERDATA_NAME);
	luaL_newlib(L, evdevFuncs);
	return 1;
}

