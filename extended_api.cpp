#include <math.h>
#include <time.h>
#include <stdio.h>
#include <chrono>
#include <vector>
#include <zlib.h>

#include "extended_api.h"

const double PI = 3.14159265358979323846;

// TODO: clocks per script
using std::chrono::duration_cast;
using std::chrono::nanoseconds;
using hrc = std::chrono::high_resolution_clock;
hrc::time_point clocks[16];
bool clock_init[16] = { false };

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

// TODO: Find by signature
namespace MEM_OFFSET {				// Addr		// Type
	uintptr_t Game					= 0xB45550; // Game*
	uintptr_t RenderDist			= 0x8BB568; // float
	uintptr_t LuaPushString			= 0x57DB50; // void fn(lua_State*, const char*)
	uintptr_t LuaCreateTable		= 0x57C8F0; // void fn(lua_State*, int, int)
	uintptr_t ProcessVideoFrameOGL	= 0x454720; // void fn(ScreenCapture*, int)
	uintptr_t RegisterGameFunctions	= 0x405F80; // void fn(ScriptCore*)
}

namespace Teardown {
	uintptr_t GetReferenceTo(uintptr_t offset) {
		return (uintptr_t)moduleBase + offset;
	}
	uintptr_t GetPointerTo(uintptr_t offset) {
		return FindDMAAddy((uintptr_t)moduleBase + offset, { 0x00 });
	}
	Game* GetGame() {
		return (Game*)GetPointerTo(MEM_OFFSET::Game);
	}
}

int GetDllVersion(lua_State* L) {
	td_lua_pushstring(L, "v1.6.0.704");
	return 1;
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
		double elapsed = duration_cast<nanoseconds>(now - clocks[index]).count();
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

int GetSystemDate(lua_State* L) {
	time_t t = time(NULL);
	struct tm* tm = localtime(&t);
	lua_pushinteger(L, tm->tm_year + 1900);
	lua_pushinteger(L, tm->tm_mon + 1);
	lua_pushinteger(L, tm->tm_mday);
	return 3;
}

int GetSystemTime(lua_State* L) {
	time_t t = time(NULL);
	struct tm* tm = localtime(&t);
	lua_pushinteger(L, tm->tm_hour);
	lua_pushinteger(L, tm->tm_min);
	lua_pushinteger(L, tm->tm_sec);
	return 3;
}

int GetBoundaryVertices(lua_State* L) {
	Game* game = Teardown::GetGame();
	unsigned int n = game->scene->boundary.getSize();
	td_lua_createtable(L, n, 0);
	for (unsigned int i = 0; i < n; i++) {
		Vec2 vertex = game->scene->boundary[i];
		LuaPushVec3(L, Vec3(vertex.x, 0, vertex.y));
		lua_rawseti(L, -2, i + 1);
	}
	return 1;
}

int RemoveBoundary(lua_State* L) {
	Game* game = Teardown::GetGame();
	game->scene->boundary.setSize(0);
	return 0;
}

int SetBoundaryVertex(lua_State* L) {
	unsigned int index = lua_tointeger(L, 1);
	Vec3 pos = LuaToVec3(L, 2);
	Game* game = Teardown::GetGame();
	if (index < game->scene->boundary.getSize())
		game->scene->boundary[index] = Vec2(pos.x, pos.z);
	return 0;
}

int GetWheelTransform(lua_State* L) {
	unsigned int handle = lua_tointeger(L, 1);
	Game* game = Teardown::GetGame();
	for (unsigned int i = 0; i < game->scene->wheels.getSize(); i++) {
		Wheel* wheel = game->scene->wheels[i];
		if (wheel->handle == handle) {
			LuaPushTransform(L, wheel->transform);
			return 1;
		}
	}
	LuaPushTransform(L, Transform());
	return 1;
}

int SetWheelTransform(lua_State* L) {
	unsigned int handle = lua_tointeger(L, 1);
	Transform transform = LuaToTransform(L, 2);
	Game* game = Teardown::GetGame();
	for (unsigned int i = 0; i < game->scene->wheels.getSize(); i++) {
		Wheel* wheel = game->scene->wheels[i];
		if (wheel->handle == handle) {
			wheel->transform = transform;
			return 0;
		}
	}
	return 0;
}

int SetWheelRadius(lua_State* L) {
	unsigned int handle = lua_tointeger(L, 1);
	float radius = lua_tonumber(L, 2);
	Game* game = Teardown::GetGame();
	for (unsigned int i = 0; i < game->scene->wheels.getSize(); i++) {
		Wheel* wheel = game->scene->wheels[i];
		if (wheel->handle == handle) {
			wheel->radius = radius;
			return 0;
		}
	}
	return 0;
}

int GetScriptPath(lua_State* L) {
	unsigned int handle = lua_tointeger(L, 1);
	Game* game = Teardown::GetGame();
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

int GetTimeScale(lua_State* L) {
	Game* game = Teardown::GetGame();
	lua_pushnumber(L, game->time_scale);
	return 1;
}

int GetShadowVolumeSize(lua_State* L) {
	Game* game = Teardown::GetGame();
	LuaPushVec3(L, game->scene->sv_size);
	return 1;
}

int GetWaterTransform(lua_State* L) {
	unsigned int handle = lua_tointeger(L, 1);
	Game* game = Teardown::GetGame();
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
	Game* game = Teardown::GetGame();
	for (unsigned int i = 0; i < game->scene->waters.getSize(); i++) {
		Water* water = game->scene->waters[i];
		if (water->handle == handle) {
			unsigned int n = water->vertices.getSize();
			td_lua_createtable(L, n, 0);
			for (unsigned int j = 0; j < n; j++) {
				Vec2 vertex = water->vertices[j];
				LuaPushVec3(L, Vec3(vertex.x, 0, vertex.y));
				lua_rawseti(L, -2, j + 1);
			}
			return 1;
		}
	}
	td_lua_createtable(L, 0, 0);
	return 1;
}

int SetWaterVertex(lua_State* L) {
	unsigned int handle = lua_tointeger(L, 1);
	unsigned int index = lua_tointeger(L, 2);
	Vec3 pos = LuaToVec3(L, 3);
	Game* game = Teardown::GetGame();
	for (unsigned int i = 0; i < game->scene->waters.getSize(); i++) {
		Water* water = game->scene->waters[i];
		if (water->handle == handle) {
			if (index < water->vertices.getSize())
				water->vertices[index] = Vec2(pos.x, pos.z);
			return 0;
		}
	}
	return 0;
}

int GetLightSize(lua_State* L) {
	unsigned int handle = lua_tointeger(L, 1);
	Game* game = Teardown::GetGame();
	for (unsigned int i = 0; i < game->scene->lights.getSize(); i++) {
		Light* light = game->scene->lights[i];
		if (light->handle == handle) {
			switch (light->type) {
			case LightSphere:
				lua_pushnumber(L, light->radius);
				lua_pushnumber(L, 0);
				break;
			case LightCapsule:
				lua_pushnumber(L, light->radius);
				lua_pushnumber(L, 2.0 * light->half_length);
				break;
			case LightCone: {
				double angle = 2.0 * acos(light->cos_half_angle_rad) * 180.0 / PI;
				lua_pushnumber(L, light->radius);
				lua_pushnumber(L, angle);
			}
				break;
			case LightArea:
				lua_pushnumber(L, 2.0 * light->half_width);
				lua_pushnumber(L, 2.0 * light->half_height);
				break;
			}
			return 2;
		}
	}
	lua_pushnumber(L, 0);
	lua_pushnumber(L, 0);
	return 2;
}

int GetTriggerVertices(lua_State* L) {
	unsigned int handle = lua_tointeger(L, 1);
	Game* game = Teardown::GetGame();
	for (unsigned int i = 0; i < game->scene->triggers.getSize(); i++) {
		Trigger* trigger = game->scene->triggers[i];
		if (trigger->handle == handle) {
			unsigned int n = trigger->vertices.getSize();
			td_lua_createtable(L, n, 0);
			for (unsigned int j = 0; j < n; j++) {
				Vec2 vertex = trigger->vertices[j];
				LuaPushVec3(L, Vec3(vertex.x, 0, vertex.y));
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
	Game* game = Teardown::GetGame();
	for (unsigned int i = 0; i < game->scene->joints.getSize(); i++) {
		Joint* joint = game->scene->joints[i];
		if (joint->handle == handle) {
			if (index == 1) {
				LuaPushVec3(L, joint->position1);
				LuaPushVec3(L, joint->axis1);
			} else {
				LuaPushVec3(L, joint->position2);
				LuaPushVec3(L, joint->axis2);
			}
			return 2;
		}
	}
	LuaPushVec3(L, Vec3());
	LuaPushVec3(L, Vec3(0, 1, 0));
	return 2;
}

int GetJointSize(lua_State* L) {
	unsigned int handle = lua_tointeger(L, 1);
	Game* game = Teardown::GetGame();
	for (unsigned int i = 0; i < game->scene->joints.getSize(); i++) {
		Joint* joint = game->scene->joints[i];
		if (joint->handle == handle) {
			lua_pushnumber(L, joint->size);
			return 1;
		}
	}
	lua_pushnumber(L, 0);
	return 1;
}

int GetJointParams(lua_State* L) {
	unsigned int handle = lua_tointeger(L, 1);
	Game* game = Teardown::GetGame();
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

static const char* MaterialTypeName[] = {
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

const int MATERIALS_COUNT = 15;

int GetPaletteMaterial(lua_State* L) {
	unsigned int palette_id = lua_tointeger(L, 1);
	unsigned int index = lua_tointeger(L, 2);

	if (index > 255)
		return 0;

	Game* game = Teardown::GetGame();
	unsigned int palette_count = game->palettes->getSize();
	if (palette_id >= palette_count)
		return 0;

	Material material = game->palettes->get(palette_id).materials[index];
	td_lua_pushstring(L, MaterialTypeName[material.type]);
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

int SetPaletteMaterialType(lua_State* L) {
	unsigned int palette_id = lua_tointeger(L, 1);
	unsigned int index = lua_tointeger(L, 2);
	const char* type = lua_tostring(L, 3);

	if (index > 255)
		return 0;

	Game* game = Teardown::GetGame();
	unsigned int palette_count = game->palettes->getSize();
	if (palette_id >= palette_count)
		return 0;

	Material& material = game->palettes->get(palette_id).materials[index];
	for (int i = 0; i < MATERIALS_COUNT; i++) {
		if (strcmp(type, MaterialTypeName[i]) == 0) {
			material.type = i;
			break;
		}
	}
	return 0;
}

int GetShapeDensity(lua_State* L) {
	unsigned int handle = lua_tointeger(L, 1);
	Game* game = Teardown::GetGame();
	for (unsigned int i = 0; i < game->scene->shapes.getSize(); i++) {
		Shape* shape = game->scene->shapes[i];
		if (shape->handle == handle) {
			lua_pushnumber(L, shape->density);
			return 1;
		}
	}
	lua_pushnumber(L, 0);
	return 1;
}

int GetShapePaletteId(lua_State* L) {
	unsigned int handle = lua_tointeger(L, 1);
	Game* game = Teardown::GetGame();
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
	Game* game = Teardown::GetGame();
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
	Game* game = Teardown::GetGame();
	for (unsigned int i = 0; i < game->scene->shapes.getSize(); i++) {
		Shape* shape = game->scene->shapes[i];
		if (shape->handle == handle) {
			LuaPushVec3(L, shape->texture_offset);
			return 1;
		}
	}
	LuaPushVec3(L, Vec3());
	return 1;
}

int SetShapeScale(lua_State* L) {
	unsigned int handle = lua_tointeger(L, 1);
	float scale = lua_tonumber(L, 2);

	Game* game = Teardown::GetGame();
	for (unsigned int i = 0; i < game->scene->shapes.getSize(); i++) {
		Shape* shape = game->scene->shapes[i];
		if (shape->handle == handle) {
			shape->vox->scale = scale;
			return 0;
		}
	}
	return 0;
}

int SetShapePalette(lua_State* L) {
	unsigned int handle = lua_tointeger(L, 1);
	unsigned int palette = lua_tointeger(L, 2);

	Game* game = Teardown::GetGame();
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

	Game* game = Teardown::GetGame();
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
	Vec3 offset = LuaToVec3(L, 2);

	Game* game = Teardown::GetGame();
	for (unsigned int i = 0; i < game->scene->shapes.getSize(); i++) {
		Shape* shape = game->scene->shapes[i];
		if (shape->handle == handle) {
			shape->texture_offset = offset;
			return 0;
		}
	}
	return 0;
}

int SaveToFile(lua_State* L) {
	const char* filename = lua_tostring(L, 1);
	size_t str_len;
	const char* str = lua_tolstring(L, 2, &str_len);

	FILE* file = fopen(filename, "wb");
	if (file == nullptr) {
		printf("Error opening file %s for writing.\n", filename);
		return 0;
	}

	fwrite(str, sizeof(char), str_len, file);
	fclose(file);
	return 0;
}

int ZlibSaveCompressed(lua_State* L) {
	const char* filename = lua_tostring(L, 1);
	size_t src_length;
	const char* str = lua_tolstring(L, 2, &src_length);

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
	// TODO: use td_lua_pushlstring(lua_State*, const char*, size_t)
	dest.resize(dest_length + 1);
	dest[dest_length] = '\0';
	td_lua_pushstring(L, (const char*)dest.data());
	return 1;
}

int GetFireInfo(lua_State* L) {
	unsigned int index = lua_tointeger(L, 1);
	if (index > 0) index--; // To 0-based index
	Game* game = Teardown::GetGame();
	if (index < game->scene->firesystem->fires.getSize()) {
		Fire fire = game->scene->firesystem->fires[index];
		lua_pushinteger(L, fire.shape->handle);
		LuaPushVec3(L, fire.position);
		return 2;
	}
	lua_pushinteger(L, 0);
	LuaPushVec3(L, Vec3());
	return 2;
}

int GetPaletteTintArray(lua_State* L) {
	unsigned int palette_id = lua_tointeger(L, 1);
	const char* color = lua_tostring(L, 2);
	unsigned int strength = lua_tointeger(L, 3);

	if (strength < 1) strength = 1;
	if (strength > 4) strength = 4;

	Game* game = Teardown::GetGame();
	unsigned int palette_count = game->palettes->getSize();
	if (palette_id >= palette_count)
		return 0;

	Palette& palette = game->palettes->get(palette_id);

	std::vector<int> tint_array;
	tint_array.reserve(255);
	for (unsigned int i = 1; i < 256; i++) { // Skip first index
		uint8_t index = 0;
		if (strcmp(color, "black") == 0)
			index = palette.black_tint[256 * strength + i];
		else if (strcmp(color, "yellow") == 0)
			index = palette.yellow_tint[256 * strength + i];
		else
			index = palette.rgba_tint[256 * strength + i];
		tint_array.push_back(index);
	}

	LuaPushList(L, tint_array);
	return 1;
}

int SetVehicleMaxSteerAngle(lua_State* L) {
	unsigned int handle = lua_tointeger(L, 1);
	float angle = lua_tonumber(L, 2);
	if (angle <= 0) angle = 30.0;
	Game* game = Teardown::GetGame();
	for (unsigned int i = 0; i < game->scene->vehicles.getSize(); i++) {
		Vehicle* vehicle = game->scene->vehicles[i];
		if (vehicle->handle == handle) {
			vehicle->max_steer_angle = angle * PI / 180.0; // To radians
			return 0;
		}
	}
	return 0;
}

int SetJointStrength(lua_State* L) {
	unsigned int handle = lua_tointeger(L, 1);
	float strength = lua_tonumber(L, 2);
	float size = lua_tonumber(L, 3);
	if (strength <= 0) strength = 3000.0;
	if (size <= 0) size = 0.8;
	Game* game = Teardown::GetGame();
	for (unsigned int i = 0; i < game->scene->joints.getSize(); i++) {
		Joint* joint = game->scene->joints[i];
		if (joint->handle == handle) {
			joint->connection_strength = strength;
			joint->disconnect_dist = size;
			return 0;
		}
	}
	return 0;
}

int GetHeatCount(lua_State* L) {
	Game* game = Teardown::GetGame();
	lua_pushinteger(L, game->player->heats.getSize());
	return 1;
}

int GetHeatInfo(lua_State* L) {
	unsigned int index = lua_tointeger(L, 1);
	if (index > 0) index--; // To 0-based index
	Game* game = Teardown::GetGame();
	if (index < game->player->heats.getSize()) {
		Heat& heat = game->player->heats[index];
		lua_pushinteger(L, heat.shape->handle);
		LuaPushVec3(L, Vec3(heat.x, heat.y, heat.z));
		lua_pushnumber(L, heat.amount);
		return 3;
	}
	lua_pushinteger(L, 0);
	LuaPushVec3(L, Vec3());
	lua_pushnumber(L, 0);
	return 3;
}

int SetSunLength(lua_State* L) {
	float length = lua_tonumber(L, 1);
	Game* game = Teardown::GetGame();
	game->scene->environment->sun_length = length;
	return 0;
}

int AllowInternalFunctions(lua_State* L) {
	unsigned int handle = lua_tointeger(L, 1);
	Game* game = Teardown::GetGame();
	for (unsigned int i = 0; i < game->scene->scripts.getSize(); i++) {
		Script* script = game->scene->scripts[i];
		if (script->handle == handle) {
			if (script->core.check_internal->privilege > 1)
				script->core.check_internal->privilege = 1;
			return 0;
		}
	}
	return 0;
}

int IsRagdoll(lua_State* L) {
	unsigned int handle = lua_tointeger(L, 1);
	Game* game = Teardown::GetGame();
	for (unsigned int i = 0; i < game->scene->animators.getSize(); i++) {
		Animator* animator = game->scene->animators[i];
		if (animator->handle == handle) {
			lua_pushboolean(L, animator->anim_core->is_ragdoll);
			return 1;
		}
	}
	lua_pushboolean(L, false);
	return 1;
}

void RegisterLuaCFunctions(lua_State* L) {
	LuaPushFunction(L, "GetDllVersion", GetDllVersion);
	LuaPushFunction(L, "Tick", Tick);
	LuaPushFunction(L, "Tock", Tock);
	LuaPushFunction(L, "GetSystemTime", GetSystemTime);
	LuaPushFunction(L, "GetSystemDate", GetSystemDate);
	LuaPushFunction(L, "HttpRequest", HttpRequest);
	LuaPushFunction(L, "SaveToFile", SaveToFile);
	LuaPushFunction(L, "ZlibSaveCompressed", ZlibSaveCompressed);
	LuaPushFunction(L, "ZlibLoadCompressed", ZlibLoadCompressed);

	LuaPushFunction(L, "RemoveBoundary", RemoveBoundary);
	LuaPushFunction(L, "GetBoundaryVertices", GetBoundaryVertices);
	LuaPushFunction(L, "SetBoundaryVertex", SetBoundaryVertex);

	LuaPushFunction(L, "IsRagdoll", IsRagdoll);
	LuaPushFunction(L, "GetHeatCount", GetHeatCount);
	LuaPushFunction(L, "GetHeatInfo", GetHeatInfo);
	LuaPushFunction(L, "SetSunLength", SetSunLength);
	LuaPushFunction(L, "GetFireInfo", GetFireInfo);
	LuaPushFunction(L, "SetJointStrength", SetJointStrength);
	LuaPushFunction(L, "GetPaletteTintArray", GetPaletteTintArray);
	LuaPushFunction(L, "AllowInternalFunctions", AllowInternalFunctions);
	LuaPushFunction(L, "SetVehicleMaxSteerAngle", SetVehicleMaxSteerAngle);

	LuaPushFunction(L, "GetTimeScale", GetTimeScale);
	LuaPushFunction(L, "GetShadowVolumeSize", GetShadowVolumeSize);

	LuaPushFunction(L, "GetWaterTransform", GetWaterTransform);
	LuaPushFunction(L, "GetWaterVertices", GetWaterVertices);
	LuaPushFunction(L, "SetWaterVertex", SetWaterVertex);

	LuaPushFunction(L, "GetScriptPath", GetScriptPath);

	LuaPushFunction(L, "GetWheelTransform", GetWheelTransform);
	LuaPushFunction(L, "SetWheelTransform", SetWheelTransform);
	LuaPushFunction(L, "SetWheelRadius", SetWheelRadius);

	LuaPushFunction(L, "GetLightSize", GetLightSize);
	LuaPushFunction(L, "GetTriggerVertices", GetTriggerVertices);

	LuaPushFunction(L, "GetJointLocalPosAndAxis", GetJointLocalPosAndAxis);
	LuaPushFunction(L, "GetJointSize", GetJointSize);
	LuaPushFunction(L, "GetJointParams", GetJointParams);

	LuaPushFunction(L, "GetPaletteMaterial", GetPaletteMaterial);
	LuaPushFunction(L, "SetPaletteMaterialType", SetPaletteMaterialType);

	LuaPushFunction(L, "GetShapeDensity", GetShapeDensity);
	LuaPushFunction(L, "GetShapePaletteId", GetShapePaletteId);
	LuaPushFunction(L, "GetShapeTexture", GetShapeTexture);
	LuaPushFunction(L, "GetTextureOffset", GetTextureOffset);
	LuaPushFunction(L, "SetShapeScale", SetShapeScale);
	LuaPushFunction(L, "SetShapePalette", SetShapePalette);
	LuaPushFunction(L, "SetShapeTexture", SetShapeTexture);
	LuaPushFunction(L, "SetTextureOffset", SetTextureOffset);
}
