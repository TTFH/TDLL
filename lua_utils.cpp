#include "lua_utils.h"

void LuaPushList(lua_State* L, std::vector<int> list) {
    lua_createtable(L, list.size(), 0);
    for (size_t i = 0; i < list.size(); ++i) {
		lua_pushinteger(L, i + 1);
        lua_pushinteger(L, list[i]);
        lua_settable(L, -3);
    }
}

void LuaPushVector(lua_State* L, float x, float y, float z) {
	lua_createtable(L, 3, 0);
	lua_pushnumber(L, 1);
	lua_pushnumber(L, x);
	lua_settable(L, -3);

	lua_pushnumber(L, 2);
	lua_pushnumber(L, y);
	lua_settable(L, -3);

	lua_pushnumber(L, 3);
	lua_pushnumber(L, z);
	lua_settable(L, -3);
}

void LuaPushTransform(lua_State* L, Transform transform) {
	lua_createtable(L, 0, 2);
	lua_pushstring(L, "pos");
	LuaPushVector(L, transform.pos.x, transform.pos.y, transform.pos.z);
	lua_settable(L, -3);

	lua_pushstring(L, "rot");
	lua_createtable(L, 4, 0);

	lua_pushnumber(L, 1);
	lua_pushnumber(L, transform.rot.x);
	lua_settable(L, -3);

	lua_pushnumber(L, 2);
	lua_pushnumber(L, transform.rot.y);
	lua_settable(L, -3);

	lua_pushnumber(L, 3);
	lua_pushnumber(L, transform.rot.z);
	lua_settable(L, -3);

	lua_pushnumber(L, 4);
	lua_pushnumber(L, transform.rot.w);
	lua_settable(L, -3);
	lua_settable(L, -3);
}
