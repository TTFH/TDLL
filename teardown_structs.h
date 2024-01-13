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
	td_string(const char* str) {
		int length = strlen(str);
		if (length < 32) {
			strcpy(StackBuffer, str);
			StackBuffer[31] = 0;
		} else {
			HeapBuffer = new char[length + 1];
			strcpy(HeapBuffer, str);
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

struct Vector {
	float x, y, z;
	Vector(float x, float y, float z) : x(x), y(y), z(z) {}
};

struct Quat {
	float x, y, z, w;
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

struct Body {
	Entity self;
};

struct Shape {
	Entity self;
	uint8_t padding1[0x8C];
	uint16_t texture_tile;			// 0xBC
	uint16_t blendtexture_tile = 0;
	float texture_weight;
	float blendtexture_weight = 1;
	Vector texture_offset;
};

static_assert(offsetof(Shape, texture_tile) == 0xBC, "Wrong offset Shape->texture_tile");

struct Light {
	Entity self;
};

struct Location {
	Entity self;
};

struct Water {
	Entity self;
	Transform transform;		// 0x30
	td_vector<Vertex> vertices;	// 0x50
};

struct Joint {
	Entity self;
	uint8_t padding1[0x40];
	Vector local_pos_body1;	// 0x70
	Vector local_pos_body2;
};

static_assert(offsetof(Joint, local_pos_body1) == 0x70, "Wrong offset Joint->local_pos_body1");

struct Vehicle {
	Entity self;
};

struct Wheel {
	Entity self;
	Vehicle* vehicle;	// 0x30
};

struct Screen {
	Entity self;
};

struct Trigger {
	Entity self;
};

struct StateWrapper {
	lua_State* state;
};

struct StateInfo {
	uint8_t padding[0x40];
	StateWrapper* state_wrapper;	// 0x40
};

static_assert(offsetof(StateInfo, state_wrapper) == 0x40, "Wrong offset StateInfo->state_wrapper");

struct ScriptCore {
	uint8_t padding1[8];
	float time;
	float dt;
	td_string path;			// 0x10
	td_string location;		// 0x30
	uint8_t padding2[0x18];
	StateInfo state_info;	// 0x68
};

static_assert(offsetof(ScriptCore, path) == 0x10, "Wrong offset ScriptCore->path");
static_assert(offsetof(ScriptCore, state_info) == 0x68, "Wrong offset ScriptCore->state_info");

struct Script {
	Entity self;
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

struct Game {
	int screen_res_x;
	int screen_res_y;
	GameState state;
	uint8_t padding[0x44];
	Scene* scene;			// 0x50
};

static_assert(offsetof(Game, scene) == 0x50, "Wrong offset game->scene");

#endif
