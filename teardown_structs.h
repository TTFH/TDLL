#ifndef _TEARDOWN_STRUCTS_H
#define _TEARDOWN_STRUCTS_H

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <atomic>

#include "lua_utils.h"

enum GameState : int {
	None,
	Splash,
	Menu,
	UI,
	Loading,
	MenuLoading,
	Play,
	Edit,
	Quit
}; // 0x04

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
	void setSize(uint32_t count) {
		if (count < size)
			size = count;
	}
	T get(uint32_t index) const {
		return data[index];
	}
	T& get(uint32_t index) {
		return data[index];
	}
	void set(uint32_t index, T value) {
		if (index < size)
			data[index] = value;
	}
	void remove(uint32_t index) {
		if (index < size) {
			for (uint32_t i = index; i < size - 1; i++)
				data[i] = data[i + 1];
			size--;
		}
	}
	T operator[](uint32_t index) const {
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
	Vec3 operator*(float f) const {
		return Vec3(x * f, y * f, z * f);
	}
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

struct Entity {
	void* class_ptr;
	uint8_t type;
	uint16_t flags;
	uint32_t handle;	// 0x0C
	Entity* parent;
	Entity* next_sibling;
	Entity* last_sibling;
	Entity* first_child;
	Entity* last_child;
	void* tags;
}; // 0x40

static_assert(sizeof(Entity) == 0x40, "Wrong Entity size");

class Body : public Entity {
public:
	Transform transform;
	uint8_t padding1[0x7C];
	Vec3 velocity;			// 0xD8
	Vec3 angular_velocity;	// 0xE4
	uint8_t padding2[0x48];
	bool dynamic;			// 0x138
	uint8_t padding3[7];
	uint8_t active;			// 0x140
	float friction;
	float restitution;
	uint8_t friction_mode;
	uint8_t restitution_mode;
}; // 0x170

static_assert(offsetof(Body, velocity) == 0xD8, "Wrong offset body->velocity");
static_assert(offsetof(Body, dynamic) == 0x138, "Wrong offset body->dynamic");
static_assert(offsetof(Body, active) == 0x140, "Wrong offset body->active");

struct Voxels {
	uint32_t size[3];
	uint32_t volume;
	uint8_t* voxels;		// 0x10
	void* physics_buffer;
	float scale;
	uint8_t light_mask[8];
	bool is_disconnected;	// 0x2C
	uint8_t padding1[0x27];
	uint32_t palette;		// 0x54
	uint8_t padding2[4];
	int32_t voxel_count;	// 0x5C
}; // ???

static_assert(offsetof(Voxels, voxels) == 0x10, "Wrong offset vox->voxels");
static_assert(offsetof(Voxels, is_disconnected) == 0x2C, "Wrong offset vox->is_disconnected");
static_assert(offsetof(Voxels, palette) == 0x54, "Wrong offset vox->palette");
static_assert(offsetof(Voxels, voxel_count) == 0x5C, "Wrong offset vox->voxel_count");

class Shape : public Entity {
public:
	uint8_t origin;
	Transform transform;
	uint8_t padding1[0x68];
	uint16_t shape_flags;		// 0xC8
	uint8_t collision_layer;
	uint8_t collision_mask;
	uint32_t z_u32;				// 0xCC
	float density;
	float strength;
	uint16_t texture_tile;
	uint16_t blendtexture_tile;
	float texture_weight;
	float blendtexture_weight;
	Vec3 texture_offset;
	Voxels* vox;				// 0xF0
	uint8_t padding2[8];
	float emissive_scale;		// 0x100
	bool is_broken;
}; // 0x178

static_assert(offsetof(Shape, shape_flags) == 0xC8, "Wrong offset shape->shape_flags");
static_assert(offsetof(Shape, z_u32) == 0xCC, "Wrong offset shape->z_u32");
static_assert(offsetof(Shape, vox) == 0xF0, "Wrong offset shape->vox");
static_assert(offsetof(Shape, emissive_scale) == 0x100, "Wrong offset shape->emissive_scale");

enum LightType : uint8_t {
	LightSphere = 1,
	Capsule,
	LightCone,
	Area,
}; // 0x01

class Light : public Entity {
public:
	bool enabled;
	uint8_t padding1[3];
	LightType type;			// 0x44
	Transform transform;
	uint8_t padding2[0x44];
	RGBA color;				// 0xA8
	float scale;
	float reach;
	float radius;
	float unshadowed;
	float cos_half_angle_rad;
	float penumbra;
	float fogiter;
	float fogscale;
	float half_width;
	float half_height;
	float half_length;
	float glare;			// 0xE4
	uint8_t padding3[4];
	Vec3 position;			// 0xEC
	uint8_t index;
	float flickering;
	//td_string sound_path;
	//float sound_volume;
}; // 0x1450

static_assert(offsetof(Light, type) == 0x44, "Wrong offset light->type");
static_assert(offsetof(Light, color) == 0xA8, "Wrong offset light->color");
static_assert(offsetof(Light, glare) == 0xE4, "Wrong offset light->glare");
static_assert(offsetof(Light, position) == 0xEC, "Wrong offset light->position");

class Location : public Entity {
	Transform transform;
}; // 0x60

static_assert(sizeof(Location) == 0x60, "Wrong Location size");

class Water : public Entity {
public:
	Transform transform;
	float depth;
	td_vector<Vec2> vertices;	// 0x60
	uint8_t padding1[0x388];	// the mariana trench?
	float wave;					// 0x3F8
	float ripple;
	float motion;
	float foam;
	float visibility;
	RGBA color;
}; // 0x420

static_assert(offsetof(Water, vertices) == 0x60, "Wrong offset water->vertices");
static_assert(offsetof(Water, wave) == 0x3F8, "Wrong offset water->wave");

enum JointType : int {
	Ball = 1,
	Hinge,
	Prismatic,
	JointRope,
	JointCone,
}; // 0x04

struct Rope {
	float zero;
	float segment_length;
	float slack;
	RGBA color;			// 0x0C
	uint8_t padding1[4];
	float strength;		// 0x20
	float maxstretch;
	uint8_t padding2[5];
	uint8_t active;		// 0x2D
	uint32_t segments_count;
}; // ???

static_assert(offsetof(Rope, color) == 0x0C, "Wrong offset rope->color");
static_assert(offsetof(Rope, strength) == 0x20, "Wrong offset rope->strength");
static_assert(offsetof(Rope, active) == 0x2D, "Wrong offset rope->active");

class Joint : public Entity {
public:
	Shape* shape1;
	Shape* shape2;		// 0x48
	uint8_t padding1[0x10];
	JointType type;		// 0x60
	float size;
	bool collide;
	bool connected;
	float rotstrength;
	float rotspring;	// 0x70
	uint8_t padding2[0xC];
	Vec3 position1;		// 0x80
	Vec3 position2;
	Vec3 axis1;
	Vec3 axis2;
	Quat hinge_rot;
	float limits[2];	// 0xC0
	uint8_t padding3[4];
	float max_velocity;	// 0xCC
	float strength;		// 0xD0
	uint8_t padding4[4];
	Rope* rope;			// 0xD8
	bool sound;
	bool autodisable;
	float connection_strength;
	float disconnect_dist;
}; // 0xF0

static_assert(offsetof(Joint, type) == 0x60, "Wrong offset joint->type");
static_assert(offsetof(Joint, position1) == 0x80, "Wrong offset joint->position1");
static_assert(offsetof(Joint, rope) == 0xD8, "Wrong offset joint->rope");

class Wheel;

class Vehicle : public Entity {
public:
	Body* body;
	Transform transform1;
	Transform transform2;		// 0x64
	uint8_t padding1[0x48];
	td_vector<Wheel*> wheels;	// 0xC8
	uint8_t padding2[0x44];
	float topspeed;				// 0x11C
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
	float assist_multiplier;
	float antiroll;
	float difflock;
	uint8_t padding3[4];
	td_string sound_name;
	float sound_pitch;
	uint8_t padding4[4];
	Vec3 camera;				// 0x180
	/*td_vector<> locations;	// 0x19C
	Vec3 player;				// 0x1B0
	Vec3 exit;
	Vec3 propeller;
	float smokeintensity;		// 0x1D4
	float passive_brake;		// 0x1FC
	float health;				// 0x208
	float bounds_dist;			// 0x220
	td_vector<Body*> bodies;	// 0x228
	td_vector<> exhausts;		// 0x240
	td_vector<> vitals;			// 0x298
	uint32_t main_voxel_count;	// 0x308
	bool braking;				// 0x310
	bool noroll;				// 0x331
	float brokenthreshold;		// 0x334*/
}; // 0x338

static_assert(offsetof(Vehicle, transform2) == 0x64, "Wrong offset vehicle->transform2");
static_assert(offsetof(Vehicle, wheels) == 0xC8, "Wrong offset vehicle->wheels");
static_assert(offsetof(Vehicle, topspeed) == 0x11C, "Wrong offset vehicle->topspeed");
static_assert(offsetof(Vehicle, camera) == 0x180, "Wrong offset vehicle->camera");

class Wheel : public Entity {
public:
	Vehicle* vehicle;
	Body* vehicle_body;
	Body* body;
	Shape* shape;
	Shape* ground_shape;
	uint32_t ground_voxel_pos[3];
	bool on_ground;
	Transform transform;
	Transform transform2;
	float steer;
	float drive;
	float travel_up;
	float travel_down;
	float radius;
	float width;
	float stance;			// 0xC8
	uint8_t padding1[0xC];
	float angular_speed;	// 0xD8
	uint8_t padding2[0x24];
	float vertical_offset;	// 0x100
}; // 0x118

static_assert(offsetof(Wheel, stance) == 0xC8, "Wrong offset wheel->stance");
static_assert(offsetof(Wheel, angular_speed) == 0xD8, "Wrong offset wheel->angular_speed");
static_assert(offsetof(Wheel, vertical_offset) == 0x100, "Wrong offset wheel->vertical_offset");

class Screen : public Entity {
public:
	Transform transform;
	Vec2 size;
	float bulge;
	uint32_t resolution_x;
	uint32_t resolution_y;
	td_string script;		// 0x70
	uint8_t padding[0x20];
	bool enabled;			// 0xB0
	bool interactive;
	float emissive;
	float fxraster;
	float fxca;
	float fxnoise;
	float fxglitch;
}; // 0x2198

static_assert(offsetof(Screen, script) == 0x70, "Wrong offset screen->script");
static_assert(offsetof(Screen, enabled) == 0xB0, "Wrong offset screen->enabled");

enum TriggerType : int {
	TriggerSphere = 1,
	TriggerBox,
	TriggerPolygon,
}; // 0x04

class Trigger : public Entity {
public:
	Transform transform;
	TriggerType type;
	float sphere_size;
	Vec3 half_box_size;
	td_vector<Vec2> vertices;	// 0x70
	uint8_t padding1[0x80];
	float polygon_size;			// 0x100
	uint8_t padding2[4];
	td_string sound_path;		// 0x108
	float sound_ramp;
	uint8_t sound_type;
	float sound_volume;
}; // 0x138

static_assert(offsetof(Trigger, vertices) == 0x70, "Wrong offset trigger->vertices");
static_assert(offsetof(Trigger, polygon_size) == 0x100, "Wrong offset trigger->polygon_size");
static_assert(offsetof(Trigger, sound_path) == 0x108, "Wrong offset trigger->sound_path");

struct LuaStateInfo {
	lua_State* state;
};

// non-ptr used for RegisterLuaFunction
struct ScriptCoreInner {
	uint8_t padding[0x30];
	LuaStateInfo* state_info;
};

struct InternalCheck {
	uint8_t padding[0x38C];
	uint32_t privilege;
};

// non-ptr used for LuaApiFunction
struct ScriptCore {
	uint8_t padding1[8];
	float time;
	float dt;
	td_string path;
	td_string location;
	uint8_t padding2[0x18];
	ScriptCoreInner inner_core;		// 0x68
	/*
	bool has_init;	// 0xA1
	bool has_tick;
	bool has_update;
	bool has_ppu;
	bool has_render;
	bool has_draw;
	bool has_handle_cmd;

	float tick_time;	// 0xAC
	float update_time;

	td_vector<> transitions;	// 0x1D8
	td_vector<> params;			// 0x214
	*/
	uint8_t padding4[0x1F8];		// an onion?
	td_vector<uint32_t> entities;	// 0x298
	//td_vector<> sounds;
	uint8_t padding5[0x18];
	InternalCheck* check_internal;	// 0x2C0
}; // 0x21A8

static_assert(offsetof(ScriptCore, inner_core) == 0x68, "Wrong offset sc->inner_core");
static_assert(offsetof(ScriptCore, entities) == 0x298, "Wrong offset sc->entities");
static_assert(offsetof(ScriptCore, check_internal) == 0x2C0, "Wrong offset sc->check_internal");

class Script : public Entity {
public:
	td_string name;
	td_string path;
	ScriptCore core;
}; // 0x2228

class AnimatorCore {
	
}; // 0x120

class Animator : public Entity {
public:
	Transform transform;
	uint8_t padding[4];
	AnimatorCore* anim_core;
	td_string name;
	td_string path;
}; // 0xB0

struct Fire {
	Shape* shape;
	Vec3 position;
	float max_time;
	float time;
	bool painted;
	bool broken;
	uint32_t spawned_count;
	uint8_t padding1[0x24];
}; // 0x48

struct FireSystem {
	uint8_t padding[8];
	td_vector<Fire> fires;
};

struct Environment {

}; // ??

struct Registry {
	
}; // 0x60

struct Scene {
	uint8_t padding1[0x38];
	FireSystem* firesystem;
	td_vector<void*> projectiles;
	Environment* environment;
	uint8_t padding2[8];
	Transform spawnpoint;			// 0x60
	uint8_t padding3[4];
	Light* flashlight;				// 0x80
	Script* explosion_lua;
	Script* achievements_lua;
	Script* characters_lua;			// 0x98
	uint8_t padding4[0x50];
	Vec3 sv_size;					// 0xF0
	uint8_t padding5[0x4C];
	td_vector<Body*> bodies;		// 0x148
	td_vector<Shape*> shapes;		// 0x158
	td_vector<Light*> lights;		// 0x168
	td_vector<Location*> locations;	// 0x178
	td_vector<Water*> waters;		// 0x188
	td_vector<Joint*> joints;		// 0x198
	td_vector<Vehicle*> vehicles;	// 0x1A8
	td_vector<Wheel*> wheels;		// 0x1B8
	td_vector<Screen*> screens;		// 0x1C8
	td_vector<Trigger*> triggers;	// 0x1D8
	td_vector<Script*> scripts;		// 0x1E8
	td_vector<Animator*> animators;	// 0x1F8
	td_vector<Entity*> top_level;	// 0x208
	uint8_t padding6[0x378];
	td_vector<Vec2> boundary;		// 0x590
	uint8_t padding7[0x428];
	td_vector<Entity*> entities;	// 0x9C8
}; // 0xA70

static_assert(offsetof(Scene, spawnpoint) == 0x60, "Wrong offset scene->spawnpoint");
static_assert(offsetof(Scene, flashlight) == 0x80, "Wrong offset scene->flashlight");
static_assert(offsetof(Scene, characters_lua) == 0x98, "Wrong offset scene->characters_lua");
static_assert(offsetof(Scene, sv_size) == 0xF0, "Wrong offset scene->sv_size");
static_assert(offsetof(Scene, bodies) == 0x148, "Wrong offset scene->bodies");
static_assert(offsetof(Scene, animators) == 0x1F8, "Wrong offset scene->animators");
static_assert(offsetof(Scene, boundary) == 0x590, "Wrong offset scene->boundary");
static_assert(offsetof(Scene, entities) == 0x9C8, "Wrong offset scene->entities");

namespace EditorEntityTypes {
	const int Invalid = 0;
	const int Entity = 1;
	const int Scene = 2;
	const int Environment = 3;
	const int PostProcessing = 4;
	const int Group = 5;
	const int Comment = 6;
	const int Compound = 7;
	const int Instance = 8;
	const int Body = 9;
	const int Shape	= 10;
	const int Voxels = 11;
	const int Light = 12;
	const int Spawnpoint = 13;
	const int Location = 14;
	const int Voxscript = 15;
	const int Joint = 16;
	const int Water = 17;
	const int Voxbox = 18;
	const int Voxagon = 19;
	const int Rope = 20;
	const int Boundary = 21;
	const int Vehicle = 22;
	const int Wheel = 23;
	const int Screen = 24;
	const int Trigger = 25;
	const int Script = 26;
	const int Animator = 27;
};

struct EditorEntity {
	uint8_t padding[0x174];
	uint32_t type;				// 0x174
	td_vector<Vec2> vertices;	// 0x178
};

struct Editor {
	uint8_t padding[0x28];
	EditorEntity* selected;		// 0x28
}; // 0xAF58

struct Player {
	Transform transform;
	uint8_t padding1[0x1C];
	Vec3 velocity;			// 0x8C
	uint8_t padding2[0xD0];
	float pitch;			// 0x16C
	float yaw;
	
	//bool is_jumping;	// 0x185
	
	uint8_t padding3[0x94];
	float health;			// 0x208

	//Body* tool_body;		// 0xE08

	//float tool_recoil;	// 0xE70

	uint8_t padding4[0x8A4];
	float time_underwater;	// 0xFD8
	uint8_t padding5[0x3040];
	float transition_timer;	// 0x401C
	/*
	Transform tool_tr_override; // 0x4030
	Vec3 tool_offset;
	float tool_sway;
	bool override_tr;
	bool hand_tr_override;
	*/
	//Transform right_hand_tr;	// 0x4098
	//Transform left_hand_tr;
	//bool dual_handed;

	uint8_t padding6[0x354];
	float bluetide_timer;	// 0x43F8
	float bluetide_power;

	/*
	bool third_person;	// 0x441D
	bool transition_3rd;
	bool first_person;
	bool transition_1st;
	*/

	//Animator* animator;	//0x45A0

	//td_string spawn_tool;	//0x45B0
}; // 0x45E8

struct Material {
	uint32_t type;
	RGBA color;
	float reflectivity;
	float shininess;
	float metallic;
	float emissive;
	uint32_t is_tint;
}; // 0x28

struct Palette {
	uint32_t padding1[2];
	bool has_transparent;
	uint8_t padding2[3];
	Material materials[256];
	uint8_t black_tint[4 * 256];
	uint8_t yellow_tint[4 * 256];
	uint8_t rgba_tint[4 * 256];
	uint32_t padding3[13];
}; // 0x3440

struct Game {
	uint32_t screen_width;
	uint32_t screen_height;
	GameState state;
	uint8_t padding1[0x44];
	// haptics
	Scene* scene;					// 0x50
	// sounds, camera
	uint8_t padding2[0x10];
	Editor* editor;					// 0x68
	//ScriptCore* splash, loading, hud, menu, unk, ui_common
	uint8_t padding3[0x48];
	Player* player;					// 0xB8
	uint8_t padding4[8];
	td_vector<Palette>* palettes;	// 0xC8
	uint8_t padding5[0xE4];
	float time_scale;				// 0x1B4
}; // 0x2000

static_assert(offsetof(Game, scene) == 0x50, "Wrong offset game->scene");
static_assert(offsetof(Game, editor) == 0x68, "Wrong offset game->editor");
static_assert(offsetof(Game, player) == 0xB8, "Wrong offset game->player");
static_assert(offsetof(Game, palettes) == 0xC8, "Wrong offset game->palettes");
static_assert(offsetof(Game, time_scale) == 0x1B4, "Wrong offset game->time_scale");

struct ScreenCapture {
	uint8_t padding1[8];
	uint32_t width;			// 0x08
	uint32_t height;
	uint8_t* image_buffer;
	uint32_t frame;
	uint8_t padding2[4];
	td_string capture_dir;	// 0x20
}; // 0xF8

static_assert(offsetof(ScreenCapture, capture_dir) == 0x20, "Wrong offset sc->capture_dir");

#endif
