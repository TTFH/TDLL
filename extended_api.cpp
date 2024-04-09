#include <stdio.h>
#include <chrono>
#include <vector>
#include <zlib.h>

#include "extended_api.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

using hrc = std::chrono::high_resolution_clock;
hrc::time_point clocks[16];
bool clock_init[16] = { false };

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
	uintptr_t RegisterGameFunctions	= 0x230A80; // void fun(ScriptCore*)
	uintptr_t LuaCreateTable		= 0x39A5D0; // void fun(lua_State*, int, int)
	uintptr_t LuaPushString			= 0x39B820; // void fun(lua_State*, const char*)
	uintptr_t RenderDist			= 0x6D9DF8; // float
	uintptr_t Game					= 0x92C100; // Game*
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
	td_lua_pushstring(L, "v1.5.4.0408");
	return 1;
}

int AllowInternalFunctions(lua_State* L) {
	SkipIsInternalFunctionCheck();
	return 0;
}

int Tick(lua_State* L) {
	unsigned int index = lua_tointeger(L, 1);
	clocks[index] = hrc::now();
	clock_init[index] = true;
	return 0;
}

int Tock(lua_State* L) {
	unsigned int index = lua_tointeger(L, 1);
	if (clock_init[index]) {
		hrc::time_point now = hrc::now();
		double elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(now - clocks[index]).count();
		lua_pushnumber(L, elapsed);
	} else
		lua_pushnumber(L, 0);
	return 1;
}

int HttpRequest(lua_State* L) {
	std::map<std::string, std::string> headers;
	const char* method = lua_tostring(L, 1);
	const char* endpoint = lua_tostring(L, 2);
	if (lua_istable(L, 3)) {
		lua_pushnil(L);
		while (lua_next(L, 3) != 0) {
			const char* key = lua_tostring(L, -2);
			const char* value = lua_tostring(L, -1);
			headers[key] = value;
			lua_pop(L, 1);
		}
	}
	const char* request = lua_tostring(L, 4);
	const char* cookies = lua_tostring(L, 5);

	std::string response;
	int status = HttpRequest(method, endpoint, headers, request, cookies, response);
	lua_pushinteger(L, status);
	td_lua_pushstring(L, response.c_str());
	return 2;
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

static const char* MaterialKindName[] = {
	"none",
	"glass",
	"wood",
	"masonry",
	"plaster",
	"metal",
	"heavymetal",
	"rock",
	"dirt",
	"foliage",
	"plastic",
	"hardmetal",
	"hardmasonry",
	"ice",
	"unphysical"
};

int GetPaletteMaterial(lua_State* L) {
	unsigned int palette_id = lua_tointeger(L, 1);
	unsigned int index = lua_tointeger(L, 2);

	if (index > 255)
		return 0;

	Game* game = (Game*)Teardown::GetGame();
	unsigned int palette_count = game->palettes->getSize();
	if (palette_id >= palette_count)
		return 0;

	Material material = game->palettes->get(palette_id).materials[index];
	td_lua_pushstring(L, MaterialKindName[material.kind]);
	lua_pushnumber(L, material.color.r);
	lua_pushnumber(L, material.color.g);
	lua_pushnumber(L, material.color.b);
	lua_pushnumber(L, material.color.a);
	lua_pushnumber(L, material.reflectivity);
	lua_pushnumber(L, material.shininess);
	lua_pushnumber(L, material.metallic);
	lua_pushnumber(L, material.emissive);
	return 9;
}

int GetShapePaletteId(lua_State* L) {
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
	LuaPushFuntion(L, "GetDllVersion", GetDllVersion);
	LuaPushFuntion(L, "Tick", Tick);
	LuaPushFuntion(L, "Tock", Tock);
	LuaPushFuntion(L, "HttpRequest", HttpRequest);
	LuaPushFuntion(L, "ZlibSaveCompressed", ZlibSaveCompressed);
	LuaPushFuntion(L, "ZlibLoadCompressed", ZlibLoadCompressed);

	LuaPushFuntion(L, "AllowInternalFunctions", AllowInternalFunctions);

	LuaPushFuntion(L, "GetShadowVolumeSize", GetShadowVolumeSize);
	LuaPushFuntion(L, "GetBoundaryVertices", GetBoundaryVertices);
	//LuaPushFuntion(L, "GetPlayerFlashlight", GetPlayerFlashlight); // GetFlashlight()

	//LuaPushFuntion(L, "GetWaters", GetWaters); // FindEntities("", true, "water")
	LuaPushFuntion(L, "GetWaterTransform", GetWaterTransform);
	LuaPushFuntion(L, "GetWaterVertices", GetWaterVertices);

	//LuaPushFuntion(L, "GetScripts", GetScripts); // FindEntities("", true, "script")
	LuaPushFuntion(L, "GetScriptPath", GetScriptPath);
	LuaPushFuntion(L, "GetScriptEntities", GetScriptEntities);

	//LuaPushFuntion(L, "GetWheels", GetWheels); // FindEntities("", true, "wheel")
	//LuaPushFuntion(L, "GetWheelVehicle", GetWheelVehicle); // GetEntityParent(wheel, "", "vehicle")
	//LuaPushFuntion(L, "GetVehicleWheels", GetVehicleWheels); // GetEntityChildren(vehicle, "", true, "wheel")

	//LuaPushFuntion(L, "GetTriggerType", GetTriggerType); // GetProperty(trigger, "type")
	//LuaPushFuntion(L, "GetTriggerSize", GetTriggerSize); // GetProperty(trigger, "size")
	LuaPushFuntion(L, "GetTriggerVertices", GetTriggerVertices);

	LuaPushFuntion(L, "GetJointLocalPosAndAxis", GetJointLocalPosAndAxis);
	LuaPushFuntion(L, "GetJointParams", GetJointParams);
	//LuaPushFuntion(L, "GetRopeColor", GetRopeColor); // GetProperty(rope, "ropecolor")

	LuaPushFuntion(L, "GetPaletteMaterial", GetPaletteMaterial);
	LuaPushFuntion(L, "GetShapePaletteId", GetShapePaletteId);
	LuaPushFuntion(L, "GetShapeTexture", GetShapeTexture);
	LuaPushFuntion(L, "GetTextureOffset", GetTextureOffset);
	LuaPushFuntion(L, "SetShapePalette", SetShapePalette);
	LuaPushFuntion(L, "SetShapeTexture", SetShapeTexture);
	LuaPushFuntion(L, "SetTextureOffset", SetTextureOffset);
}
