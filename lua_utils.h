#ifndef _LUA_H
#define _LUA_H

#include <vector>
/*
extern "C" {
	#include <lua5.1/lua.h>
	#include <lua5.1/lualib.h>
	#include <lua5.1/lauxlib.h>
}
*/
#include "lua5.1.4/lua.hpp"
#include "teardown_structs.h"

void LuaPushVector(lua_State* L, float x, float y, float z);
void LuaPushTransform(lua_State* L, Transform transform);

#endif
