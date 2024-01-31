#include <stdio.h>
#include <vector>
#include <zlib.h>

#include "extended_api.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

t_lua_createtable td_lua_createtable = nullptr;
t_lua_pushstring td_lua_pushstring = nullptr;

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

namespace MEM_OFFSET {				// Addr		// Type
	uintptr_t GetSteer				= 0x1FC3A4; // lua_pushinteger(vehicle->steer)
	uintptr_t RegisterGameFunctions	= 0x215A00; // void fun(ScriptCore*)
	uintptr_t LuaCreateTable		= 0x3897A0; // void fun(lua_State*, int, int)
	uintptr_t LuaPushString			= 0x38A9E0; // void fun(lua_State*, const char*)
	uintptr_t RenderDist			= 0x5063C8; // float
	uintptr_t Game					= 0x713090; // Game*
	uintptr_t ImguiCtx				= 0x719A60; // ImGuiContext*
}

namespace Teardown {
	uintptr_t GetReferenceTo(uintptr_t offset) {
		return moduleBase + offset;
	}
	uintptr_t GetPointerTo(uintptr_t offset) {
		return FindDMAAddy(moduleBase + offset, { 0x00 });
	}
	Game* GetGame() {
		return (Game*)GetPointerTo(MEM_OFFSET::Game);
	}
}

void SkipIsInternalFunctionCheck() {
	Game* game = (Game*)Teardown::GetGame();
	unsigned int n = game->mod_data->external_scripts.getSize();
	for (unsigned int i = 0; i < n; i++) {
		ExternalScript* script = game->mod_data->external_scripts[i];
		if (script->privilege > 1)
			script->privilege = 1;
	}
}

int GetDllVersion(lua_State* L) {
	td_lua_pushstring(L, "v1.5.3.118");
	return 1;
}

int AllowInternalFunctions(lua_State* L) {
	SkipIsInternalFunctionCheck();
	return 0;
}

int GetWaters(lua_State* L) {
	Game* game = (Game*)Teardown::GetGame();
	unsigned int n = game->scene->waters.getSize();
	td_lua_createtable(L, n, 0);
	for (unsigned int i = 0; i < n; i++) {
		Water* water = game->scene->waters[i];
		lua_pushinteger(L, water->handle);
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
		lua_pushinteger(L, script->handle);
		lua_rawseti(L, -2, i + 1);
	}
	return 1;
}

int GetWheels(lua_State* L) {
	Game* game = (Game*)Teardown::GetGame();
	unsigned int n = game->scene->wheels.getSize();
	td_lua_createtable(L, n, 0);
	for (unsigned int i = 0; i < n; i++) {
		Wheel* wheel = game->scene->wheels[i];
		lua_pushinteger(L, wheel->handle);
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
		if (wheel->vehicle->handle == handle)
			wheels.push_back(wheel->handle);
	}
	LuaPushList(L, wheels);
	return 1;
}

int GetScriptEntities(lua_State* L) {
	unsigned int handle = lua_tointeger(L, 1);
	Game* game = (Game*)Teardown::GetGame();
	for (unsigned int i = 0; i < game->scene->scripts.getSize(); i++) {
		Script* script = game->scene->scripts[i];
		if (script->handle == handle) {
			unsigned int n = script->core.entities.getSize();
			td_lua_createtable(L, n, 0);
			for (unsigned int j = 0; j < n; j++) {
				int entity_handle = script->core.entities[j];
				lua_pushinteger(L, entity_handle);
				lua_rawseti(L, -2, j + 1);
			}
			return 1;
		}
	}
	td_lua_createtable(L, 0, 0);
	return 1;
}

int GetWheelVehicle(lua_State* L) {
	unsigned int handle = lua_tointeger(L, 1);
	Game* game = (Game*)Teardown::GetGame();
	for (unsigned int i = 0; i < game->scene->wheels.getSize(); i++) {
		Wheel* wheel = game->scene->wheels[i];
		if (wheel->handle == handle) {
			lua_pushinteger(L, wheel->vehicle->handle);
			return 1;
		}
	}
	lua_pushinteger(L, 0);
	return 1;
}

int GetScriptPath(lua_State* L) {
	unsigned int handle = lua_tointeger(L, 1);
	Game* game = (Game*)Teardown::GetGame();
	for (unsigned int i = 0; i < game->scene->scripts.getSize(); i++) {
		Script* script = game->scene->scripts[i];
		if (script->handle == handle) {
			td_lua_pushstring(L, script->path.c_str());
			return 1;
		}
	}
	td_lua_pushstring(L, "");
	return 1;
}

int GetPlayerFlashlight(lua_State* L) {
	Game* game = (Game*)Teardown::GetGame();
	lua_pushinteger(L, game->scene->flashlight->handle);
	return 1;
}

int GetShadowVolumeSize(lua_State* L) {
	Game* game = (Game*)Teardown::GetGame();
	LuaPushVector(L, game->scene->sv_size);
	return 1;
}

int GetWaterTransform(lua_State* L) {
	unsigned int handle = lua_tointeger(L, 1);
	Game* game = (Game*)Teardown::GetGame();
	for (unsigned int i = 0; i < game->scene->waters.getSize(); i++) {
		Water* water = game->scene->waters[i];
		if (water->handle == handle) {
			LuaPushTransform(L, water->transform);
			return 1;
		}
	}
	LuaPushTransform(L, Transform());
	return 1;
}

int GetWaterVertices(lua_State* L) {
	unsigned int handle = lua_tointeger(L, 1);
	Game* game = (Game*)Teardown::GetGame();
	for (unsigned int i = 0; i < game->scene->waters.getSize(); i++) {
		Water* water = game->scene->waters[i];
		if (water->handle == handle) {
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
	td_lua_createtable(L, 0, 0);
	return 1;
}

int GetTriggerType(lua_State* L) {
	unsigned int handle = lua_tointeger(L, 1);
	Game* game = (Game*)Teardown::GetGame();
	for (unsigned int i = 0; i < game->scene->triggers.getSize(); i++) {
		Trigger* trigger = game->scene->triggers[i];
		if (trigger->handle == handle) {
			switch (trigger->type) {
			case TrSphere:
				td_lua_pushstring(L, "sphere");
				break;
			case TrBox:
				td_lua_pushstring(L, "box");
				break;
			case TrPolygon:
				td_lua_pushstring(L, "polygon");
				break;
			}
			return 1;
		}
	}
	td_lua_pushstring(L, "");
	return 1;
}

int GetTriggerSize(lua_State* L) {
	unsigned int handle = lua_tointeger(L, 1);
	Game* game = (Game*)Teardown::GetGame();
	for (unsigned int i = 0; i < game->scene->triggers.getSize(); i++) {
		Trigger* trigger = game->scene->triggers[i];
		if (trigger->handle == handle) {
			switch (trigger->type) {
			case TrSphere:
				lua_pushnumber(L, trigger->sphere_size);
				break;
			case TrBox:
				LuaPushVector(L, trigger->half_box_size * 2);
				break;
			case TrPolygon:
				lua_pushnumber(L, trigger->polygon_size);
				break;
			}
			return 1;
		}
	}
	lua_pushnumber(L, 0);
	return 1;
}

int GetTriggerVertices(lua_State* L) {
	unsigned int handle = lua_tointeger(L, 1);
	Game* game = (Game*)Teardown::GetGame();
	for (unsigned int i = 0; i < game->scene->triggers.getSize(); i++) {
		Trigger* trigger = game->scene->triggers[i];
		if (trigger->handle == handle) {
			unsigned int n = trigger->vertices.getSize();
			td_lua_createtable(L, n, 0);
			for (unsigned int j = 0; j < n; j++) {
				Vertex vertex = trigger->vertices[j];
				LuaPushVector(L, Vector(vertex.x, 0, vertex.y));
				lua_rawseti(L, -2, j + 1);
			}
			return 1;
		}
	}
	td_lua_createtable(L, 0, 0);
	return 1;
}

int GetJointLocalPosAndAxis(lua_State* L) {
	unsigned int handle = lua_tointeger(L, 1);
	unsigned int index = lua_tointeger(L, 2);
	Game* game = (Game*)Teardown::GetGame();
	for (unsigned int i = 0; i < game->scene->joints.getSize(); i++) {
		Joint* joint = game->scene->joints[i];
		if (joint->handle == handle) {
			if (index == 1) {
				LuaPushVector(L, joint->local_pos1);
				LuaPushVector(L, joint->local_rot1);
			} else {
				LuaPushVector(L, joint->local_pos2);
				LuaPushVector(L, joint->local_rot2);
			}
			return 2;
		}
	}
	LuaPushVector(L, Vector());
	LuaPushVector(L, Vector(0, 1, 0));
	return 2;
}

int GetJointParams(lua_State* L) {
	unsigned int handle = lua_tointeger(L, 1);
	Game* game = (Game*)Teardown::GetGame();
	for (unsigned int i = 0; i < game->scene->joints.getSize(); i++) {
		Joint* joint = game->scene->joints[i];
		if (joint->handle == handle) {
			lua_pushboolean(L, joint->collide);
			lua_pushboolean(L, joint->sound);
			lua_pushboolean(L, joint->autodisable);
			return 3;
		}
	}
	lua_pushboolean(L, false);
	lua_pushboolean(L, false);
	lua_pushboolean(L, false);
	return 3;
}

int GetRopeColor(lua_State* L) {
	unsigned int handle = lua_tointeger(L, 1);
	Game* game = (Game*)Teardown::GetGame();
	for (unsigned int i = 0; i < game->scene->joints.getSize(); i++) {
		Joint* joint = game->scene->joints[i];
		if (joint->handle == handle && joint->type == _Rope) {
			lua_pushnumber(L, joint->rope->color.r);
			lua_pushnumber(L, joint->rope->color.g);
			lua_pushnumber(L, joint->rope->color.b);
			return 3;
		}
	}
	lua_pushnumber(L, 0);
	lua_pushnumber(L, 0);
	lua_pushnumber(L, 0);
	return 3;
}

int GetShapePaletteIndex(lua_State* L) {
	unsigned int handle = lua_tointeger(L, 1);
	Game* game = (Game*)Teardown::GetGame();
	for (unsigned int i = 0; i < game->scene->shapes.getSize(); i++) {
		Shape* shape = game->scene->shapes[i];
		if (shape->handle == handle) {
			lua_pushinteger(L, shape->vox->palette);
			return 1;
		}
	}
	lua_pushinteger(L, 0);
	return 1;
}

int GetShapeTexture(lua_State* L) {
	unsigned int handle = lua_tointeger(L, 1);
	Game* game = (Game*)Teardown::GetGame();
	for (unsigned int i = 0; i < game->scene->shapes.getSize(); i++) {
		Shape* shape = game->scene->shapes[i];
		if (shape->handle == handle) {
			lua_pushinteger(L, shape->texture_tile);
			lua_pushnumber(L, shape->texture_weight);
			lua_pushinteger(L, shape->blendtexture_tile);
			lua_pushnumber(L, shape->blendtexture_weight);
			return 4;
		}
	}
	lua_pushinteger(L, 0);
	lua_pushnumber(L, 1);
	lua_pushinteger(L, 0);
	lua_pushnumber(L, 1);
	return 4;
}

int GetTextureOffset(lua_State* L) {
	unsigned int handle = lua_tointeger(L, 1);
	Game* game = (Game*)Teardown::GetGame();
	for (unsigned int i = 0; i < game->scene->shapes.getSize(); i++) {
		Shape* shape = game->scene->shapes[i];
		if (shape->handle == handle) {
			LuaPushVector(L, shape->texture_offset);
			return 1;
		}
	}
	LuaPushVector(L, Vector());
	return 1;
}

int SetShapePalette(lua_State* L) {
	unsigned int handle = lua_tointeger(L, 1);
	unsigned int palette = lua_tointeger(L, 2);

	Game* game = (Game*)Teardown::GetGame();
	for (unsigned int i = 0; i < game->scene->shapes.getSize(); i++) {
		Shape* shape = game->scene->shapes[i];
		if (shape->handle == handle) {
			shape->vox->palette = palette;
			return 0;
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
		if (shape->handle == handle) {
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
	Vector offset = LuaToVector(L, 2);

	Game* game = (Game*)Teardown::GetGame();
	for (unsigned int i = 0; i < game->scene->shapes.getSize(); i++) {
		Shape* shape = game->scene->shapes[i];
		if (shape->handle == handle) {
			shape->texture_offset = offset;
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
	if (dest == nullptr) {
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
	if (file == nullptr) {
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
	if (file == nullptr) {
		printf("Error opening file %s for reading.\n", filename);
		return 0;
	}

	fseek(file, 0, SEEK_END);
	long file_size = ftell(file);
	fseek(file, 0, SEEK_SET);
	
	Bytef* compressed_data = new Bytef[file_size];
	if (compressed_data == nullptr) {
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
	td_lua_pushstring(L, (const char*)dest.data());
	return 1;
}

void RegisterLuaCFunctions(lua_State* L) {
	lua_pushcfunction(L, GetDllVersion);
	lua_setglobal(L, "GetDllVersion");
	lua_pushcfunction(L, AllowInternalFunctions);
	lua_setglobal(L, "AllowInternalFunctions");

	lua_pushcfunction(L, GetShadowVolumeSize);
	lua_setglobal(L, "GetShadowVolumeSize");
	lua_pushcfunction(L, GetBoundaryVertices);
	lua_setglobal(L, "GetBoundaryVertices");
	lua_pushcfunction(L, GetPlayerFlashlight);
	lua_setglobal(L, "GetPlayerFlashlight");

	lua_pushcfunction(L, GetJointLocalPosAndAxis);
	lua_setglobal(L, "GetJointLocalPosAndAxis");
	lua_pushcfunction(L, GetJointParams);
	lua_setglobal(L, "GetJointParams");
	lua_pushcfunction(L, GetRopeColor);
	lua_setglobal(L, "GetRopeColor");

	lua_pushcfunction(L, GetWaters);
	lua_setglobal(L, "GetWaters");
	lua_pushcfunction(L, GetWaterTransform);
	lua_setglobal(L, "GetWaterTransform");
	lua_pushcfunction(L, GetWaterVertices);
	lua_setglobal(L, "GetWaterVertices");

	lua_pushcfunction(L, GetScripts);
	lua_setglobal(L, "GetScripts");
	lua_pushcfunction(L, GetScriptPath);
	lua_setglobal(L, "GetScriptPath");
	lua_pushcfunction(L, GetScriptEntities);
	lua_setglobal(L, "GetScriptEntities");

	lua_pushcfunction(L, GetWheels);
	lua_setglobal(L, "GetWheels");
	lua_pushcfunction(L, GetWheelVehicle);
	lua_setglobal(L, "GetWheelVehicle");
	lua_pushcfunction(L, GetVehicleWheels);
	lua_setglobal(L, "GetVehicleWheels");

	lua_pushcfunction(L, GetTriggerType);
	lua_setglobal(L, "GetTriggerType");
	lua_pushcfunction(L, GetTriggerSize);
	lua_setglobal(L, "GetTriggerSize");
	lua_pushcfunction(L, GetTriggerVertices);
	lua_setglobal(L, "GetTriggerVertices");

	lua_pushcfunction(L, GetShapePaletteIndex);
	lua_setglobal(L, "GetShapePaletteIndex");
	lua_pushcfunction(L, GetShapeTexture);
	lua_setglobal(L, "GetShapeTexture");
	lua_pushcfunction(L, GetTextureOffset);
	lua_setglobal(L, "GetTextureOffset");
	lua_pushcfunction(L, SetShapePalette);
	lua_setglobal(L, "SetShapePalette");
	lua_pushcfunction(L, SetShapeTexture);
	lua_setglobal(L, "SetShapeTexture");
	lua_pushcfunction(L, SetTextureOffset);
	lua_setglobal(L, "SetTextureOffset");

	lua_pushcfunction(L, ZlibSaveCompressed);
	lua_setglobal(L, "ZlibSaveCompressed");
	lua_pushcfunction(L, ZlibLoadCompressed);
	lua_setglobal(L, "ZlibLoadCompressed");
}
