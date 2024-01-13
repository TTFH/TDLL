#ifndef _LUA_H
#define _LUA_H

#include <vector>
#include "lua5.1.4/lua.hpp"
#include "teardown_structs.h"

typedef void (*t_lua_createtable) (lua_State *L, int narr, int nrec);
extern t_lua_createtable td_lua_createtable;

void LuaPushList(lua_State* L, std::vector<int> list);
void LuaPushVector(lua_State* L, Vector v);
Vector LuaToVector(lua_State* L, int index);
void LuaPushTransform(lua_State* L, Transform transform);

#endif
