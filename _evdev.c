
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

#define USERDATA_NAME "us.tropi.evdev.struct.userdev"
struct userdev {
	int fd; // file descriptor
	int init; // 0 if not initialized, 1 if initialized
	struct uinput_user_dev dev;
};

#define CHECK_UINPUT(dev, index, isInit) \
struct userdev *dev = luaL_checkudata(L, 1, USERDATA_NAME); \
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
	const char *path = luaL_checkstring(L, 1);
	
	int fd = open(path, O_WRONLY | O_NONBLOCK);
	if(fd < 0) {
		return luaL_error(L, "Couldn't open uinput device node.");
	}

	/* Don't leave device open for children should process fork */
	fcntl(fd, F_SETFD, FD_CLOEXEC);
	
	/* create userdata */
	struct userdev *dev = lua_newuserdata(L, sizeof(struct userdev));
	memset(dev, 0, sizeof(struct userdev));
	dev->fd = fd;
	
	luaL_setmetatable(L, USERDATA_NAME);
	
	/* init dummy device description */
	strncpy(dev->dev.name, "Lua-Powered Virtual Input Device", UINPUT_MAX_NAME_SIZE);
	
	
	return 1;
}

//static int uinput_setBit(lua_State *L) {

//static int uinput_setAxis(lua_State *L) {

static int uinput_init(lua_State *L) {
	
	CHECK_UINPUT(dev, 1, 0)
	
	// register device
	write(dev->fd, dev, sizeof(struct uinput_user_dev));
	
	if(ioctl(dev->fd, UI_DEV_CREATE)) {
		return luaL_error(L, "Couldn't create uinput device node.");
	}
	
	dev->init = 1;
	
	return 0;
}

static int uinput_close(lua_State *L) {

	struct userdev *dev = luaL_checkudata(L, 1, USERDATA_NAME);
	
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
	{ "open", &evdev_open },
	{ "read", &evdev_read },
	{ "close", &evdev_close },
	{ "uinput_open", &uinput_open },
	{ NULL, NULL }
};

static const luaL_Reg uinput_mtFuncs[] = {
	{ "init", &uinput_init },
	{ "close", &uinput_close },
	{ NULL, NULL }
};

int luaopen__evdev(lua_State *L) {
	
	/* Uinput metatable */
	luaL_newmetatable(L, USERDATA_NAME);
	
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

