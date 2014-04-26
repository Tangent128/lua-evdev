
#include <unistd.h>
#include <fcntl.h>
#include <linux/input.h>

#include <lua.h>
#include <lauxlib.h>

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

static const luaL_Reg evdevFuncs[] = {
	{ "open", &evdev_open },
	{ "read", &evdev_read },
	{ "close", &evdev_close },
	{ NULL, NULL }
};

int luaopen__evdev(lua_State *L) {
	luaL_newlib(L, evdevFuncs);
	return 1;
}

