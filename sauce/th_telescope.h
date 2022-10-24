// Telescope is now the base layer. Thomas the Game Engine is built on top.
// For now, I'm just using the base types and math
#include "third_party/telescope_light.h"

// additions
function B8 Overlap2F32(Rng2F32 a, Rng2F32 b)
{
	Rng2F32 inter = Intersection2F32(a, b);
	return (inter.min.x < inter.max.x && inter.min.y < inter.max.y);
}

// MATH OPERATOR OVERLOADS
function Vec2 operator*(F32 scale, Vec2 vec)
{
	return Scale2F32(vec, scale);
}

function Vec2 operator*(Vec2 vec, F32 scale)
{
	return Scale2F32(vec, scale);
}

function Vec2 operator*(Vec2 vec_a, Vec2 vec_b)
{
	return Mul2F32(vec_a, vec_b);
}

function Vec2 operator+(Vec2 vec_a, Vec2 vec_b)
{
	return Add2F32(vec_a, vec_b);
}

function Vec2 operator-(Vec2 vec_a, Vec2 vec_b)
{
	return Sub2F32(vec_a, vec_b);
}

function Vec2& operator+=(Vec2& self, Vec2 other)
{
	self.x += other.x;
	self.y += other.y;
	return self;
}