#include "lua_utils.h"
#include "signatures.h"

void LuaPushList(lua_State* L, std::vector<int> list) {
	td_lua_createtable(L, list.size(), 0);
	for (size_t i = 0; i < list.size(); i++) {
		lua_pushinteger(L, list[i]);
		lua_rawseti(L, -2, i + 1);
	}
}

void LuaPushVec3(lua_State* L, Vec3 v) {
	td_lua_createtable(L, 3, 0);

	lua_pushnumber(L, v.x);
	lua_rawseti(L, -2, 1);

	lua_pushnumber(L, v.y);
	lua_rawseti(L, -2, 2);

	lua_pushnumber(L, v.z);
	lua_rawseti(L, -2, 3);
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

	td_lua_pushstring(L, "pos");
	LuaPushVec3(L, transform.pos);
	lua_settable(L, -3);

	td_lua_pushstring(L, "rot");
	LuaPushQuat(L, transform.rot);
	lua_settable(L, -3);
}

Vec3 LuaToVec3(lua_State* L, int index) {
	Vec3 v;
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

Quat LuaToQuat(lua_State* L, int index) {
	Quat q;
	lua_rawgeti(L, index, 1);
	q.x = lua_tonumber(L, -1);
	lua_pop(L, 1);

	lua_rawgeti(L, index, 2);
	q.y = lua_tonumber(L, -1);
	lua_pop(L, 1);

	lua_rawgeti(L, index, 3);
	q.z = lua_tonumber(L, -1);
	lua_pop(L, 1);

	lua_rawgeti(L, index, 4);
	q.w = lua_tonumber(L, -1);
	lua_pop(L, 1);
	return q;
}

Transform LuaToTransform(lua_State* L, int index) {
	Transform transform;
	lua_getfield(L, index, "pos");
	transform.pos = LuaToVec3(L, -1);
	lua_pop(L, 1);

	lua_getfield(L, index, "rot");
	transform.rot = LuaToQuat(L, -1);
	lua_pop(L, 1);
	return transform;
}

std::map<std::string, std::string> LuaToMap(lua_State* L, int index) {
	std::map<std::string, std::string> map;
	lua_pushnil(L);
	if (lua_istable(L, index)) {
		while (lua_next(L, index) != 0) {
			std::string key = lua_tostring(L, -2);
			std::string value = lua_tostring(L, -1);
			map[key] = value;
			lua_pop(L, 1);
		}
	}
	return map;
}

void LuaPushFunction(lua_State* L, const char* name, lua_CFunction func) {
	lua_pushcfunction(L, func);
	lua_setglobal(L, name);
}

void LuaPushEmptyTable(lua_State* L) {
	td_lua_createtable(L, 0, 0);
}

bool LuaIsGlobalDefined(lua_State* L, const char* name) {
	lua_getglobal(L, name);
	bool defined = !lua_isnil(L, -1);
	lua_pop(L, 1);
	return defined;
}
