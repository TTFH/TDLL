#ifndef _SIGNATURES_H
#define _SIGNATURES_H

#include <lua.hpp>
#include <stdint.h>
#include "teardown_structs.h"

namespace MEM_OFFSET {						// Type
	extern uintptr_t Game;					// Game*
/*	extern uintptr_t LuaPushString;			// void fn(lua_State*, const char*)
	extern uintptr_t LuaCreateTable;		// void fn(lua_State*, int, int)
	extern uintptr_t InitScriptInnerLoop;	// void fn(ScriptCoreInner*)
	extern uintptr_t FarPlane;				// float
	extern uintptr_t NewNearPlane;			// float
	extern uintptr_t LoadNearPlane;			// movss xmm8, 0.05
	extern uintptr_t ProcessVideoFrameGL;	// void fn(ScreenCapture*, int)*/
}

typedef void (*t_lua_pushstring) (lua_State* L, const char* s);
extern t_lua_pushstring td_lua_pushstring;

typedef void (*t_lua_createtable) (lua_State* L, int narr, int nrec);
extern t_lua_createtable td_lua_createtable;

typedef void (*t_InitScriptInnerLoop) (ScriptCoreInner* inner_core);
extern t_InitScriptInnerLoop td_InitScriptInnerLoop;

//typedef void (*t_ProcessVideoFrameGL) (ScreenCapture* sc, int frame);
//extern t_ProcessVideoFrameGL td_ProcessVideoFrameGL;

extern const char* luaPushStringPattern;
extern const char* luaCreateTablePattern;
extern const char* getFlashlightPattern;
extern const char* initScriptInnerLoopPattern;

#endif
