#ifndef _EXTENDED_API_H
#define _EXTENDED_API_H

#include "lua_utils.h"

extern uintptr_t moduleBase;

namespace MEM_OFFSET {
	extern uintptr_t GetSteer;
	extern uintptr_t RegisterGameFunctions;
	extern uintptr_t RegisterLuaFunction;
	extern uintptr_t RenderDist;
	extern uintptr_t Game;
	extern uintptr_t ImguiCtx;
}

namespace Teardown {
	uintptr_t GetReferenceTo(uintptr_t offset);
	uintptr_t GetPointerTo(uintptr_t offset);
	Game* GetGame();
}

int GetDllVersion(lua_State* L);

int GetWater(lua_State* L);

int GetScripts(lua_State* L);

int GetBoundaryVertices(lua_State* L);

int GetVehicleWheels(lua_State* L);

int GetScriptPath(lua_State* L);

int GetPlayerFlashlight(lua_State* L);

int GetWaterTransform(lua_State* L);

int GetWaterVertices(lua_State* L);

int GetJointLocalBodyPos(lua_State* L);

int GetShapeTexture(lua_State* L);

int GetTextureOffset(lua_State* L);

int SetShapeTexture(lua_State* L);

int SetTextureOffset(lua_State* L);

int ZlibSaveCompressed(lua_State* L);

int ZlibLoadCompressed(lua_State* L);

#endif
