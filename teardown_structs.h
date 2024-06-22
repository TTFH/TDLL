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
};

class td_string {
	union {
		char* heap;
		char stack[32] = { 0 };
	};
public:
	td_string() { }
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
};

struct Vec3 {
	float x, y, z;
	Vec3() : x(0), y(0), z(0) { }
	Vec3(float x, float y, float z) : x(x), y(y), z(z) { }
	Vec3 operator*(float f) const {
		return Vec3(x * f, y * f, z * f);
	}
};

struct RGBA {
	float r, g, b, a;
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
	void* class_ptr;
	uint8_t type;
	uint8_t padding;
	uint16_t flags;
	uint32_t handle;	// 0xC
	Entity* parent;
	Entity* sibling;
	Entity* child;		// ?
	Entity* first_child;
}; // 0x30

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

struct Voxels {
	uint32_t size[3];
	uint32_t volume;
	uint8_t* voxels;
	void* physics_buffer;
	float scale;			// 0x20
	uint32_t light_mask1;	// 0x24
	uint32_t light_mask2;	// 0x28
	bool is_disconnected;	// 0x2C
	uint8_t padding1[0x27];
	uint32_t palette;		// 0x54
	uint8_t padding2[4];
	int32_t voxel_count;
}; // 0x68

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
}; // 0x150

enum LightType : int {
	Sphere = 1,
	Capsule,
	LightCone,
	Area,
};

class Light : public Entity {
public:
	bool enabled;
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

class Location : public Entity {
	Transform transform;
}; // 0x50

class Water : public Entity {
public:
	Transform transform;		// 0x30
	float depth;
	td_vector<Vec2> vertices;	// 0x50
}; // 0x410

enum JointType : int {
	Ball = 1,
	Hinge,
	Prismatic,
	JointRope,
	JointCone,
};

struct Rope {
	float z_f32;	// 0x0 (0.0)
	float segment_length;
	float slack;
	RGBA color;		// 0xC
}; // 0x444C

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
	float connection_strength;	// 0xD4
	float disconnect_dist;		// 0xD8
}; // 0xE0

struct Vital {
	Entity* body;
	Vec3 position;
	float radius;	// 0x14
	int nearby_voxels;
}; // 0x20

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
	float assist_multiplier;	// 0x130
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
	uint8_t padding5[0x1C];
	td_vector<Entity*> bodies; // 0x1E0
	uint8_t padding6[0x68];
	td_vector<Vital> vitals; // 0x258
}; // 0x2F8

class Wheel : public Entity {
public:
	Vehicle* vehicle;		// 0x30
	Body* vehicle_body;
	Body* body;
	Shape* shape;
	Shape* ground_shape;
	int ground_voxel_pos[3];
	bool on_ground;			// 0x64
	Transform transform;	// 0x68
	Transform transform2;
	float steer;
	float drive;
	float travel_up;
	float travel_down;
	float radius;			// 0xB0
	float width;
	float stance;			// 0xB8
	uint8_t padding[0x34];
	float vertical_offset;	// 0xF0
}; // 0x108

class Screen : public Entity {
public:
	Transform transform;
	Vec2 size;
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

enum TriggerType : int {
	TrSphere = 1,
	TrBox,
	TrPolygon,
};

class Trigger : public Entity {
public:
	uint8_t padding1[0x1C];
	TriggerType type;			// 0x4C
	float sphere_size;			// 0x50
	Vec3 half_box_size;			// 0x54
	td_vector<Vec2> vertices;	// 0x60
	uint8_t padding2[0x80];
	float polygon_size; 		// 0xF0
	uint8_t padding3[0x28];
	uint8_t sound_type;			// 0x11C
}; // 0x128

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

class Script : public Entity {
public:
	td_string name;		// 0x30
	td_string path;		// 0x50
	uint8_t padding[0x10];
	ScriptCore core;	// 0x80
}; // 0x1B50 + 10 ?

class Animator : public Entity {
public:

}; // 0x

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

struct FireSystem {
	uint8_t padding[8];
	td_vector<Fire> fires;	// 0x08
}; // 0x30

struct Environment {
	
}; // 0x1A8

struct Scene {
	uint8_t padding1[0x38];
	FireSystem* firesystem;			// 0x38
	td_vector<void*> projectiles;
	Environment* environment;		// 0x50
	uint8_t padding2[8];
	Transform spawnpoint;			// 0x60
	uint8_t padding3[4];
	Light* flashlight;				// 0x80
	Script* explosion_lua;			// 0x88
	Script* achievements_lua;		// 0x90
	uint8_t padding4[0x48];
	Vec3 sv_size;					// 0xE0
	uint8_t padding5[0x5C];
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
	td_vector<Script*> scripts;		// 0x1D8	<- This is wrong
	td_vector<Animator*> animators;	// 0x1F8
	td_vector<Entity*> top_level;	// 0x208
	uint8_t padding6[0x378];
	td_vector<Vec2> boundary;		// 0x590
	uint8_t padding7[0x390];
	td_vector<Entity*> entities;	// 0x???
	uint8_t padding8[0x28];
	bool has_snow;
	uint8_t padding9[0x23];
	int assets;
}; // 0x9A8 ?

static_assert(offsetof(Scene, animators) == 0x1F8, "Wrong offset scene->animators");
static_assert(offsetof(Scene, boundary) == 0x590, "Wrong offset scene->boundary");

struct ExternalScript {
	uint8_t padding[0x38C];
	int privilege; // 0x38C
};

struct ModData {
	uint8_t padding[0x158];
	td_vector<ExternalScript*> external_scripts; // 0x158
};

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

enum EditorEntityType : int {
	Editor_Water = 17,
	Editor_Voxagon = 19,
	Editor_Boundary = 21,
	Editor_Trigger = 25,
};

struct EditorEntity {
	uint8_t padding[0x174];
	int type;					// 0x174
	td_vector<Vec2> vertices;	// 0x178
};

struct Editor {
	uint8_t padding[0x28];
	EditorEntity* selected;		// 0x28
};

struct Game {
	int screen_width;
	int screen_height;
	GameState state;
	uint8_t padding1[0x44];
	Scene* scene;					// 0x50
	uint8_t padding2[0x10];
	Editor* editor;					// 0x68
	uint8_t padding3[0x48];
	Player* player;					// 0xB8
	uint8_t padding4[0x8];
	td_vector<Palette>* palettes;	// 0xC8
	uint8_t padding5[8];
	ModData* mod_data;				// 0xD8
	uint8_t padding6[0xCC];
	float time_scale;				// 0x1AC
}; // 0x6A8

static_assert(offsetof(Game, scene) == 0x50, "Wrong offset game->scene");

struct ScreenCapture {
	uint8_t padding1[8];
	int width;
	int height;
	uint8_t* image_buffer;		// 0x10
	int frame;
	uint8_t padding2[4];
	td_string capture_dir;		// 0x20
}; // 0xF8

static_assert(offsetof(ScreenCapture, capture_dir) == 0x20, "Wrong offset sc->capture_dir");

#endif
