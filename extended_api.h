#ifndef _EXTENDED_API_H
#define _EXTENDED_API_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "lua_utils.h"

extern HMODULE moduleBase;

namespace MEM_OFFSET {
	extern uintptr_t Game;
	extern uintptr_t RenderDist;
	extern uintptr_t LuaPushString;
	extern uintptr_t LuaCreateTable;
	extern uintptr_t ProcessVideoFrameOGL;
	extern uintptr_t RegisterGameFunctions;
}

namespace Teardown {
	uintptr_t GetReferenceTo(uintptr_t offset);
	uintptr_t GetPointerTo(uintptr_t offset);
	Game* GetGame();
}

extern bool clock_init[16];

void RegisterLuaCFunctions(lua_State* L);

#endif
