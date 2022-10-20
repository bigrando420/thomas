#pragma once

// thx to Big Al for the inspiration - https://www.youtube.com/watch?v=6_AvIAlKhG8&list=PLT6InxK-XQvNKTyLXk6H6KKy12UYS_KDL&index=2

#define SET_HELL_LOOSE() (*(int*)0 = 0)
#define ENABLE_ASSERT
#ifdef ENABLE_ASSERT
#undef assert
// TODO - inject this into the libraries above so I can override
#define assert(condition) if (!(condition)) { SET_HELL_LOOSE(); }
#else
#define assert(condition)
#endif

#define STRINGIFY(str) #str
#define GLUE(a, b) a##b

#define ARRAY_COUNT(a) (sizeof(a) / sizeof(*(a)))
#define DEFER_LOOP(start, end) for (int _i_ = ((start), 0); _i_ == 0; _i_++, (end))
// ^ trick from the goat ryan fleury. Useful for wrapping open/close pairs into a scope

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define CLAMP_UPPER(a, b) MIN(a, b)
#define CLAMP_LOWER(a, b) MAX(a, b)
#define CLAMP(minimum, x, maximum)  

#define SQUARE(a) ((a) * (a))

#define global static
#define local static
#define function static

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
typedef int8_t bool64; // or 64 bits for packing a bunch of flags. There is no in-between.

// I'm thinking we pull a Godot and just have a project-wide define on the precision. I'll keep it 32-bit for now.
typedef float real;

// todo - min/max ints, pi, infinity, and other constants https://youtu.be/Wggh4K6wdgA?t=284

// todo - converter functions helpers for handmade math (separate file called thomas_handmade.h which'll be the glue)

typedef struct vec2_int32 vec2_int32;
typedef struct vec4_uint8 vec4_uint8;

typedef struct vec2_real vec2_real;
typedef struct vec3_real vec3_real;
typedef struct vec4_real vec4_real;

typedef struct range1_real range1_real;
typedef struct range2_real range2_real;

typedef struct mat4 mat4;

// default types
typedef vec2_int32 vec2i;
typedef vec2_real vec2;
typedef vec3_real vec3;
typedef vec4_real vec4;
typedef range1_real range1;
typedef range2_real range2;

// todo - pull function defs up top so I can see wassup

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

struct vec2_real
{
	real x;
	real y;
};

struct vec3_real
{
	real x;
	real y;
	real z;
};

struct vec4_real
{
	// todo - union for rgba and elements array
	real x;
	real y;
	real z;
	real w;
};

struct range1_real
{
	real min;
	real max;
};

struct range2_real
{
	vec2_real min;
	vec2_real max;
};

struct mat4
{
	float elements[4][4];
};

function vec2 vec2_multiply_real(const vec2& vec, const real& scale)
{
	vec2 result = vec;
	result.x *= scale;
	result.y *= scale;
	return result;
}
function vec2 vec2_multiply_vec2(const vec2& vec_a, const vec2& vec_b)
{
	vec2 result = vec_a;
	result.x *= vec_b.x;
	result.y *= vec_b.y;
	return result;
}
function vec2 operator*(const real& scale, const vec2& vec)
{
	return vec2_multiply_real(vec, scale);
}
function vec2 operator*(const vec2& vec, const real& scale)
{
	return vec2_multiply_real(vec, scale);
}
function vec2 operator*(const vec2& vec_a, const vec2& vec_b)
{
	return vec2_multiply_vec2(vec_a, vec_b);
}

function vec2 vec2_add_vec2(const vec2& vec_a, const vec2& vec_b)
{
	vec2 result = vec_a;
	result.x += vec_b.x;
	result.y += vec_b.y;
	return result;
}
function vec2 operator+(const vec2& vec_a, const vec2& vec_b)
{
	return vec2_add_vec2(vec_a, vec_b);
}

function vec2 vec2_subtract_vec2(const vec2& vec_a, const vec2& vec_b)
{
	vec2 result = vec_a;
	result.x -= vec_b.x;
	result.y -= vec_b.y;
	return result;
}
function vec2 operator-(const vec2& vec_a, const vec2& vec_b)
{
	return vec2_subtract_vec2(vec_a, vec_b);
}

function vec2& operator+=(vec2& self, const vec2& other)
{
	self.x += other.x;
	self.y += other.y;
	return self;
}


function range2 range2_shift(const range2& rng, const vec2& v)
{
	range2 result = rng;
	result.min += v;
	result.max += v;
	return result;
}

function vec2 range2_size(const range2& rng)
{
	assert(rng.max.x >= rng.min.x && rng.max.y >= rng.min.y);
	vec2 result;
	result.x = rng.max.x - rng.min.x;
	result.y = rng.max.y - rng.min.y;
	return result;
}