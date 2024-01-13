#include "lua_utils.h"

void LuaPushList(lua_State* L, std::vector<int> list) {
	td_lua_createtable(L, list.size(), 0);
	for (size_t i = 0; i < list.size(); i++) {
		lua_pushinteger(L, list[i]);
		lua_rawseti(L, -2, i + 1);
	}
}

void LuaPushVector(lua_State* L, Vector v) {
	td_lua_createtable(L, 3, 0);

	lua_pushnumber(L, v.x);
	lua_rawseti(L, -2, 1);

	lua_pushnumber(L, v.y);
	lua_rawseti(L, -2, 2);

	lua_pushnumber(L, v.z);
	lua_rawseti(L, -2, 3);
}

Vector LuaToVector(lua_State* L, int index) {
	Vector v;
	lua_rawgeti(L, index, 1);
	v.x = lua_tonumber(L, -1);
	lua_pop(L, 1);

	lua_rawgeti(L, index, 2);
	v.y = lua_tonumber(L, -1);
	lua_pop(L, 1);

	lua_rawgeti(L, index, 3);
	v.z = lua_tonumber(L, -1);
	lua_pop(L, 1);
	return v;
}

void LuaPushQuat(lua_State* L, Quat q) {
	td_lua_createtable(L, 4, 0);

	lua_pushnumber(L, q.x);
	lua_rawseti(L, -2, 1);

	lua_pushnumber(L, q.y);
	lua_rawseti(L, -2, 2);

	lua_pushnumber(L, q.z);
	lua_rawseti(L, -2, 3);

	lua_pushnumber(L, q.w);
	lua_rawseti(L, -2, 4);
}

void LuaPushTransform(lua_State* L, Transform transform) {
	td_lua_createtable(L, 0, 2);

	lua_pushstring(L, "pos");
	LuaPushVector(L, transform.pos);
	lua_settable(L, -3);

	lua_pushstring(L, "rot");
	LuaPushQuat(L, transform.rot);
	lua_settable(L, -3);
}
