#ifndef _EXTENDED_API_H
#define _EXTENDED_API_H

#include <mutex>
#include <string>
#include <vector>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "lua_utils.h"
#include "networking.h"
#include "signatures.h"

extern HMODULE moduleBase;

namespace Teardown {
	uintptr_t GetReferenceTo(uintptr_t offset);
	uintptr_t GetPointerTo(uintptr_t offset);
	Game* GetGame();
}

extern Broadcast broadcast;
extern std::mutex msg_mutex;
extern std::vector<std::string> messages;

void RegisterLuaCFunctions(lua_State* L);

#endif
