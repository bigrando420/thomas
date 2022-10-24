// Telescope is now the base layer. Thomas the Game Engine is built on top.
// For now, I'm just using the base types and math
#include "third_party/telescope_light.h"

// MATH OPERATOR OVERLOADS
static Vec2 operator*(F32 scale, Vec2 vec)
{
	return Scale2F32(vec, scale);
}

static Vec2 operator*(Vec2 vec, F32 scale)
{
	return Scale2F32(vec, scale);
}

static Vec2 operator*(Vec2 vec_a, Vec2 vec_b)
{
	return Mul2F32(vec_a, vec_b);
}

static Vec2 operator+(Vec2 vec_a, Vec2 vec_b)
{
	return Add2F32(vec_a, vec_b);
}

static Vec2 operator-(Vec2 vec_a, Vec2 vec_b)
{
	return Sub2F32(vec_a, vec_b);
}

static Vec2& operator+=(Vec2& self, Vec2 other)
{
	self.x += other.x;
	self.y += other.y;
	return self;
}