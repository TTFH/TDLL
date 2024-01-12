#include "lua_utils.h"

void LuaPushListA(lua_State* L, std::vector<float> list) {
	lua_newtable(L);
	for (size_t i = 0; i < list.size(); i++) {
		lua_pushnumber(L, list[i]);
		lua_rawseti(L, -2, i + 1);
	}
}

void LuaPushListB(lua_State* L, std::vector<float> list) {
	lua_createtable(L, list.size(), 0);
	for (size_t i = 0; i < list.size(); i++) {
		lua_pushinteger(L, i + 1);
		lua_pushnumber(L, list[i]);
		lua_settable(L, -3);
	}
}

void LuaPushVector(lua_State* L, float x, float y, float z) {
	lua_createtable(L, 3, 0);
	lua_pushnumber(L, x);
	lua_rawseti(L, -2, 1);

	lua_pushnumber(L, y);
	lua_rawseti(L, -2, 2);

	lua_pushnumber(L, z);
	lua_rawseti(L, -2, 3);
}

void LuaPushQuat(lua_State* L, float x, float y, float z, float w) {
	lua_createtable(L, 4, 0);
	lua_pushnumber(L, x);
	lua_rawseti(L, -2, 1);

	lua_pushnumber(L, y);
	lua_rawseti(L, -2, 2);

	lua_pushnumber(L, z);
	lua_rawseti(L, -2, 3);

	lua_pushnumber(L, w);
	lua_rawseti(L, -2, 4);
}

void LuaPushTransform(lua_State* L, Transform transform) {
	lua_createtable(L, 0, 2);

	lua_pushstring(L, "pos");
	LuaPushVector(L, transform.pos.x, transform.pos.y, transform.pos.z);
	lua_settable(L, -3);

	lua_pushstring(L, "rot");
	LuaPushQuat(L, transform.rot.x, transform.rot.y, transform.rot.z, transform.rot.w);
	lua_settable(L, -3);
}
