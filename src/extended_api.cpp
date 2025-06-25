#include <map>
#include <chrono>
#include <math.h>
#include <stdio.h>
#include <string>
#include <time.h>
#include <vector>
#include <zlib.h>

#include "memory.h"
#include "recorder.h"
#include "extended_api.h"

#include "../lib/stb_image.h"

#define PI 3.14159265358979323846

// TODO: clocks per script
using std::chrono::duration_cast;
using std::chrono::nanoseconds;
using hrc = std::chrono::high_resolution_clock;
hrc::time_point clocks[16];
bool clock_init[16] = { false };

// TODO: Find by signature
namespace MEM_OFFSET {				// Addr		// Type
	uintptr_t Game					= 0xB411D0; // Game*
	uintptr_t FarPlane				= 0x8B74F0; // float
	uintptr_t LuaPushString			= 0x57D430; // void fn(lua_State*, const char*)
	uintptr_t LuaCreateTable		= 0x57C1D0; // void fn(lua_State*, int, int)
	uintptr_t ProcessVideoFrameOGL	= 0x450930; // void fn(ScreenCapture*, int)
	uintptr_t RegisterGameFunctions	= 0x41A6F0; // void fn(ScriptCoreInner*)
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

template<typename T>
T* GetEntity(int handle, uint8_t type) {
	Game* game = Teardown::GetGame();
	if (handle > 0 && handle < game->scene->entities.getSize()) {
		Entity* entity = game->scene->entities[handle];
		if (entity->type == type)
			return (T*)entity;
	}
	return nullptr;
}

int GetDllVersion(lua_State* L) {
	td_lua_pushstring(L, "v1.7.0.0625");
	return 1;
}

int Tick(lua_State* L) {
	unsigned int index = lua_tointeger(L, 1);
	clock_init[index] = true;
	clocks[index] = hrc::now();
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

#ifdef _USE_CURL
int HttpRequest(lua_State* L) {
	const char* method = lua_tostring(L, 1);
	const char* endpoint = lua_tostring(L, 2);
	std::map<std::string, std::string> headers = LuaToMap(L, 3);
	const char* body = lua_tostring(L, 4);
	const char* cookies = lua_tostring(L, 5);

	std::string response;
	int status = HttpRequest(method, endpoint, headers, body, cookies, response);
	lua_pushinteger(L, status);
	td_lua_pushstring(L, response.c_str());
	return 2;
}

int HttpAsyncRequest(lua_State* L) {
	const char* method = lua_tostring(L, 1);
	const char* endpoint = lua_tostring(L, 2);
	std::map<std::string, std::string> headers = LuaToMap(L, 3);
	const char* body = lua_tostring(L, 4);

	int handle = AddRequest(method, endpoint, headers, body);
	lua_pushinteger(L, handle);
	return 1;
}

int FetchHttpResponses(lua_State* L) {
	std::vector<HTTP_Response> responses = FetchResponses();
	td_lua_createtable(L, responses.size(), 0);
	for (unsigned int i = 0; i < responses.size(); i++) {
		HTTP_Response response = responses[i];
		td_lua_createtable(L, 0, 4);
		lua_pushinteger(L, response.id);
		lua_setfield(L, -2, "id");
		td_lua_pushstring(L, response.url.c_str());
		lua_setfield(L, -2, "url");
		lua_pushinteger(L, response.status);
		lua_setfield(L, -2, "status");
		td_lua_pushstring(L, response.body.c_str());
		lua_setfield(L, -2, "body");
		lua_rawseti(L, -2, i + 1);
	}
	return 1;
}
#endif

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
	game->scene->boundary.clear();
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
	Wheel* wheel = GetEntity<Wheel>(handle, EntityType::Wheel);
	if (wheel != nullptr)
		LuaPushTransform(L, wheel->transform);
	else
		LuaPushTransform(L, Transform());
	return 1;
}

int SetWheelTransform(lua_State* L) {
	unsigned int handle = lua_tointeger(L, 1);
	Transform transform = LuaToTransform(L, 2);
	Wheel* wheel = GetEntity<Wheel>(handle, EntityType::Wheel);
	if (wheel != nullptr)
		wheel->transform = transform;
	return 0;
}

int SetWheelRadius(lua_State* L) {
	unsigned int handle = lua_tointeger(L, 1);
	float radius = lua_tonumber(L, 2);
	Wheel* wheel = GetEntity<Wheel>(handle, EntityType::Wheel);
	if (wheel != nullptr)
		wheel->radius = radius;
	return 0;
}

int GetScriptPath(lua_State* L) {
	unsigned int handle = lua_tointeger(L, 1);
	Script* script = GetEntity<Script>(handle, EntityType::Script);
	if (script != nullptr)
		td_lua_pushstring(L, script->path.c_str());
	else
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
	Water* water = GetEntity<Water>(handle, EntityType::Water);
	if (water != nullptr)
		LuaPushTransform(L, water->transform);
	else
		LuaPushTransform(L, Transform());
	return 1;
}

int GetWaterVertices(lua_State* L) {
	unsigned int handle = lua_tointeger(L, 1);
	Water* water = GetEntity<Water>(handle, EntityType::Water);
	if (water != nullptr) {
		unsigned int n = water->vertices.getSize();
		td_lua_createtable(L, n, 0);
		for (unsigned int j = 0; j < n; j++) {
			Vec2 vertex = water->vertices[j];
			LuaPushVec3(L, Vec3(vertex.x, 0, vertex.y));
			lua_rawseti(L, -2, j + 1);
		}
	} else
		LuaPushEmptyTable(L);
	return 1;
}

int SetWaterVertex(lua_State* L) {
	unsigned int handle = lua_tointeger(L, 1);
	unsigned int index = lua_tointeger(L, 2);
	if (index > 0) index--; // To 0-based index
	Vec3 pos = LuaToVec3(L, 3);
	Water* water = GetEntity<Water>(handle, EntityType::Water);
	if (water != nullptr && index < water->vertices.getSize())
		water->vertices[index] = Vec2(pos.x, pos.z);
	return 0;
}

int GetLightSize(lua_State* L) {
	unsigned int handle = lua_tointeger(L, 1);
	Light* light = GetEntity<Light>(handle, EntityType::Light);
	if (light != nullptr) {
		switch (light->type) {
		case LightType::Sphere:
			lua_pushnumber(L, light->radius);
			lua_pushnumber(L, 0);
			break;
		case LightType::Capsule:
			lua_pushnumber(L, light->radius);
			lua_pushnumber(L, 2.0 * light->half_length);
			break;
		case LightType::Cone: {
			double angle = 2.0 * acos(light->cos_half_angle_rad) * 180.0 / PI;
			lua_pushnumber(L, light->radius);
			lua_pushnumber(L, angle);
		}
			break;
		case LightType::Area:
			lua_pushnumber(L, 2.0 * light->half_size.x);
			lua_pushnumber(L, 2.0 * light->half_size.y);
			break;
		}
	} else {
		lua_pushnumber(L, 0);
		lua_pushnumber(L, 0);
	}
	return 2;
}

int GetTriggerVertices(lua_State* L) {
	unsigned int handle = lua_tointeger(L, 1);
	Trigger* trigger = GetEntity<Trigger>(handle, EntityType::Trigger);
	if (trigger != nullptr) {
		unsigned int n = trigger->vertices.getSize();
		td_lua_createtable(L, n, 0);
		for (unsigned int j = 0; j < n; j++) {
			Vec2 vertex = trigger->vertices[j];
			LuaPushVec3(L, Vec3(vertex.x, 0, vertex.y));
			lua_rawseti(L, -2, j + 1);
		}
	} else
		LuaPushEmptyTable(L);
	return 1;
}

int GetJointLocalPosAndAxis(lua_State* L) {
	unsigned int handle = lua_tointeger(L, 1);
	unsigned int index = lua_tointeger(L, 2);
	Joint* joint = GetEntity<Joint>(handle, EntityType::Joint);
	if (joint != nullptr) {
		if (index == 1) {
			LuaPushVec3(L, joint->position1);
			LuaPushVec3(L, joint->axis1);
		} else {
			LuaPushVec3(L, joint->position2);
			LuaPushVec3(L, joint->axis2);
		}
	} else {
		LuaPushVec3(L, Vec3());
		LuaPushVec3(L, Vec3(0, 1, 0));
	}
	return 2;
}

int GetJointSize(lua_State* L) {
	unsigned int handle = lua_tointeger(L, 1);
	Joint* joint = GetEntity<Joint>(handle, EntityType::Joint);
	if (joint != nullptr)
		lua_pushnumber(L, joint->size);
	else
		lua_pushnumber(L, 0);
	return 1;
}

int GetJointParams(lua_State* L) {
	unsigned int handle = lua_tointeger(L, 1);
	Joint* joint = GetEntity<Joint>(handle, EntityType::Joint);
	if (joint != nullptr) {
		lua_pushboolean(L, joint->collide);
		lua_pushboolean(L, joint->sound);
		lua_pushboolean(L, joint->autodisable);
	} else {
		lua_pushboolean(L, false);
		lua_pushboolean(L, false);
		lua_pushboolean(L, false);
	}
	return 3;
}

const char* MaterialTypeName[] = {
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

const int MATERIALS_COUNT = sizeof(MaterialTypeName) / sizeof(MaterialTypeName[0]);

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
	Shape* shape = GetEntity<Shape>(handle, EntityType::Shape);
	if (shape != nullptr)
		lua_pushnumber(L, shape->density);
	else
		lua_pushnumber(L, 0);
	return 1;
}

int GetShapePaletteId(lua_State* L) {
	unsigned int handle = lua_tointeger(L, 1);
	Shape* shape = GetEntity<Shape>(handle, EntityType::Shape);
	if (shape != nullptr)
		lua_pushinteger(L, shape->vox->palette);
	else
		lua_pushinteger(L, 0);
	return 1;
}

int GetShapeVoxelMatrix(lua_State* L) {
	unsigned int handle = lua_tointeger(L, 1);
	Shape* shape = GetEntity<Shape>(handle, EntityType::Shape);
	if (shape != nullptr) {
		int sizex = shape->vox->size[0];
		int sizey = shape->vox->size[1];
		int sizez = shape->vox->size[2];
		uint8_t* voxels = shape->vox->voxels;

		td_lua_createtable(L, sizex, 0);
		for (int x = 0; x < sizex; x++) {
			td_lua_createtable(L, sizey, 0);
			for (int y = 0; y < sizey; y++) {
				td_lua_createtable(L, sizez, 0);
				for (int z = 0; z < sizez; z++) {
					lua_pushinteger(L, voxels[x + sizex * (y + sizey * z)]);
					lua_rawseti(L, -2, z + 1);
				}
				lua_rawseti(L, -2, y + 1);
			}
			lua_rawseti(L, -2, x + 1);
		}
	} else
		LuaPushEmptyTable(L);
	return 1;
}

int GetShapeTexture(lua_State* L) {
	unsigned int handle = lua_tointeger(L, 1);
	Shape* shape = GetEntity<Shape>(handle, EntityType::Shape);
	if (shape != nullptr) {
		lua_pushinteger(L, shape->texture_tile);
		lua_pushnumber(L, shape->texture_weight);
		lua_pushinteger(L, shape->blendtexture_tile);
		lua_pushnumber(L, shape->blendtexture_weight);
	} else {
		lua_pushinteger(L, 0);
		lua_pushnumber(L, 1);
		lua_pushinteger(L, 0);
		lua_pushnumber(L, 1);
	}
	return 4;
}

int GetTextureOffset(lua_State* L) {
	unsigned int handle = lua_tointeger(L, 1);
	Shape* shape = GetEntity<Shape>(handle, EntityType::Shape);
	if (shape != nullptr)
		LuaPushVec3(L, shape->texture_offset);
	else
		LuaPushVec3(L, Vec3());
	return 1;
}

int SetShapeScale(lua_State* L) {
	unsigned int handle = lua_tointeger(L, 1);
	float scale = lua_tonumber(L, 2);
	Shape* shape = GetEntity<Shape>(handle, EntityType::Shape);
	if (shape != nullptr)
		shape->vox->scale = scale;
	return 0;
}

int SetShapePalette(lua_State* L) {
	unsigned int handle = lua_tointeger(L, 1);
	unsigned int palette = lua_tointeger(L, 2);
	Shape* shape = GetEntity<Shape>(handle, EntityType::Shape);
	if (shape != nullptr)
		shape->vox->palette = palette;
	return 0;
}

int SetShapeTexture(lua_State* L) {
	unsigned int handle = lua_tointeger(L, 1);
	unsigned int texture_tile = lua_tointeger(L, 2);
	float texture_weight = lua_tonumber(L, 3);
	unsigned int blendtexture_tile = lua_tointeger(L, 4);
	float blendtexture_weight = lua_tonumber(L, 5);
	Shape* shape = GetEntity<Shape>(handle, EntityType::Shape);
	if (shape != nullptr) {
		shape->texture_tile = texture_tile;
		shape->blendtexture_tile = blendtexture_tile;
		shape->texture_weight = texture_weight;
		shape->blendtexture_weight = blendtexture_weight;
	}
	return 0;
}

int SetTextureOffset(lua_State* L) {
	unsigned int handle = lua_tointeger(L, 1);
	Vec3 offset = LuaToVec3(L, 2);
	Shape* shape = GetEntity<Shape>(handle, EntityType::Shape);
	if (shape != nullptr)
		shape->texture_offset = offset;
	return 0;
}

int SaveToFile(lua_State* L) {
	const char* filename = lua_tostring(L, 1);
	size_t str_len;
	const char* str = lua_tolstring(L, 2, &str_len);

	FILE* file = fopen(filename, "wb");
	if (file == nullptr) {
		printf("[ERROR] Could not open file %s for writing.\n", filename);
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
		printf("[ERROR] Memory allocation failed.\n");
		return 0;
	}

	int result = compress(dest, &dest_length, (const Bytef*)str, src_length);
	if (result != Z_OK) {
		printf("[ERROR] Compression failed with code %d\n", result);
		delete[] dest;
		return 0;
	}

	FILE* file = fopen(filename, "wb");
	if (file == nullptr) {
		printf("[ERROR] Could not open file %s for writing.\n", filename);
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
		printf("[ERROR] Could not open %s for reading.\n", filename);
		return 0;
	}

	fseek(file, 0, SEEK_END);
	long file_size = ftell(file);
	fseek(file, 0, SEEK_SET);

	Bytef* compressed_data = new Bytef[file_size];
	if (compressed_data == nullptr) {
		printf("[ERROR] Memory allocation failed.\n");
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
			printf("[ERROR] Decompression failed with code %d\n", result);
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
	Vehicle* vehicle = GetEntity<Vehicle>(handle, EntityType::Vehicle);
	if (vehicle != nullptr)
		vehicle->max_steer_angle = angle * PI / 180.0; // To radians
	return 0;
}

int SetJointStrength(lua_State* L) {
	unsigned int handle = lua_tointeger(L, 1);
	float strength = lua_tonumber(L, 2);
	float size = lua_tonumber(L, 3);
	if (strength <= 0) strength = 3000.0;
	if (size <= 0) size = 0.8;
	Joint* joint = GetEntity<Joint>(handle, EntityType::Joint);
	if (joint != nullptr) {
		joint->connection_strength = strength;
		joint->disconnect_dist = size;
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
	Script* script = GetEntity<Script>(handle, EntityType::Script);
	if (script != nullptr && script->core.check_internal != nullptr && script->core.check_internal->privilege > 1)
		script->core.check_internal->privilege = 1;
	return 0;
}

int IsRagdoll(lua_State* L) {
	unsigned int handle = lua_tointeger(L, 1);
	Animator* animator = GetEntity<Animator>(handle, EntityType::Animator);
	if (animator != nullptr)
		lua_pushboolean(L, animator->core->is_ragdoll);
	else
		lua_pushboolean(L, false);
	return 1;
}

int HasCollision(lua_State* L) {
	unsigned int handle = lua_tointeger(L, 1);
	Shape* shape = GetEntity<Shape>(handle, EntityType::Shape);
	if (shape != nullptr)
		lua_pushboolean(L, shape->shape_flags & ShapeFlags::Physical);
	else
		lua_pushboolean(L, false);
	return 1;
}

int SetCollision(lua_State* L) {
	unsigned int handle = lua_tointeger(L, 1);
	bool enable = lua_toboolean(L, 2);
	Shape* shape = GetEntity<Shape>(handle, EntityType::Shape);
	if (shape != nullptr) {
		if (enable)
			shape->shape_flags |= ShapeFlags::Physical;
		else
			shape->shape_flags &= ~ShapeFlags::Physical;
	}
	return 0;
}

int SendDatagram(lua_State* L) {
	const char* message = lua_tostring(L, 1);
	broadcast.Send(message);
	return 0;
}

int FetchDatagrams(lua_State* L) {
	msg_mutex.lock();
	td_lua_createtable(L, messages.size(), 0);
	for (std::vector<std::string>::iterator it = messages.begin(); it != messages.end(); it++) {
		td_lua_pushstring(L, it->c_str());
		lua_rawseti(L, -2, std::distance(messages.begin(), it) + 1);
	}
	messages.clear();
	msg_mutex.unlock();
	return 1;
}

int LoadImagePixels(lua_State* L) {
	const char* filename = lua_tostring(L, 1);
	int width, height;
	uint8_t* pixels = stbi_load(filename, &width, &height, NULL, STBI_rgb);
	if (pixels != nullptr) {
		lua_pushinteger(L, width);
		lua_pushinteger(L, height);
		td_lua_createtable(L, width * height, 0);
		for (int i = 0; i < width * height; i++) {
			uint8_t red = pixels[i * 3];
			uint8_t green = pixels[i * 3 + 1];
			uint8_t blue = pixels[i * 3 + 2];
			lua_pushinteger(L, i + 1);
			td_lua_createtable(L, 0, 3);
			lua_pushinteger(L, red);
			lua_setfield(L, -2, "r");
			lua_pushinteger(L, green);
			lua_setfield(L, -2, "g");
			lua_pushinteger(L, blue);
			lua_setfield(L, -2, "b");
			lua_rawset(L, -3);
		}
		stbi_image_free(pixels);
		return 3;
	}
	lua_pushinteger(L, 0);
	lua_pushinteger(L, 0);
	td_lua_createtable(L, 0, 0);
	return 3;
}

int SaveImageToFile(lua_State* L) {
	const char* filename = lua_tostring(L, 1);
	int width = lua_tointeger(L, 2);
	int height = lua_tointeger(L, 3);
	uint8_t* pixels = new uint8_t[width * height * 3];

	lua_pushnil(L);
	while (lua_next(L, 4) != 0) {
		int index = lua_tointeger(L, -2);
		if (index > 0 && index <= width * height) {
			lua_getfield(L, -1, "r");
			uint8_t red = lua_tointeger(L, -1);
			lua_pop(L, 1);
			lua_getfield(L, -1, "g");
			uint8_t green = lua_tointeger(L, -1);
			lua_pop(L, 1);
			lua_getfield(L, -1, "b");
			uint8_t blue = lua_tointeger(L, -1);
			lua_pop(L, 1);
			pixels[(index - 1) * 3] = red;
			pixels[(index - 1) * 3 + 1] = green;
			pixels[(index - 1) * 3 + 2] = blue;
		}
		lua_pop(L, 1);
	}

	SaveImagePNG(filename, width, height, pixels);
	delete[] pixels;
	return 0;
}

void RegisterLuaCFunctions(lua_State* L) {
	LuaPushFunction(L, "GetDllVersion", GetDllVersion);

	LuaPushFunction(L, "Tick", Tick);
	LuaPushFunction(L, "Tock", Tock);
	LuaPushFunction(L, "GetSystemTime", GetSystemTime);
	LuaPushFunction(L, "GetSystemDate", GetSystemDate);
#ifdef _USE_CURL
	LuaPushFunction(L, "HttpRequest", HttpRequest);
	LuaPushFunction(L, "HttpAsyncRequest", HttpAsyncRequest);
	LuaPushFunction(L, "FetchHttpResponses", FetchHttpResponses);
#endif
	LuaPushFunction(L, "SendDatagram", SendDatagram);
	LuaPushFunction(L, "FetchDatagrams", FetchDatagrams);

	LuaPushFunction(L, "SaveToFile", SaveToFile);
	LuaPushFunction(L, "LoadImagePixels", LoadImagePixels);
	LuaPushFunction(L, "SaveImageToFile", SaveImageToFile);
	LuaPushFunction(L, "ZlibSaveCompressed", ZlibSaveCompressed);
	LuaPushFunction(L, "ZlibLoadCompressed", ZlibLoadCompressed);

	LuaPushFunction(L, "RemoveBoundary", RemoveBoundary);
	LuaPushFunction(L, "GetBoundaryVertices", GetBoundaryVertices);
	LuaPushFunction(L, "SetBoundaryVertex", SetBoundaryVertex);

	LuaPushFunction(L, "HasCollision", HasCollision);
	LuaPushFunction(L, "SetCollision", SetCollision);

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
	LuaPushFunction(L, "GetShapeVoxelMatrix", GetShapeVoxelMatrix);
}
