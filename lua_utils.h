#ifndef _LUA_UTILS_H
#define _LUA_UTILS_H

#include <map>
#include <string>
#include <vector>

#include "lua5.1.4/lua.hpp"
#include "teardown_structs.h"

typedef void (*t_lua_createtable) (lua_State* L, int narr, int nrec);
extern t_lua_createtable td_lua_createtable;

typedef void (*t_lua_pushstring) (lua_State* L, const char* s);
extern t_lua_pushstring td_lua_pushstring;

void LuaPushList(lua_State* L, std::vector<int> list);
void LuaPushVector(lua_State* L, Vector v);
void LuaPushQuat(lua_State* L, Quat q);
void LuaPushTransform(lua_State* L, Transform transform);
Vector LuaToVector(lua_State* L, int index);
void LuaPushFuntion(lua_State* L, const char* name, lua_CFunction func);
bool LuaIsGlobalDefined(lua_State* L, const char* name);

// Not so Lua related
int HttpRequest(const char*, const char*, std::map<std::string, std::string>, const char*, const char*, std::string&);

#endif
