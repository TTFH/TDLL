#include <stdio.h>
#include <vector>
#include <zlib.h>

#include "extended_api.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

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
	uintptr_t GetSteer		= 0x1FC3A4;
	uintptr_t RenderDist	= 0x5063C8;
	uintptr_t Game			= 0x713090;
	uintptr_t ImguiCtx		= 0x719A60;
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
	lua_pushstring(L, "v1.5.3_01.07");
	return 1;
}

int GetWater(lua_State* L) {
	Game* game = (Game*)Teardown::GetGame();
	unsigned int n = game->scene->waters.getSize();
	lua_createtable(L, n, 0);
	for (unsigned int j = 0; j < n; j++) {
		Water* water = game->scene->waters[j];
        lua_pushinteger(L, j + 1);
		lua_pushinteger(L, water->self.handle);
        lua_settable(L, -3);
	}
	return 1;
}

int GetScripts(lua_State* L) {
	Game* game = (Game*)Teardown::GetGame();
	unsigned int n = game->scene->scripts.getSize();
	lua_createtable(L, n, 0);
	for (unsigned int j = 0; j < n; j++) {
		Script* script = game->scene->scripts[j];
		lua_pushinteger(L, j + 1);
		lua_pushinteger(L, script->self.handle);
		lua_settable(L, -3);
	}
	return 1;
}

int GetBoundaryVertices(lua_State* L) {
	Game* game = (Game*)Teardown::GetGame();
	unsigned int n = game->scene->boundary.getSize();
	lua_createtable(L, n, 0);
	for (unsigned int j = 0; j < n; j++) {
		Vertex vertex = game->scene->boundary[j];
		lua_pushinteger(L, j + 1);
		LuaPushVector(L, vertex.x, 0, vertex.y);
        lua_settable(L, -3);
	}
	return 1;
}

int GetVehicleWheels(lua_State* L) {
	unsigned int handle = lua_tointeger(L, 1);
	int i = 1;
	lua_createtable(L, 0, 0);
	Game* game = (Game*)Teardown::GetGame();
	for (unsigned int j = 0; j < game->scene->wheels.getSize(); j++) {
		Wheel* wheel = game->scene->wheels[j];
		if (wheel->vehicle->self.handle == handle) {
			lua_pushinteger(L, i++);
			lua_pushinteger(L, wheel->self.handle);
			lua_settable(L, -3);
		}
	}
	return 1;
}

int GetScriptPath(lua_State* L) {
	unsigned int handle = lua_tointeger(L, 1);
	Game* game = (Game*)Teardown::GetGame();
	for (unsigned int j = 0; j < game->scene->scripts.getSize(); j++) {
		Script* script = game->scene->scripts[j];
		if (script->self.handle == handle) {
			lua_pushstring(L, script->file_path1.c_str());
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
	for (unsigned int j = 0; j < game->scene->waters.getSize(); j++) {
		Water* water = game->scene->waters[j];
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
	for (unsigned int j = 0; j < game->scene->waters.getSize(); j++) {
		Water* water = game->scene->waters[j];
		if (water->self.handle == handle) {
			unsigned int n = water->vertices.getSize();
			lua_createtable(L, n, 0);
			for (unsigned int k = 0; k < n; k++) {
				Vertex vertex = water->vertices[k];
				lua_pushinteger(L, k + 1);
				LuaPushVector(L, vertex.x, 0, vertex.y);
				lua_settable(L, -3);
			}
			return 1;
		}
	}
	return 0;
}

int GetJointLocalBodyPos(lua_State* L) {
	unsigned int handle = lua_tointeger(L, 1);
	Game* game = (Game*)Teardown::GetGame();
	for (unsigned int j = 0; j < game->scene->joints.getSize(); j++) {
		Joint* joint = game->scene->joints[j];
		if (joint->self.handle == handle) {
			LuaPushVector(L, joint->local_pos_body1.x, joint->local_pos_body1.y, joint->local_pos_body1.z);
			LuaPushVector(L, joint->local_pos_body2.x, joint->local_pos_body2.y, joint->local_pos_body2.z);
			return 2;
		}
	}
	return 0;
}

int GetShapeTexture(lua_State* L) {
	unsigned int handle = lua_tointeger(L, 1);
	Game* game = (Game*)Teardown::GetGame();
	for (unsigned int j = 0; j < game->scene->shapes.getSize(); j++) {
		Shape* shape = game->scene->shapes[j];
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
	for (unsigned int j = 0; j < game->scene->shapes.getSize(); j++) {
		Shape* shape = game->scene->shapes[j];
		if (shape->self.handle == handle) {
			//LuaPushVector(L, shape->texture_offset.x, shape->texture_offset.y, shape->texture_offset.z);
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
	for (unsigned int j = 0; j < game->scene->shapes.getSize(); j++) {
		Shape* shape = game->scene->shapes[j];
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
	for (unsigned int j = 0; j < game->scene->shapes.getSize(); j++) {
		Shape* shape = game->scene->shapes[j];
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
		printf("Error opening file for writing.\n");
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
		printf("Error opening file for reading.\n");
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
