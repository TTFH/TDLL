#ifndef _TEARDOWN_STRUCTS_H
#define _TEARDOWN_STRUCTS_H

#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "lua_utils.h"

class td_string {
	union {
		char* heap;
		char stack[32] = { 0 };
	};
public:
	const char* c_str() const {
		return stack[31] != '\0' ? heap : &stack[0];
	}
}; // 0x20

template<typename T>
class td_vector {
	uint32_t size = 0;
	uint32_t capacity = 0;
	T* data = nullptr;
public:
	uint32_t getSize() const {
		return size;
	}
	void clear() {
		size = 0;
	}
	T& get(uint32_t index) {
		return data[index];
	}
	T& operator[](uint32_t index) {
		return data[index];
	}
}; // 0x10

struct Vec2 {
	float x, y;
	Vec2() : x(0), y(0) { }
	Vec2(float x, float y) : x(x), y(y) { }
}; // 0x08

struct Vec3 {
	float x, y, z;
	Vec3() : x(0), y(0), z(0) { }
	Vec3(float x, float y, float z) : x(x), y(y), z(z) { }
}; // 0x0C

struct RGBA {
	float r, g, b, a;
}; // 0x10

struct Quat {
	float x, y, z, w;
	Quat() : x(0), y(0), z(0), w(1) { }
}; // 0x10

struct Transform {
	Vec3 pos;
	Quat rot;
}; // 0x1C

namespace EntityType {
	const uint8_t Unknown = 0;
	const uint8_t Body = 1;
	const uint8_t Shape = 2;
	const uint8_t Light = 3;
	const uint8_t Location = 4;
	const uint8_t Water = 5;
	const uint8_t Joint = 6;
	const uint8_t Vehicle = 7;
	const uint8_t Wheel = 8;
	const uint8_t Screen = 9;
	const uint8_t Trigger = 10;
	const uint8_t Script = 11;
	const uint8_t Animator = 12;
};

namespace LightType {
	const uint8_t Sphere = 1;
	const uint8_t Capsule = 2;
	const uint8_t Cone = 3;
	const uint8_t Area = 4;
};

namespace ShapeFlags {
	const int Physical = 1 << 4;
}

// ----------------------------------------------------------------------------

struct Entity {
	uint8_t type;
	uint32_t handle;
};

struct Voxels {
	uint32_t size[3];
	uint32_t volume;
	uint8_t* voxels;
	float scale;
	uint32_t palette;
};

class Shape : public Entity {
public:
	uint16_t shape_flags;
	float density;
	uint16_t texture_tile;
	uint16_t blendtexture_tile;
	float texture_weight;
	float blendtexture_weight;
	Vec3 texture_offset;
	Voxels* vox;
};

class Light : public Entity {
public:
	uint8_t type;
	float radius;
	float cos_half_angle_rad;
	float half_width;
	float half_height;
	float half_length;
};

class Water : public Entity {
public:
	Transform transform;
	td_vector<Vec2> vertices;
};

class Joint : public Entity {
public:
	float size;
	bool collide;
	Vec3 position1;
	Vec3 position2;
	Vec3 axis1;
	Vec3 axis2;
	bool sound;
	bool autodisable;
	float connection_strength;
	float disconnect_dist;
};

class Vehicle : public Entity {
public:
	float max_steer_angle;
};

class Wheel : public Entity {
public:
	Transform transform;
	float radius;
};

class Trigger : public Entity {
public:
	td_vector<Vec2> vertices;
};

struct LuaStateInfo {
	lua_State* state;
};

struct ScriptCoreInner {
	LuaStateInfo* state_info;
};

struct InternalCheck {
	uint32_t privilege;
};

struct ScriptCore {
	td_string path;
	ScriptCoreInner inner_core;
	InternalCheck* check_internal;
};

class Script : public Entity {
public:
	td_string path;
	ScriptCore core;
};

struct AnimatorCore {
	bool is_ragdoll;
};

class Animator : public Entity {
public:
	AnimatorCore* anim_core;
};

struct ScreenCapture {
	uint32_t width;
	uint32_t height;
	uint8_t* image_buffer;
	td_string capture_dir;
};

struct Material {
	uint32_t type;
	RGBA color;
	float reflectivity;
	float shininess;
	float metallic;
	float emissive;
};

struct Palette {
	Material materials[256];
	uint8_t black_tint[4 * 256];
	uint8_t yellow_tint[4 * 256];
	uint8_t rgba_tint[4 * 256];
};

struct Fire {
	Shape* shape;
	Vec3 position;
}; 

struct FireSystem {
	td_vector<Fire> fires;
};

struct Heat {
	Shape* shape;
	uint32_t x;
	uint32_t y;
	uint32_t z;
	float amount;
};

struct Player {
	td_vector<Heat> heats;
};

struct Environment {
	float sun_length;
};

struct Scene {
	FireSystem* firesystem;
	Environment* environment;
	Vec3 sv_size;
	td_vector<Vec2> boundary;
	td_vector<Entity*> entities;
};

struct Game {
	Scene* scene;
	Player* player;
	td_vector<Palette>* palettes;
	float time_scale;
};

//static_assert(sizeof(Entity) == 0x40, "Wrong Entity size");
//static_assert(sizeof(Fire) == 0x48, "Wrong Fire size");
//static_assert(sizeof(Heat) == 0x18, "Wrong Heat size");
//static_assert(sizeof(Palette) == 0x3440, "Wrong Palette size");

#endif
