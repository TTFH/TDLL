#ifndef _LUA_UTILS_H
#define _LUA_UTILS_H

#include <map>
#include <string>
#include <vector>

#include <lua.hpp>
#include "teardown_structs.h"

typedef void (*t_lua_createtable) (lua_State* L, int narr, int nrec);
extern t_lua_createtable td_lua_createtable;

typedef void (*t_lua_pushstring) (lua_State* L, const char* s);
extern t_lua_pushstring td_lua_pushstring;

void LuaPushList(lua_State* L, std::vector<int> list);
void LuaPushVec3(lua_State* L, Vec3 v);
void LuaPushQuat(lua_State* L, Quat q);
void LuaPushTransform(lua_State* L, Transform transform);
Vec3 LuaToVec3(lua_State* L, int index);
Quat LuaToQuat(lua_State* L, int index);
Transform LuaToTransform(lua_State* L, int index);
void LuaPushFunction(lua_State* L, const char* name, lua_CFunction func);
void LuaPushEmptyTable(lua_State* L);
bool LuaIsGlobalDefined(lua_State* L, const char* name);

// Not so Lua related
int HttpRequest(const char*, const char*, std::map<std::string, std::string>, const char*, const char*, std::string&);

#endif
