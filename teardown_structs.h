#ifndef _TEARDOWN_STRUCTS_H
#define _TEARDOWN_STRUCTS_H

#include <stdint.h>
#include <stddef.h>

enum GameState : int {
	Splash = 1,
	Menu = 2,
	Play = 5,
	Editor = 6,
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
	void removeVertices() {
		size = 0;
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
};

struct Quat {
	float x, y, z, w;
};

struct Transform {
	Vector pos;
	Quat rot;
};

struct LuaStateInfo {
	lua_State* state;
};

struct Entity {
	void* unknown;
	uint8_t kind;
	uint8_t flags;
	uint8_t padding[2];
	uint32_t handle;	// 0x0C
	Entity* tree[4];
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

struct Script {
	Entity self;
	td_string editor_path;		// 0x30
	td_string file_path1;		// 0x50
	uint8_t padding1[0x10];
	td_string file_path2;		// 0x80
	td_string folder_path;		// 0xA0
	uint8_t padding2[0x58];
	LuaStateInfo* state_info;	// 0x118
};

static_assert(offsetof(Script, file_path1) == 0x50, "Wrong offset Script->file_path1");
static_assert(offsetof(Script, file_path2) == 0x80, "Wrong offset Script->file_path2");
static_assert(offsetof(Script, folder_path) == 0xA0, "Wrong offset Script->folder_path");
static_assert(offsetof(Script, state_info) == 0x118, "Wrong offset Script->state_info");

struct Scene {
	uint8_t padding1[0x80];
	Light* flashlight;				// 0x80
	uint8_t padding2[0xB0];
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
	uint8_t padding3[0x380];
	td_vector<Vertex> boundary;		// 0x568
};

static_assert(offsetof(Scene, flashlight) == 0x80, "Wrong offset Scene->flashlight");
static_assert(offsetof(Scene, bodies) == 0x138, "Wrong offset Scene->bodies");
static_assert(offsetof(Scene, boundary) == 0x568, "Wrong offset Scene->boundary");

struct Game {
	int screen_res_x;
	int screen_res_y;
	GameState state;
	uint8_t padding[0x44];
	Scene* scene;
};

static_assert(offsetof(Game, scene) == 0x50, "Wrong offset game->scene");

#endif
