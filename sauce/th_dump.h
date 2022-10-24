#ifndef TH_BASE_H
#define BASE_H

// thx to Big Al for the inspiration - https://www.youtube.com/watch?v=6_AvIAlKhG8&list=PLT6InxK-XQvNKTyLXk6H6KKy12UYS_KDL&index=2

#define SET_HELL_LOOSE() (*(int*)0 = 0)
#define ENABLE_ASSERT
#ifdef ENABLE_ASSERT
// TODO - inject this into the libraries above so I can override
#define Assert(condition) if (!(condition)) { SET_HELL_LOOSE(); }
#else
#define assert(condition)
#endif

#define OutputDebugString(_str) puts(_str)
#define PRINT_STRING(_str) OutputDebugString(_str)
// @string - if you overrun this with a long string, you will die.
#define LOG(_str, ...) {char output[256] = { 0 }; sprintf(output, _str"\n", ##__VA_ARGS__); PRINT_STRING(output);}

#define STRINGIFY(str) #str
#define GLUE(a, b) a##b

#define ArrayCount(a) (sizeof(a) / sizeof(*(a)))
#define DEFER_LOOP(start, end) for (int _i_ = ((start), 0); _i_ == 0; _i_++, (end))
// ^ trick from the goat ryan fleury. Useful for wrapping open/close pairs into a scope

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define CLAMP_UPPER(a, b) MIN(a, b)
#define CLAMP_LOWER(a, b) MAX(a, b)
#define CLAMP(minimum, x, maximum) (((minimum)>(x))?(minimum):((maximum)<(x))?(maximum):(x))
#define SIGN(x) (((x) > 0) - ((x) < 0))

#define SQUARE(a) ((a) * (a))

#define V4_EXPAND(vec) vec.x, vec.y, vec.z, vec.w

#define PI_FLOAT (3.1415926535897f)

#define MEMORY_ZERO_STRUCT(struct_ptr) memset(struct_ptr, 0, sizeof(*(struct_ptr)))
// todo - memory wrappers for crt - https://youtu.be/6_AvIAlKhG8?t=725
// for more helper macros like int / point arithmatic, member offsets, etc - https://youtu.be/6_AvIAlKhG8?t=435

#include "stdint.h"
typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef int8_t bool8; // you're either after a 1 or 0
// or
typedef int8_t bool64; // 64 bits for packing a bunch of flags
// There is no in-between.

// todo - min/max ints, pi, infinity, and other constants https://youtu.be/Wggh4K6wdgA?t=284

// todo - converter statics helpers for handmade math (separate file called thomas_handmade.h which'll be the glue)

typedef struct vec2_int32 vec2_int32;
typedef struct vec4_uint8 vec4_uint8;

typedef struct vec2_float vec2_float;
typedef struct vec3_float vec3_float;
typedef struct vec4_float vec4_float;

typedef struct range1_float range1_float;
typedef struct range2_float range2_float;

typedef struct mat4 mat4;

// default types
typedef vec2_int32 vec2i;
typedef vec2_float vec2;
typedef vec3_float vec3;
typedef vec4_float vec4;
typedef range1_float range1;
typedef range2_float range2;

// todo - move definitions into internal.cpp and create a clean .h API

struct vec2_int32
{
	int32 x;
	int32 y;
};

struct vec4_uint8
{
	uint8 r;
	uint8 g;
	uint8 b;
	uint8 a;
};

struct vec2_float
{
	vec2_float(float _x = 0.0f, float _y = 0.0f) : x(_x), y(_y) {};
	float x;
	float y;
};

struct vec3_float
{
	float x;
	float y;
	float z;
};

struct vec4_float
{
	vec4_float(float _x = 0.0f, float _y = 0.0f, float _z = 0.0f, float _w = 0.0f) :
		x(_x), y(_y), z(_z), w(_w) {};
	union {
		float x;
		float r;
	};
	union {
		float y;
		float g;
	};
	union {
		float z;
		float b;
	};
	union {
		float w;
		float a;
	};
};

struct range1_float
{
	float min;
	float max;
};

struct range2_float
{
	range2_float(vec2 _min, vec2 _max) : min(_min), max(_max) {};
	range2_float(float val = 0.0f) : min(vec2(val, val)), max(vec2(val, val)) {};
	vec2_float min;
	vec2_float max;
};

struct mat4
{
	float elements[4][4];
};

static vec2 vec2_multiply_float(const vec2& vec, const float& scale)
{
	vec2 result = vec;
	result.x *= scale;
	result.y *= scale;
	return result;
}
static vec2 vec2_multiply_vec2(const vec2& vec_a, const vec2& vec_b)
{
	vec2 result = vec_a;
	result.x *= vec_b.x;
	result.y *= vec_b.y;
	return result;
}
static vec2 operator*(const float& scale, const vec2& vec)
{
	return vec2_multiply_float(vec, scale);
}
static vec2 operator*(const vec2& vec, const float& scale)
{
	return vec2_multiply_float(vec, scale);
}
static vec2 operator*(const vec2& vec_a, const vec2& vec_b)
{
	return vec2_multiply_vec2(vec_a, vec_b);
}

static vec2 vec2_add_vec2(const vec2& vec_a, const vec2& vec_b)
{
	vec2 result = vec_a;
	result.x += vec_b.x;
	result.y += vec_b.y;
	return result;
}
static vec2 operator+(const vec2& vec_a, const vec2& vec_b)
{
	return vec2_add_vec2(vec_a, vec_b);
}

static vec2 vec2_subtract_vec2(const vec2& vec_a, const vec2& vec_b)
{
	vec2 result = vec_a;
	result.x -= vec_b.x;
	result.y -= vec_b.y;
	return result;
}
static vec2 operator-(const vec2& vec_a, const vec2& vec_b)
{
	return vec2_subtract_vec2(vec_a, vec_b);
}

static vec2& operator+=(vec2& self, const vec2& other)
{
	self.x += other.x;
	self.y += other.y;
	return self;
}


static range2 range2_shift(const range2& rng, const vec2& v)
{
	range2 result = rng;
	result.min += v;
	result.max += v;
	return result;
}

// guarenteed to be positive, even if the min/max is fucked up and back to front
static vec2 range2_size(const range2& rng)
{
	vec2 result;
	result.x = fabsf(rng.max.x - rng.min.x);
	result.y = fabsf(rng.max.y - rng.min.y);
	return result;
}

static bool8 float_equals(const float& a, const float& b, const float& epsilon = 0.00001f) {
	return (fabsf(a - b) < epsilon);
}

static bool float_is_zero(const float& a) {
	return float_equals(a, 0.0f);
}

// @robust - test this properly with a slider visual to see what happens when it goes out of bounds
static float float_lerp(const float& x, const float& a, const float& b) {
	return a * (1.f - x) + (b * x);
}

static float float_map(const float& x, const float& x_min, const float& x_max, const float& map_min, const float& map_max) {
	return ((x - x_min) / (x_max - x_min) * (map_max - map_min) + map_min);
}

// Progress of x in the range of begin -> end. Guarenteed to be 0 -> 1
// is this just a normalised lerp?
static float float_alpha(const float& x, const float& begin, const float& end) {
	if (float_equals(begin, end))
		return begin;
	float delta = end - begin;
	float result = (x - begin) / delta;
	Assert(result >= 0.0f && result <= 1.f);
	return result;
}

// https://easings.net/
enum TH_Ease {
	TH_EASE_linear,
	TH_EASE_cubic_in_out,
};

static float float_alpha_cubic_in_out(const float& alpha)
{
	return (alpha < 0.5f ? 4.f * alpha * alpha * alpha : 1.f - powf(-2.f * alpha + 2.f, 3.f) / 2.f);
}

static float float_alpha_ease(const float& alpha, const TH_Ease type) {
	switch (type) {
	case TH_EASE_linear:
		return alpha;
	case TH_EASE_cubic_in_out:
		return float_alpha_cubic_in_out(alpha);
	default:
		SET_HELL_LOOSE(); // not implemented
		return 0;
	}
}

static float float_random_alpha() {
	return (float)rand() / (float)RAND_MAX;
}

static float float_random_range(const float& min, const float& max) {
	float result = float_random_alpha();
	result = float_lerp(result, min, max);
	return result;
}

static float float_alpha_sin_mid(const float& alpha)
{
	float sin = alpha;
	sin = (sinf((alpha - .25f) * 2.f * PI_FLOAT) / 2.f) + 0.5f;
	return sin;
}

static range2 range2_center_bottom(const range2& range) {
	range2 result = range;
	vec2 size = range2_size(range);
	result.min.x -= size.x * 0.5f;
	result.max.x -= size.x * 0.5f;
	return result;
}

static range2 range2_center_middle(const range2& range) {
	range2 result = range;
	vec2 size = range2_size(range);
	result.min.x -= size.x * 0.5f;
	result.min.y -= size.y * 0.5f;
	result.max.x -= size.x * 0.5f;
	result.max.y -= size.y * 0.5f;
	return result;
}

static range2 range2_scale(const range2& range, const float& scale) {
	range2 result = range;
	result.min = result.min * scale;
	result.max = result.max * scale;
	return result;
}

#endif
