#include <stdio.h>
#include <vector>
#include <zlib.h>

#include "extended_api.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

t_lua_createtable td_lua_createtable = nullptr;

uintptr_t FindDMAAddy(uintptr_t ptr, std::vector<uintptr_t> offsets) {
	uintptr_t addr = ptr;
	for (size_t i = 0; i < offsets.size(); i++) {
		addr = *(uintptr_t*)addr;
		if (IsBadReadPtr((uintptr_t*)addr, sizeof(addr)))
			return 0;
		addr += offsets[i];
	}
	return addr;
}

namespace MEM_OFFSET {
	uintptr_t GetSteer				= 0x1FC3A4;
	uintptr_t RegisterGameFunctions	= 0x215A00;
	uintptr_t LuaCreateTable		= 0x3897A0;
	uintptr_t RenderDist			= 0x5063C8;
	uintptr_t Game					= 0x713090;
	uintptr_t ImguiCtx				= 0x719A60;
}

namespace Teardown {
	uintptr_t GetReferenceTo(uintptr_t offset) {
		return FindDMAAddy(moduleBase + offset, { });
	}
	uintptr_t GetPointerTo(uintptr_t offset) {
		return FindDMAAddy(moduleBase + offset, { 0x00 });
	}
	Game* GetGame() {
		return (Game*)GetPointerTo(MEM_OFFSET::Game);
	}
}

int GetDllVersion(lua_State* L) {
	lua_pushstring(L, "v1.5.3.112");
	return 1;
}

int GetWater(lua_State* L) {
	Game* game = (Game*)Teardown::GetGame();
	unsigned int n = game->scene->waters.getSize();
	td_lua_createtable(L, n, 0);
	for (unsigned int i = 0; i < n; i++) {
		Water* water = game->scene->waters[i];
		lua_pushinteger(L, water->self.handle);
		lua_rawseti(L, -2, i + 1);
	}
	return 1;
}

int GetScripts(lua_State* L) {
	Game* game = (Game*)Teardown::GetGame();
	unsigned int n = game->scene->scripts.getSize();
	td_lua_createtable(L, n, 0);
	for (unsigned int i = 0; i < n; i++) {
		Script* script = game->scene->scripts[i];
		lua_pushinteger(L, script->self.handle);
		lua_rawseti(L, -2, i + 1);
	}
	return 1;
}

int GetBoundaryVertices(lua_State* L) {
	Game* game = (Game*)Teardown::GetGame();
	unsigned int n = game->scene->boundary.getSize();
	td_lua_createtable(L, n, 0);
	for (unsigned int i = 0; i < n; i++) {
		Vertex vertex = game->scene->boundary[i];
		LuaPushVector(L, Vector(vertex.x, 0, vertex.y));
		lua_rawseti(L, -2, i + 1);
	}
	return 1;
}

int GetVehicleWheels(lua_State* L) {
	unsigned int handle = lua_tointeger(L, 1);
	std::vector<int> wheels;
	Game* game = (Game*)Teardown::GetGame();
	for (unsigned int i = 0; i < game->scene->wheels.getSize(); i++) {
		Wheel* wheel = game->scene->wheels[i];
		if (wheel->vehicle->self.handle == handle)
			wheels.push_back(wheel->self.handle);
	}
	LuaPushList(L, wheels);
	return 1;
}

int GetScriptPath(lua_State* L) {
	unsigned int handle = lua_tointeger(L, 1);
	Game* game = (Game*)Teardown::GetGame();
	for (unsigned int i = 0; i < game->scene->scripts.getSize(); i++) {
		Script* script = game->scene->scripts[i];
		if (script->self.handle == handle) {
			lua_pushstring(L, script->path.c_str());
			return 1;
		}
	}
	return 0;
}

int GetPlayerFlashlight(lua_State* L) {
	Game* game = (Game*)Teardown::GetGame();
	lua_pushinteger(L, game->scene->flashlight->self.handle);
	return 1;
}

int GetWaterTransform(lua_State* L) {
	unsigned int handle = lua_tointeger(L, 1);
	Game* game = (Game*)Teardown::GetGame();
	for (unsigned int i = 0; i < game->scene->waters.getSize(); i++) {
		Water* water = game->scene->waters[i];
		if (water->self.handle == handle) {
			LuaPushTransform(L, water->transform);
			return 1;
		}
	}
	return 0;
}

int GetWaterVertices(lua_State* L) {
	unsigned int handle = lua_tointeger(L, 1);
	Game* game = (Game*)Teardown::GetGame();
	for (unsigned int i = 0; i < game->scene->waters.getSize(); i++) {
		Water* water = game->scene->waters[i];
		if (water->self.handle == handle) {
			unsigned int n = water->vertices.getSize();
			td_lua_createtable(L, n, 0);
			for (unsigned int j = 0; j < n; j++) {
				Vertex vertex = water->vertices[j];
				LuaPushVector(L, Vector(vertex.x, 0, vertex.y));
				lua_rawseti(L, -2, j + 1);
			}
			return 1;
		}
	}
	return 0;
}

int GetJointLocalBodyPos(lua_State* L) {
	unsigned int handle = lua_tointeger(L, 1);
	Game* game = (Game*)Teardown::GetGame();
	for (unsigned int i = 0; i < game->scene->joints.getSize(); i++) {
		Joint* joint = game->scene->joints[i];
		if (joint->self.handle == handle) {
			LuaPushVector(L, joint->local_pos_body1);
			//LuaPushVector(L, joint->local_pos_body2);
			return 1;
		}
	}
	return 0;
}

int GetShapeTexture(lua_State* L) {
	unsigned int handle = lua_tointeger(L, 1);
	Game* game = (Game*)Teardown::GetGame();
	for (unsigned int i = 0; i < game->scene->shapes.getSize(); i++) {
		Shape* shape = game->scene->shapes[i];
		if (shape->self.handle == handle) {
			lua_pushinteger(L, shape->texture_tile);
			lua_pushnumber(L, shape->texture_weight);
			lua_pushinteger(L, shape->blendtexture_tile);
			lua_pushnumber(L, shape->blendtexture_weight);
			return 4;
		}
	}
	return 0;
}

int GetTextureOffset(lua_State* L) {
	unsigned int handle = lua_tointeger(L, 1);
	Game* game = (Game*)Teardown::GetGame();
	for (unsigned int i = 0; i < game->scene->shapes.getSize(); i++) {
		Shape* shape = game->scene->shapes[i];
		if (shape->self.handle == handle) {
			//LuaPushVector(L, shape->texture_offset);
			//return 1;
			lua_pushnumber(L, shape->texture_offset.x);
			lua_pushnumber(L, shape->texture_offset.y);
			lua_pushnumber(L, shape->texture_offset.z);
			return 3;
		}
	}
	return 0;
}

int SetShapeTexture(lua_State* L) {
	unsigned int handle = lua_tointeger(L, 1);
	unsigned int texture_tile = lua_tointeger(L, 2);
	float texture_weight = lua_tonumber(L, 3);
	unsigned int blendtexture_tile = lua_tointeger(L, 4);
	float blendtexture_weight = lua_tonumber(L, 5);

	Game* game = (Game*)Teardown::GetGame();
	for (unsigned int i = 0; i < game->scene->shapes.getSize(); i++) {
		Shape* shape = game->scene->shapes[i];
		if (shape->self.handle == handle) {
			shape->texture_tile = texture_tile;
			shape->blendtexture_tile = blendtexture_tile;
			shape->texture_weight = texture_weight;
			shape->blendtexture_weight = blendtexture_weight;
			return 0;
		}
	}
	return 0;
}

int SetTextureOffset(lua_State* L) {
	unsigned int handle = lua_tointeger(L, 1);
	float x = lua_tonumber(L, 2);
	float y = lua_tonumber(L, 3);
	float z = lua_tonumber(L, 4);

	Game* game = (Game*)Teardown::GetGame();
	for (unsigned int i = 0; i < game->scene->shapes.getSize(); i++) {
		Shape* shape = game->scene->shapes[i];
		if (shape->self.handle == handle) {
			shape->texture_offset.x = x;
			shape->texture_offset.y = y;
			shape->texture_offset.z = z;
			return 0;
		}
	}
	return 0;
}

int ZlibSaveCompressed(lua_State* L) {
	const char* filename = lua_tostring(L, 1);
	const char* str = lua_tostring(L, 2);

	unsigned long src_length = strlen(str) + 1;
	uLong dest_length = compressBound(src_length);
	Bytef* dest = new Bytef[dest_length];
	if (dest == NULL) {
		printf("Memory allocation failed.\n");
		return 0;
	}

	int result = compress(dest, &dest_length, (const Bytef*)str, src_length);
	if (result != Z_OK) {
		printf("Compression failed with error code %d\n", result);
		delete[] dest;
		return 0;
	}

	FILE* file = fopen(filename, "wb");
	if (file == NULL) {
		printf("Error opening file %s for writing.\n", filename);
		delete[] dest;
		return 0;
	}

	fwrite(dest, 1, dest_length, file);
	fclose(file);
	delete[] dest;
	return 0;
}

const int BLOCK_SIZE = 8 * 1024; // 8 KiB

int ZlibLoadCompressed(lua_State* L) {
	const char* filename = lua_tostring(L, 1);

	FILE* file = fopen(filename, "rb");
	if (file == NULL) {
		printf("Error opening file %s for reading.\n", filename);
		return 0;
	}

	fseek(file, 0, SEEK_END);
	long file_size = ftell(file);
	fseek(file, 0, SEEK_SET);
	
	Bytef* compressed_data = new Bytef[file_size];
	if (compressed_data == NULL) {
		printf("Memory allocation failed.\n");
		fclose(file);
		return 0;
	}
	
	fread(compressed_data, 1, file_size, file);
	fclose(file);
	
	uLongf dest_length = BLOCK_SIZE;
	std::vector<Bytef> dest(dest_length);

	int result;
	do {
		result = uncompress(dest.data(), &dest_length, compressed_data, file_size);
		if (result == Z_BUF_ERROR) {
			dest_length *= 2;
			dest.resize(dest_length);
		} else if (result != Z_OK) {
			printf("Decompression failed with error code %d\n", result);
			delete[] compressed_data;
			return 0;
		}
	} while (result == Z_BUF_ERROR);

	delete[] compressed_data;
	dest.resize(dest_length + 1);
	dest[dest_length] = '\0';
	lua_pushstring(L, (const char*)dest.data());
	return 1;
}
