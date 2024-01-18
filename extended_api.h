#ifndef _EXTENDED_API_H
#define _EXTENDED_API_H

#include "lua_utils.h"

extern uintptr_t moduleBase;

namespace MEM_OFFSET {
	extern uintptr_t GetSteer;
	extern uintptr_t RegisterGameFunctions;
	extern uintptr_t LuaCreateTable;
	extern uintptr_t LuaPushString;
	extern uintptr_t RenderDist;
	extern uintptr_t Game;
	extern uintptr_t ImguiCtx;
}

namespace Teardown {
	uintptr_t GetReferenceTo(uintptr_t offset);
	uintptr_t GetPointerTo(uintptr_t offset);
	Game* GetGame();
}

void RegisterLuaCFunctions(lua_State* L);

#endif
