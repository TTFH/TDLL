#ifndef _TEARDOWN_STRUCTS_H
#define _TEARDOWN_STRUCTS_H

#include <stdint.h>

#include <lua.hpp>

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

//#define static_assert(cond, msg)

struct Entity {
	uint8_t padding1[0x08];
	uint8_t type;		// 0x08
	uint8_t padding2[0x03];
	uint32_t handle;	// 0x0C
	uint8_t padding3[0x30];
}; // 0x40

static_assert(sizeof(Entity) == 0x40, "Wrong Entity size");
static_assert(offsetof(Entity, type) == 0x08, "Wrong offset entity->type");
static_assert(offsetof(Entity, handle) == 0x0C, "Wrong offset entity->handle");

struct Voxels {
	uint32_t size[3];	// 0x00
	uint8_t padding1[0x04];
	uint8_t* voxels;	// 0x10
	uint8_t padding2[0x08];
	float scale;		// 0x20
	uint8_t padding3[0x30];
	uint32_t palette;	// 0x54
};

static_assert(offsetof(Voxels, voxels) == 0x10, "Wrong offset vox->voxels");
static_assert(offsetof(Voxels, scale) == 0x20, "Wrong offset vox->scale");
static_assert(offsetof(Voxels, palette) == 0x54, "Wrong offset vox->palette");

class Shape : public Entity {
public:
	uint8_t padding1[0x88];
	uint16_t shape_flags;		// 0xC8
	uint8_t padding2[0x06];
	float density;				// 0xD0
	uint8_t padding3[0x04];
	uint16_t texture_tile;		// 0xD8
	uint16_t blendtexture_tile;	// 0xDA
	float texture_weight;		// 0xDC
	float blendtexture_weight;	// 0xE0
	Vec3 texture_offset;		// 0xE4
	Voxels* vox;				// 0xF0
};

static_assert(offsetof(Shape, shape_flags) == 0xC8, "Wrong offset shape->shape_flags");
static_assert(offsetof(Shape, density) == 0xD0, "Wrong offset shape->density");
static_assert(offsetof(Shape, texture_tile) == 0xD8, "Wrong offset shape->texture_tile");
static_assert(offsetof(Shape, blendtexture_tile) == 0xDA, "Wrong offset shape->blendtexture_tile");
static_assert(offsetof(Shape, texture_weight) == 0xDC, "Wrong offset shape->texture_weight");
static_assert(offsetof(Shape, blendtexture_weight) == 0xE0, "Wrong offset shape->blendtexture_weight");
static_assert(offsetof(Shape, texture_offset) == 0xE4, "Wrong offset shape->texture_offset");
static_assert(offsetof(Shape, vox) == 0xF0, "Wrong offset shape->vox");

class Light : public Entity {
public:
	uint8_t padding1[0x04];
	uint8_t type;				// 0x44
	uint8_t padding2[0x7B];
	float radius;				// 0xC0
	uint8_t padding3[0x04];
	float cos_half_angle_rad;	// 0xC8
	uint8_t padding4[0x0C];
	Vec2 half_size;				// 0xD8
	float half_length;			// 0xE0
};

static_assert(offsetof(Light, type) == 0x44, "Wrong offset light->type");
static_assert(offsetof(Light, radius) == 0xC0, "Wrong offset light->radius");
static_assert(offsetof(Light, cos_half_angle_rad) == 0xC8, "Wrong offset light->cos_half_angle_rad");
static_assert(offsetof(Light, half_size) == 0xD8, "Wrong offset light->half_size");
static_assert(offsetof(Light, half_length) == 0xE0, "Wrong offset light->half_length");

class Water : public Entity {
public:
	Transform transform;		// 0x40
	td_vector<Vec2> vertices;	// 0x60
};

static_assert(offsetof(Water, transform) == 0x40, "Wrong offset water->transform");
static_assert(offsetof(Water, vertices) == 0x60, "Wrong offset water->vertices");

class Joint : public Entity {
public:
	uint8_t padding1[0x24];
	float size;					// 0x64
	bool collide;				// 0x68
	uint8_t padding2[0x17];
	Vec3 position1;				// 0x80
	Vec3 position2;				// 0x8C
	Vec3 axis1;					// 0x98
	Vec3 axis2;					// 0xA4
	uint8_t padding3[0x30];
	bool sound;					// 0xE0
	bool autodisable;			// 0xE1
	float connection_strength;	// 0xE4
	float disconnect_dist;		// 0xE8
};

static_assert(offsetof(Joint, size) == 0x64, "Wrong offset joint->size");
static_assert(offsetof(Joint, collide) == 0x68, "Wrong offset joint->collide");
static_assert(offsetof(Joint, position1) == 0x80, "Wrong offset joint->position1");
static_assert(offsetof(Joint, position2) == 0x8C, "Wrong offset joint->position2");
static_assert(offsetof(Joint, axis1) == 0x98, "Wrong offset joint->axis1");
static_assert(offsetof(Joint, axis2) == 0xA4, "Wrong offset joint->axis2");
static_assert(offsetof(Joint, sound) == 0xE0, "Wrong offset joint->sound");
static_assert(offsetof(Joint, autodisable) == 0xE1, "Wrong offset joint->autodisable");
static_assert(offsetof(Joint, connection_strength) == 0xE4, "Wrong offset joint->connection_strength");
static_assert(offsetof(Joint, disconnect_dist) == 0xE8, "Wrong offset joint->disconnect_dist");

class Vehicle : public Entity {
public:
	uint8_t padding[0xF8];
	float max_steer_angle;	// 0x138
};

static_assert(offsetof(Vehicle, max_steer_angle) == 0x138, "Wrong offset vehicle->max_steer_angle");

class Wheel : public Entity {
public:
	uint8_t padding1[0x38];
	Transform transform;	// 0x78
	uint8_t padding2[0x2C];
	float radius;			// 0xC0
};

static_assert(offsetof(Wheel, transform) == 0x78, "Wrong offset wheel->transform");
static_assert(offsetof(Wheel, radius) == 0xC0, "Wrong offset wheel->radius");

class Trigger : public Entity {
public:
	uint8_t padding1[0x30];
	td_vector<Vec2> vertices;	// 0x70
};

static_assert(offsetof(Trigger, vertices) == 0x70, "Wrong offset trigger->vertices");

struct LuaStateInfo {
	lua_State* state;	// 0x00
};

struct ScriptCoreInner {
	uint8_t padding[0x30];
	LuaStateInfo* state_info;	// 0x30
};

static_assert(sizeof(ScriptCoreInner) == 0x38, "Wrong ScriptCoreInner size");
static_assert(offsetof(ScriptCoreInner, state_info) == 0x30, "Wrong offset sci->state_info");

struct InternalCheck {
	uint8_t padding[0x3AC];
	uint32_t privilege;		// 0x3AC
};

static_assert(offsetof(InternalCheck, privilege) == 0x3AC, "Wrong offset ic->privilege");

struct ScriptCore {
	uint8_t padding1[0x10];
	td_string path;					// 0x10
	uint8_t padding2[0x30];
	ScriptCoreInner inner_core;		// 0x60
	uint8_t padding3[0x220];
	InternalCheck* check_internal;	// 0x2B8
};

static_assert(offsetof(ScriptCore, path) == 0x10, "Wrong offset sc->path");
static_assert(offsetof(ScriptCore, inner_core) == 0x60, "Wrong offset sc->inner_core");
static_assert(offsetof(ScriptCore, check_internal) == 0x2B8, "Wrong offset sc->check_internal");

class Script : public Entity {
public:
	td_string name;
	td_string path;
	ScriptCore core;	// 0x80
};

static_assert(offsetof(Script, core) == 0x80, "Wrong offset script->core");

struct AnimatorCore {
	uint8_t padding[0x119];
	bool is_ragdoll;	// 0x119
};

static_assert(offsetof(AnimatorCore, is_ragdoll) == 0x119, "Wrong offset anim_core->is_ragdoll");

class Animator : public Entity {
public:
	uint8_t padding[0x20];
	AnimatorCore* core;	// 0x60
};

static_assert(offsetof(Animator, core) == 0x60, "Wrong offset animator->core");

// ----------------------------------------------------------------------------

struct Fire {
	Shape* shape;
	Vec3 position;
	uint8_t padding[0x34];
}; // 0x48

static_assert(sizeof(Fire) == 0x48, "Wrong Fire size");

struct FireSystem {
	uint8_t padding[0x08];
	td_vector<Fire> fires;	// 0x08
};

static_assert(offsetof(FireSystem, fires) == 0x08, "Wrong offset firesystem->fires");

struct Material {
	uint32_t type;
	RGBA color;
	float reflectivity;
	float shininess;
	float metallic;
	float emissive;
	uint32_t is_tint;
}; // 0x28

static_assert(sizeof(Material) == 0x28, "Wrong Material size");

struct Palette {
	uint32_t padding1[3];
	Material materials[256];
	uint8_t black_tint[4 * 256];
	uint8_t yellow_tint[4 * 256];
	uint8_t rgba_tint[4 * 256];
	uint32_t padding2[13];
}; // 0x3440

static_assert(sizeof(Palette) == 0x3440, "Wrong Palette size");

struct Environment {
	uint8_t padding[0x114];
	float sun_length;	// 0x114
};

static_assert(offsetof(Environment, sun_length) == 0x114, "Wrong offset env->sun_length");

struct Scene {
	uint8_t padding1[0x38];
	FireSystem* firesystem;			// 0x38
	uint8_t padding2[0x10];
	Environment* environment;		// 0x50
	uint8_t padding3[0xA0];
	Vec3 sv_size;					// 0xF8
	uint8_t padding4[0x494];
	td_vector<Vec2> boundary;		// 0x598
	uint8_t padding5[0x428];
	td_vector<Entity*> entities;	// 0x9D0
};

static_assert(offsetof(Scene, firesystem) == 0x38, "Wrong offset scene->firesystem");
static_assert(offsetof(Scene, environment) == 0x50, "Wrong offset scene->environment");
static_assert(offsetof(Scene, sv_size) == 0xF8, "Wrong offset scene->sv_size");
static_assert(offsetof(Scene, boundary) == 0x598, "Wrong offset scene->boundary");
static_assert(offsetof(Scene, entities) == 0x9D0, "Wrong offset scene->entities");

struct Heat {
	Shape* shape;
	uint32_t x;
	uint32_t y;
	uint32_t z;
	float amount;
}; // 0x18

static_assert(sizeof(Heat) == 0x18, "Wrong Heat size");

struct Player {
	uint8_t padding[0xC30];
	td_vector<Heat> heats;	// 0xC30
};

static_assert(offsetof(Player, heats) == 0xC30, "Wrong offset player->heats");

struct Game {
	uint8_t padding1[0x50];
	Scene* scene;					// 0x50
	uint8_t padding2[0x60];
	Player* player;					// 0xB8
	uint8_t padding3[0x08];
	td_vector<Palette>* palettes;	// 0xC8
	uint8_t padding4[0xE4];
	float time_scale;				// 0x1B4
};

static_assert(offsetof(Game, scene) == 0x50, "Wrong offset game->scene");
static_assert(offsetof(Game, player) == 0xB8, "Wrong offset game->player");
static_assert(offsetof(Game, palettes) == 0xC8, "Wrong offset game->palettes");
static_assert(offsetof(Game, time_scale) == 0x1B4, "Wrong offset game->time_scale");

struct ScreenCapture {
	uint8_t padding1[0x08];
	uint32_t width;			// 0x08
	uint32_t height;		// 0x0C
	uint8_t* image_buffer;	// 0x10
	uint8_t padding2[0x08];
	td_string capture_dir;	// 0x20
};

static_assert(offsetof(ScreenCapture, width) == 0x08, "Wrong offset sc->width");
static_assert(offsetof(ScreenCapture, height) == 0x0C, "Wrong offset sc->height");
static_assert(offsetof(ScreenCapture, image_buffer) == 0x10, "Wrong offset sc->image_buffer");
static_assert(offsetof(ScreenCapture, capture_dir) == 0x20, "Wrong offset sc->capture_dir");

#endif
