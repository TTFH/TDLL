#ifndef _TEARDOWN_STRUCTS_H
#define _TEARDOWN_STRUCTS_H

#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "lua_utils.h"

enum GameState : int {
	Splash = 1,
	Menu = 2,
	UI = 3,
	Loading = 4,
	Play = 5,
	Edit = 6,
	Quit = 7
};

class td_string {
	union {
		char* HeapBuffer;
		char StackBuffer[32] = { 0 };
	};
public:
	td_string() {}
	td_string(const char* str) {
		size_t len = strlen(str);
		if (len < 32) {
			memcpy(StackBuffer, str, len);
		} else {
			HeapBuffer = new char[len + 1];
			memcpy(HeapBuffer, str, len);
			StackBuffer[31] = 1;
		}
	}
	const char* c_str() const {
		return StackBuffer[31] != '\0' ? HeapBuffer : &StackBuffer[0];
	}
};

template<typename T>
class td_vector {
	uint32_t size = 0;
	uint32_t capacity = 0;
	T* data = nullptr;
public:
	uint32_t getSize() const {
		return size;
	}
	void setSize(uint32_t size) {
		this->size = size;
	}
	T get(uint32_t index) const {
		return data[index];
	}
	T operator[](uint32_t index) const {
		return data[index];
	}
	T& operator[](uint32_t index) {
		return data[index];
	}
}; // 0x10

struct Vertex {
	float x, y;
};

struct RGBA {
	float r, g, b, a;
};

struct Vector {
	float x, y, z;
	Vector() : x(0), y(0), z(0) {}
	Vector(float x, float y, float z) : x(x), y(y), z(z) {}
	Vector operator*(float f) const {
		return Vector(x * f, y * f, z * f);
	}
};

struct Quat {
	float x, y, z, w;
	Quat() : x(0), y(0), z(0), w(1) {}
};

struct Transform {
	Vector pos;
	Quat rot;
};

struct Entity {
	void* unknown;
	uint8_t kind;
	uint8_t flags;
	uint8_t padding1[2];
	uint32_t handle;	// 0x0C
	Entity* parent;
	Entity* sibling;
	Entity* child;
	uint8_t padding2[8];
}; // 0x30

static_assert(sizeof(Entity) == 0x30, "Wrong size Entity");
static_assert(offsetof(Entity, handle) == 0x0C, "Wrong offset Entity->handle");

class Body : public Entity { };

struct Vox {
	uint32_t size[3];
	uint32_t volume;
	uint8_t* voxels;
	void* physics_buffer;
	float scale;			// 0x20
	uint8_t unknown1[48];
	uint32_t palette;		// 0x54
	uint8_t unknown2[4];
	int32_t voxel_count;
};

static_assert(offsetof(Vox, scale) == 0x20, "Wrong offset Vox->scale");
static_assert(offsetof(Vox, palette) == 0x54, "Wrong offset Vox->palette");

class Shape : public Entity {
public:
	uint8_t padding1[0x8C];
	float density;				// 0xBC
	uint8_t padding2[4];
	uint16_t texture_tile;		// 0xC4
	uint16_t blendtexture_tile = 0;
	float texture_weight;
	float blendtexture_weight = 1;
	Vector texture_offset;
	uint8_t padding3[4];
	Vox* vox;					// 0xE0
};

static_assert(offsetof(Shape, density) == 0xBC, "Wrong offset Shape->density");
static_assert(offsetof(Shape, texture_tile) == 0xC4, "Wrong offset Shape->texture_tile");
static_assert(offsetof(Shape, vox) == 0xE0, "Wrong offset Shape->vox");

enum LightType : int {
	Sphere = 1,
	Capsule,
	Cone,
	Area,
};

class Light : public Entity {
public:
	uint8_t padding1[4];
	int type;					// 0x34
	uint8_t padding2[0x74];
	float radius;				// 0xAC for sphere, capsule and cone
	float unshadowed;			// 0xB0
	float cos_half_angle_rad;	// 0xB4 for cone
	uint8_t padding3[0xC];
	float half_width;			// 0xC4 for area
	float half_height;			// 0xC8 for area
	float half_length;			// 0xCC for capsule
};

static_assert(offsetof(Light, type) == 0x34, "Wrong offset Light->type");
static_assert(offsetof(Light, radius) == 0xAC, "Wrong offset Light->radius");
static_assert(offsetof(Light, half_width) == 0xC4, "Wrong offset Light->half_width");

class Location : public Entity { };

class Water : public Entity {
public:
	Transform transform;		// 0x30
	float depth;
	td_vector<Vertex> vertices;	// 0x50
};

static_assert(offsetof(Water, vertices) == 0x50, "Wrong offset Water->vertices");

enum JointType : int {
	Ball = 1,
	Hinge,
	Prismatic,
	_Rope,
};

struct Rope {
	uint8_t padding[0xC];
	RGBA color;		// 0xC
};

class Joint : public Entity {
public:
	uint8_t padding1[0x20];
	JointType type;		// 0x50
	float size;
	bool collide;
	uint8_t padding3[0x17];
	Vector local_pos1;	// 0x70
	Vector local_pos2;
	Vector local_rot1;
	Vector local_rot2;
	uint8_t padding4[0x28];
	Rope* rope;			// 0xC8
	bool sound;			// 0xD0
	bool autodisable;	// 0xD1
};

static_assert(offsetof(Joint, type) == 0x50, "Wrong offset Joint->type");
static_assert(offsetof(Joint, local_pos1) == 0x70, "Wrong offset Joint->local_pos1");
static_assert(offsetof(Joint, rope) == 0xC8, "Wrong offset Joint->rope");
static_assert(offsetof(Joint, sound) == 0xD0, "Wrong offset Joint->sound");
static_assert(offsetof(Joint, autodisable) == 0xD1, "Wrong offset Joint->autodisable");

class Vehicle : public Entity { };

class Wheel : public Entity {
public:
	Vehicle* vehicle;	// 0x30
};

class Screen : public Entity { };

enum TriggerKind : int {
	TrSphere = 1,
	TrBox,
	TrPolygon,
};

class Trigger : public Entity {
public:
	uint8_t padding1[0x1C];
	TriggerKind type;			// 0x4C
	float sphere_size;			// 0x50
	Vector half_box_size;		// 0x54
	td_vector<Vertex> vertices;	// 0x60
	uint8_t padding2[0x80];
	float polygon_size; 		// 0xF0
};

static_assert(offsetof(Trigger, type) == 0x4C, "Wrong offset Trigger->type");
static_assert(offsetof(Trigger, polygon_size) == 0xF0, "Wrong offset Trigger->polygon_size");

struct ReturnInfo {
	lua_State* L;
	int ret_count;
	int max_ret;
};

struct LuaStateInfo {
	lua_State* state;
};

struct ScriptCoreInner {
	uint8_t padding[0x40];
	LuaStateInfo* state_info;		// 0x98
};

struct ScriptUiStatus {
	uint8_t padding1[0x348];
	int align_h;			// 0x348
	int align_v;			// 0x34C
};

struct ScriptCore {
	uint8_t padding1[8];
	float time;
	float dt;
	td_string path;					// 0x10
	td_string location;				// 0x30
	uint8_t padding2[8];
	ScriptCoreInner inner_core;
	uint8_t padding3[0x1E8];
	td_vector<uint32_t> entities;	// 0x288
	uint8_t padding4[0x10];
	ScriptUiStatus ui_status;		// 0x2A8
};

static_assert(offsetof(ScriptCore, path) == 0x10, "Wrong offset ScriptCore->path");
static_assert(offsetof(ScriptCore, entities) == 0x288, "Wrong offset ScriptCore->entities");
static_assert(offsetof(ScriptCore, ui_status) == 0x2A8, "Wrong offset ScriptCore->ui_status");

class Script : public Entity {
public:
	td_string name;			// 0x30
	td_string path;			// 0x50
	ScriptCore core;		// 0x70
};

static_assert(offsetof(Script, name) == 0x30, "Wrong offset Script->name");
static_assert(offsetof(Script, core) == 0x70, "Wrong offset Script->core");

struct Scene {
	uint8_t padding1[0x80];
	Light* flashlight;				// 0x80
	uint8_t padding2[0x58];
	Vector sv_size;					// 0xE0
	uint8_t padding3[0x4C];
	td_vector<Body*> bodies;		// 0x138
	td_vector<Shape*> shapes;		// 0x148
	td_vector<Light*> lights;		// 0x158
	td_vector<Location*> locations;	// 0x168
	td_vector<Water*> waters;		// 0x178
	td_vector<Joint*> joints;		// 0x188
	td_vector<Vehicle*> vehicles;	// 0x198
	td_vector<Wheel*> wheels;		// 0x1A8
	td_vector<Screen*> screens;		// 0x1B8
	td_vector<Trigger*> triggers;	// 0x1C8
	td_vector<Script*> scripts;		// 0x1D8
	uint8_t padding4[0x380];
	td_vector<Vertex> boundary;		// 0x568
};

static_assert(offsetof(Scene, flashlight) == 0x80, "Wrong offset Scene->flashlight");
static_assert(offsetof(Scene, sv_size) == 0xE0, "Wrong offset Scene->sv_size");
static_assert(offsetof(Scene, bodies) == 0x138, "Wrong offset Scene->bodies");
static_assert(offsetof(Scene, boundary) == 0x568, "Wrong offset Scene->boundary");

struct ExternalScript {
	uint8_t padding[0x38C];
	int privilege; // 0x38C
};

struct ModData {
	uint8_t padding[0x158];
	td_vector<ExternalScript*> external_scripts; // 0x158
};

struct Material {
	uint32_t kind;
	RGBA color;
	float reflectivity;
	float shininess;
	float metallic;
	float emissive;
	uint32_t replaceable;
}; // 0x28

static_assert(sizeof(Material) == 0x28, "Wrong size Material");

struct Palette {
	uint32_t padding1[3];
	Material materials[256];
	uint8_t black_tint[4 * 256];
	uint8_t yellow_tint[4 * 256];
	uint8_t other_tint[4 * 256];
	uint32_t padding2[13];
}; // 0x3440

static_assert(sizeof(Palette) == 0x3440, "Wrong size Palette");

struct Game {
	int screen_res_x;
	int screen_res_y;
	GameState state;
	uint8_t padding[0x44];
	Scene* scene;					// 0x50
	uint8_t padding2[0x70];
	td_vector<Palette>* palettes;	// 0xC8
	uint8_t padding3[8];
	ModData* mod_data;				// 0xD8
	uint8_t padding4[0xCC];
	float time_scale;				// 0x1AC
};

static_assert(offsetof(Game, scene) == 0x50, "Wrong offset game->scene");
static_assert(offsetof(Game, palettes) == 0xC8, "Wrong offset game->palette");
static_assert(offsetof(Game, mod_data) == 0xD8, "Wrong offset game->mod_data");
static_assert(offsetof(Game, time_scale) == 0x1AC, "Wrong offset game->time_scale");

#endif
