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
	td_string() { }
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
	T& get(uint32_t index) {
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
	Vertex() : x(0), y(0) { }
	Vertex(float x, float y) : x(x), y(y) { }
};

struct RGBA {
	float r, g, b, a;
};

struct Vec3 {
	float x, y, z;
	Vec3() : x(0), y(0), z(0) { }
	Vec3(float x, float y, float z) : x(x), y(y), z(z) { }
	Vec3 operator*(float f) const {
		return Vec3(x * f, y * f, z * f);
	}
};

struct Quat {
	float x, y, z, w;
	Quat() : x(0), y(0), z(0), w(1) { }
};

struct Transform {
	Vec3 pos;
	Quat rot;
}; // 0x1C

struct Entity {
	uint8_t padding1[8];
	uint8_t kind;
	uint8_t padding2;
	uint16_t flags;
	uint32_t handle;	// 0x0C
	Entity* parent;
	Entity* sibling;
	Entity* child;
	uint8_t padding3[8];
}; // 0x30

static_assert(sizeof(Entity) == 0x30, "Wrong size Entity");
static_assert(offsetof(Entity, handle) == 0x0C, "Wrong offset Entity->handle");

class Body : public Entity {
public:
	Transform transform;
	uint8_t padding1[0x38];
	Vec3 velocity;				// 0x84
	Vec3 angular_velocity;
	uint8_t padding2[0x48];
	bool dynamic;				// 0xE4
	uint8_t padding3[0x7];
	uint8_t active;				// 0XEC
	float friction;				// 0xF0
	float restitution;
	uint8_t friction_mode;
	uint8_t restitution_mode;
}; // 0x120

static_assert(offsetof(Body, velocity) == 0x84, "Wrong offset Body->velocity");
static_assert(offsetof(Body, dynamic) == 0xE4, "Wrong offset Body->dynamic");
static_assert(offsetof(Body, active) == 0xEC, "Wrong offset Body->active");
static_assert(offsetof(Body, friction) == 0xF0, "Wrong offset Body->friction");

struct Voxels {
	uint32_t size[3];
	uint32_t volume;
	uint8_t* voxels;
	void* physics_buffer;
	float scale;			// 0x20
	uint8_t unknown1[48];
	uint32_t palette;		// 0x54
	uint8_t unknown2[4];
	int32_t voxel_count;
}; // 0x68

static_assert(offsetof(Voxels, scale) == 0x20, "Wrong offset Voxels->scale");
static_assert(offsetof(Voxels, palette) == 0x54, "Wrong offset Voxels->palette");

class Shape : public Entity {
public:
	uint8_t origin;
	Transform transform;
	uint8_t padding1[0x68];
	uint16_t shape_flags;
	uint8_t collision_layer;
	uint8_t collision_mask;
	float density;				// 0xBC
	uint8_t padding2[4];
	uint16_t texture_tile;		// 0xC4
	uint16_t blendtexture_tile;
	float texture_weight;
	float blendtexture_weight;
	Vec3 texture_offset;
	uint8_t padding3[4];
	Voxels* vox;				// 0xE0
	uint8_t padding4[8];
	float scale;
	uint32_t z_u32_1;
	uint32_t z_u32_2;
	bool is_disconnected;		// 0xFC
}; // 0x150

static_assert(offsetof(Shape, shape_flags) == 0xB8, "Wrong offset Shape->shape_flags");
static_assert(offsetof(Shape, density) == 0xBC, "Wrong offset Shape->density");
static_assert(offsetof(Shape, texture_tile) == 0xC4, "Wrong offset Shape->texture_tile");
static_assert(offsetof(Shape, vox) == 0xE0, "Wrong offset Shape->vox");
static_assert(offsetof(Shape, is_disconnected) == 0xFC, "Wrong offset Shape->is_disconnected");

enum LightType : int {
	Sphere = 1,
	Capsule,
	Cone,
	Area,
};

class Light : public Entity {
public:
	bool is_on;
	uint8_t padding1[3];
	uint8_t type;				// 0x34
	Transform transform;		// 0x38
	uint8_t padding2[0x40];
	RGBA color;					// 0x94
	float scale;
	float reach;
	float radius;				// 0xAC for sphere, capsule and cone
	float unshadowed;			// 0xB0
	float cos_half_angle_rad;	// 0xB4 for cone
	float penumbra;
	float fogiter;
	float fogscale;
	float half_width;			// 0xC4 for area
	float half_height;			// 0xC8 for area
	float half_length;			// 0xCC for capsule
	float glare;
	uint8_t padding3[4];
	Vec3 position;				// 0xD8
	uint8_t index;
	float flickering;			// 0xE8
}; // 0x1438

static_assert(offsetof(Light, type) == 0x34, "Wrong offset Light->type");
static_assert(offsetof(Light, transform) == 0x38, "Wrong offset Light->transform");
static_assert(offsetof(Light, color) == 0x94, "Wrong offset Light->color");
static_assert(offsetof(Light, radius) == 0xAC, "Wrong offset Light->radius");
static_assert(offsetof(Light, half_width) == 0xC4, "Wrong offset Light->half_width");
static_assert(offsetof(Light, position) == 0xD8, "Wrong offset Light->position");

class Location : public Entity {
	Transform transform;
}; // 0x50

class Water : public Entity {
public:
	Transform transform;		// 0x30
	float depth;
	td_vector<Vertex> vertices;	// 0x50
}; // 0x410

static_assert(offsetof(Water, vertices) == 0x50, "Wrong offset Water->vertices");

enum JointType : int {
	Ball = 1,
	Hinge,
	Prismatic,
	_Rope,
};

struct Rope {
	float z_f32;
	float segment_length;
	float slack;
	RGBA color;		// 0xC
};

class Joint : public Entity {
public:
	uint8_t padding1[0x20];
	JointType type;		// 0x50
	float size;
	bool collide;
	uint8_t padding3[0x17];
	Vec3 local_pos1;	// 0x70
	Vec3 local_pos2;
	Vec3 local_rot1;
	Vec3 local_rot2;
	uint8_t padding4[0x28];
	Rope* rope;			// 0xC8
	bool sound;			// 0xD0
	bool autodisable;	// 0xD1
	float z_f32_1;
	float z_f32_2;
}; // 0xE0

static_assert(offsetof(Joint, type) == 0x50, "Wrong offset Joint->type");
static_assert(offsetof(Joint, local_pos1) == 0x70, "Wrong offset Joint->local_pos1");
static_assert(offsetof(Joint, rope) == 0xC8, "Wrong offset Joint->rope");

class Vehicle : public Entity {
public:
	Body* body;
	Transform transform1;
	Transform transform2;
	uint8_t padding1[0x94];	// 0x70
	float topspeed;			// 0x104
	float top_speed_clamp;
	float spring;
	float damping;
	float acceleration;
	float strength;
	float friction;
	float max_steer_angle;
	bool handbrake;
	float antispin;
	float steerassist;
	float z_f32_1;
	float antiroll;
	float difflock;
	uint8_t padding2[4];
	td_string sound_name;
	float sound_pitch;
	uint8_t padding3[4];
	Vec3 camera;
	Vec3 player;
	Vec3 exit;
	Vec3 propeller;
	float smokeintensity;	// 0x198
	uint8_t padding4[0x24];
	float passive_brake;	// 0x1C0
}; // 0x2F8

static_assert(offsetof(Vehicle, topspeed) == 0x104, "Wrong offset Vehicle->topspeed");
static_assert(offsetof(Vehicle, smokeintensity) == 0x198, "Wrong offset Vehicle->smokeintensity");

class Wheel : public Entity {
public:
	Vehicle* vehicle;		// 0x30
	Body* vehicle_body;
	Body* body;
	Shape* shape;
	Shape* ground_shape;
	int ground_voxel_pos[3];
	bool z_u8;
	Transform transform;	// 0x68
	Transform transform2;
	float steer;
	float drive;
	float travel_up;
	float travel_down;
	float radius;			// 0xB0
	float width;
	float z_f32_1;			// 0xB8
	uint8_t padding[0x34];
	float z_f32_2;			// 0xF0
}; // 0x108

static_assert(offsetof(Wheel, transform) == 0x68, "Wrong offset Wheel->transform");
static_assert(offsetof(Wheel, radius) == 0xB0, "Wrong offset Wheel->radius");

class Screen : public Entity {
public:
	Transform transform;
	Vertex size;
	float bulge;
	int resolution_x;
	int resolution_y;
	td_string script;
	uint8_t padding[0x20];
	bool enabled;		// 0xA0
	bool interactive;
	float emissive;		// 0xA4
	float fxraster;
	float fxca;
	float fxnoise;
	float fxglitch;
}; // 0x1AD8

static_assert(offsetof(Screen, enabled) == 0xA0, "Wrong offset Screen->enabled");
static_assert(offsetof(Screen, emissive) == 0xA4, "Wrong offset Screen->emissive");

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
	Vec3 half_box_size;			// 0x54
	td_vector<Vertex> vertices;	// 0x60
	uint8_t padding2[0x80];
	float polygon_size; 		// 0xF0
	uint8_t padding3[0x28];
	uint8_t sound_z_u8;			// 0x11C
}; // 0x128

static_assert(offsetof(Trigger, type) == 0x4C, "Wrong offset Trigger->type");
static_assert(offsetof(Trigger, polygon_size) == 0xF0, "Wrong offset Trigger->polygon_size");
static_assert(offsetof(Trigger, sound_z_u8) == 0x11C, "Wrong offset Trigger->sound_z_u8");

struct ReturnInfo {
	lua_State* L;
	int count;
	int max;
};

struct LuaStateInfo {
	lua_State* state;
};

struct ScriptCoreInner {
	uint8_t padding[0x30];
	LuaStateInfo* state_info;
};

struct ScriptUiStatus {
	uint8_t padding[0x348];
	int align_h;			// 0x348
	int align_v;			// 0x34C
};

struct ScriptCore {
	uint8_t padding1[8];
	float time;
	float dt;
	td_string path;					// 0x10
	td_string location;				// 0x30
	uint8_t padding2[0x18];
	ScriptCoreInner inner_core;		// 0x68
	uint8_t padding3[8];
	float tick_time;				// 0xA8
	float update_time;
	uint8_t padding4[0x1D8];
	td_vector<uint32_t> entities;	// 0x288
	uint8_t padding5[0x10];
	ScriptUiStatus* ui_status;		// 0x2A8
}; // 0x1AE0

static_assert(offsetof(ScriptCore, path) == 0x10, "Wrong offset ScriptCore->path");
static_assert(offsetof(ScriptCore, inner_core) == 0x68, "Wrong offset ScriptCore->inner_core");
static_assert(offsetof(ScriptCore, entities) == 0x288, "Wrong offset ScriptCore->entities");
static_assert(offsetof(ScriptCore, ui_status) == 0x2A8, "Wrong offset ScriptCore->ui_status");

class Script : public Entity {
public:
	td_string name;		// 0x30
	td_string path;		// 0x50
	ScriptCore core;	// 0x70
}; // 0x1B50

static_assert(offsetof(Script, name) == 0x30, "Wrong offset Script->name");
static_assert(offsetof(Script, core) == 0x70, "Wrong offset Script->core");

struct Fire {
	Shape* shape;
	Vec3 position;
	float max_time;
	float time;
	bool painted;
	bool broken;
	uint8_t padding1[2];
	uint32_t spawned_count;	// 0x20
	uint8_t padding2[0x24];
}; // 0x48

static_assert(sizeof(Fire) == 0x48, "Wrong size Fire");
static_assert(offsetof(Fire, spawned_count) == 0x20, "Wrong offset Fire->spawned_count");

struct FireSystem {
	uint8_t padding[8];
	td_vector<Fire> fires;	// 0x08
};

struct Scene {
	uint8_t padding1[0x38];
	FireSystem* firesystem;			// 0x38
	td_vector<void*> projectiles;
	uint8_t padding2[0x10];
	Transform spawnpoint;			// 0x60
	uint8_t padding3[4];
	Light* flashlight;				// 0x80
	Script* explosion_lua;
	Script* achievements_lua;
	uint8_t padding4[0x48];
	Vec3 sv_size;					// 0xE0
	uint8_t padding5[0x4C];
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
	td_vector<Entity*> top_level;	// 0x1E8
	uint8_t padding6[0x370];
	td_vector<Vertex> boundary;		// 0x568
	uint8_t padding7[0x390];
	td_vector<Entity*> entities;	// 0x908
	uint8_t padding8[0x28];
	bool has_snow;					// 0x940
	uint8_t padding9[0x23];
	int z_st2;						// 0x964
};

static_assert(offsetof(Scene, firesystem) == 0x38, "Wrong offset Scene->firesystem");
static_assert(offsetof(Scene, spawnpoint) == 0x60, "Wrong offset Scene->spawnpoint");
static_assert(offsetof(Scene, flashlight) == 0x80, "Wrong offset Scene->flashlight");
static_assert(offsetof(Scene, sv_size) == 0xE0, "Wrong offset Scene->sv_size");
static_assert(offsetof(Scene, bodies) == 0x138, "Wrong offset Scene->bodies");
static_assert(offsetof(Scene, boundary) == 0x568, "Wrong offset Scene->boundary");
static_assert(offsetof(Scene, entities) == 0x908, "Wrong offset Scene->entities");
static_assert(offsetof(Scene, has_snow) == 0x940, "Wrong offset Scene->has_snow");
static_assert(offsetof(Scene, z_st2) == 0x964, "Wrong offset Scene->z_st2");

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
	uint32_t is_tint;
}; // 0x28

static_assert(sizeof(Material) == 0x28, "Wrong size Material");

struct Palette {
	uint32_t padding1[2];
	uint8_t z_u8;
	uint8_t padding2[3];
	Material materials[256];
	uint8_t black_tint[4 * 256];
	uint8_t yellow_tint[4 * 256];
	uint8_t rgba_tint[4 * 256];
	uint32_t padding3[13];
}; // 0x3440

static_assert(sizeof(Palette) == 0x3440, "Wrong size Palette");

struct Player {
	Transform transform;
	uint8_t padding1[0x1C];
	Vec3 velocity;			// 0x38
	uint8_t padding2[0xD0];
	float pitch;			// 0x114
	float yaw;
	uint8_t padding3[0x94];
	float health;			// 0x1B0
	uint8_t padding4[0x8A4];
	float time_underwater;	// 0xA58
	uint8_t padding5[0x3040];
	float transition_timer;	// 0x3A9C
	uint8_t padding6[0x354];
	float bluetide_timer;	// 0x3DF4
	float bluetide_power;
};

static_assert(offsetof(Player, velocity) == 0x38, "Wrong offset player->velocity");
static_assert(offsetof(Player, pitch) == 0x114, "Wrong offset player->pitch");
static_assert(offsetof(Player, health) == 0x1B0, "Wrong offset player->health");
static_assert(offsetof(Player, time_underwater) == 0xA58, "Wrong offset player->time_underwater");
static_assert(offsetof(Player, transition_timer) == 0x3A9C, "Wrong offset player->transition_timer");
static_assert(offsetof(Player, bluetide_timer) == 0x3DF4, "Wrong offset player->bluetide_timer");

struct Game {
	int screen_res_x;
	int screen_res_y;
	GameState state;
	uint8_t padding1[0x44];
	Scene* scene;					// 0x50
	uint8_t padding2[0x60];
	Player* player;					// 0xB8
	uint8_t padding3[0x8];
	td_vector<Palette>* palettes;	// 0xC8
	uint8_t padding4[8];
	ModData* mod_data;				// 0xD8
	uint8_t padding5[0xCC];
	float time_scale;				// 0x1AC
}; // 0x6A8

static_assert(offsetof(Game, scene) == 0x50, "Wrong offset game->scene");
static_assert(offsetof(Game, player) == 0xB8, "Wrong offset game->player");
static_assert(offsetof(Game, palettes) == 0xC8, "Wrong offset game->palette");
static_assert(offsetof(Game, mod_data) == 0xD8, "Wrong offset game->mod_data");
static_assert(offsetof(Game, time_scale) == 0x1AC, "Wrong offset game->time_scale");

/*struct Environment {
	// stuff goes here
};*/

#endif
