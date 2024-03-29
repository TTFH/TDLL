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
	T operator[](uint32_t index) const {
		return data[index];
	}
	T& operator[](uint32_t index) {
		return data[index];
	}
};

struct Vertex {
	float x, y;
};

struct Color {
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
	float scale;
	uint8_t unknown1[48];
	uint32_t palette;		// 0x54
	uint8_t unknown2[4];
	int32_t voxel_count;
};

static_assert(offsetof(Vox, palette) == 0x54, "Wrong offset Vox->palette");

class Shape : public Entity {
public:
	uint8_t padding1[0x8C];
	uint16_t texture_tile;		// 0xBC
	uint16_t blendtexture_tile = 0;
	float texture_weight;
	float blendtexture_weight = 1;
	Vector texture_offset;
	uint8_t padding2[0x4];
	Vox* vox;					// 0xD8
};

static_assert(offsetof(Shape, texture_tile) == 0xBC, "Wrong offset Shape->texture_tile");
static_assert(offsetof(Shape, vox) == 0xD8, "Wrong offset Shape->vox");

class Light : public Entity { };
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
	Color color;		// 0xC
};

class Joint : public Entity {
public:
	uint8_t padding1[0x20];
	JointType type;		// 0x50
	uint8_t padding2[4];
	bool collide;		// 0x58
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
static_assert(offsetof(Joint, collide) == 0x58, "Wrong offset Joint->collide");
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

struct LuaStateInfo {
	lua_State* state;
};

struct ScriptCore {
	uint8_t padding1[8];
	float time;
	float dt;
	td_string path;					// 0x10
	td_string location;				// 0x30
	uint8_t padding2[0x18 + 0x40];
	LuaStateInfo* state_info;		// 0xA8
	uint8_t padding3[0x1E8];
	td_vector<uint32_t> entities;	// 0x298
};

static_assert(offsetof(ScriptCore, path) == 0x10, "Wrong offset ScriptCore->path");
static_assert(offsetof(ScriptCore, state_info) == 0xA8, "Wrong offset ScriptCore->state_info");
static_assert(offsetof(ScriptCore, entities) == 0x298, "Wrong offset ScriptCore->entities");

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

struct Game {
	int screen_res_x;
	int screen_res_y;
	GameState state;
	uint8_t padding[0x44];
	Scene* scene;		// 0x50
	uint8_t padding2[0x78];
	ModData* mod_data; // 0xD0
};

static_assert(offsetof(Game, scene) == 0x50, "Wrong offset game->scene");
static_assert(offsetof(Game, mod_data) == 0xD0, "Wrong offset game->mod_data");

#endif
