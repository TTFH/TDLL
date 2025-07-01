#include "signatures.h"

uintptr_t MEM_OFFSET::Game = 0;
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

const char* getFlashlightPattern =
	"48 8B 05 ????????"
	"48 8B 48 50"
	"48 8B 91 80000000"
	"49 8B C8"
	"8B 52 0C";

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
