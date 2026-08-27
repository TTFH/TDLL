#include "signatures.h"

//uintptr_t MEM_OFFSET::Game = 0;
Game* game_ptr = nullptr;
t_lua_pushstring td_lua_pushstring = nullptr;
t_lua_createtable td_lua_createtable = nullptr;
t_InitScriptInnerLoop td_InitScriptInnerLoop = nullptr;

const char* luaPushStringPattern =
	"48 85 D2"
	"75 0D"
	"48 8B 41 10"
	"89 50 08"
	"48 83 41 10 10"
	"C3"
	"49 C7 C0 FFFFFFFF"
	"0F1F 80 00000000"
	"49 FF C0"
	"42 80 3C 02 00"
	"75 F6"
	"E9 21FFFFFF";

const char* luaCreateTablePattern =
	"48 89 5C 24 08"
	"48 89 6C 24 10"
	"48 89 74 24 18"
	"57"
	"48 83 EC 20"
	"4C 8B 49 20"
	"41 8B F0"
	"8B EA"
	"48 8B F9"
	"49 8B 41 70"
	"49 39 41 78"
	"72 05";
/*
const char* getFlashlightPattern =
	"48 8B 05 ????????"
	"48 8B 48 50"
	"48 8B 91 80000000"
	"49 8B C8"
	"8B 52 0C"
	"E9 93 FF 19 00";
*/
const char* getBodyMassPattern =
	"40 53"
	"48 83 EC 20"
	"49 8B D8"
	"48 8B CA"
	"45 33 C0"
	"33 D2"
	"E8 ????????"		// CALL LuaReadInt
	"48 8B 0D ????????" // MOV RCX, [game]
	"0F 57 C9";

const char* initScriptInnerLoopPattern =
	"48 89 5C 24 08"
	"48 89 74 24 10"
	"57"
	"48 83 EC 40"
	"48 8B F1"
	"48 8B 1D ????????"
	"48 85 DB"
	"74 38"
	"66 90"
	"48 8B 3B"
	"48 8B 53 08"
	"48 8D 4C 24 20";

// What the game uses to register a lua function for a ScriptCoreInner
const char* scriptRegisterFunctionPattern = "48 89 5c 24 08 57 48 83 ec 20 48 8b f9 48 8b da 48 8b 49 38 49 8b d0 48 8b 09 e8 ?? ?? ?? ?? 48 8b 4f 38 48 8b d7 48 8b 09 e8 ?? ?? ?? ?? 48 8b 4f 38 48 ?? ?? ?? ?? ?? ?? 41 b8 02 00 00 00 48 8b 09 e8 ?? ?? ?? ?? 80 7b 1f 00 74 ?? 48 8b 1b";

// Responsible for registering some of the lua api functions
// The pattern for this actually matches both the client and shared version of this but doesn't really matter
extern const char* scriptRegisterSharedFunctionsPattern = "48 89 5c 24 08 48 89 74 24 10 57 48 83 ec 40 48 8b f1 48 ?? ?? ?? ?? ?? ?? 48 85 db 74 38 66 90 48 8b 3b 48 8b 53 08 48 8d 4c 24 20 e8 ?? ?? ?? ?? 90 4c 8b c7 48 8d 54 24 20 48 8b ce e8 ?? ?? ?? ?? 90 48 8d 4c 24 20 e8 ?? ?? ?? ?? 48 8b 5b 10 48 85 db 75 ?? 48 8b 5c 24 50 48 8b 74 24 58 48 83 c4 40 5f c3";