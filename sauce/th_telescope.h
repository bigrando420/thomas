// Telescope is now the base layer. Thomas the Game Engine is built on top.
// For now, I'm just using the base types and math
#include "third_party/telescope_light.h"

// additions
function B8 Overlap2F32(Rng2F32 a, Rng2F32 b)
{
	Rng2F32 inter = Intersection2F32(a, b);
	return (inter.min.x < inter.max.x && inter.min.y < inter.max.y);
}

function Rng2F32 Flip2F32(Rng2F32 r)
{
	Rng2F32 result = r;
	result.min.x *= -1.0f;
	result.max.x *= -1.0f;
	F32 temp = result.min.x;
	result.min.x = result.max.x;
	result.max.x = temp;
	return result;
}

function Vec4F32 RGBA8ToV4F32(const U8 r, const U8 g, const U8 b, const U8 a)
{
  Vec4F32 result = { (F32)r / 255.f, (F32)g / 255.f, (F32)b / 255.f, (F32)a / 255.f };
  return result;
}

function Vec4F32 ShiftV4HSV(const Vec4F32 col, const Vec3F32 hsv)
{
  Vec3F32 result = V3F32(col.r, col.g, col.b);
  result = HSVFromRGB(result);
  result.x += hsv.x;
  result.x = Clamp(0.f, result.x, 1.f);
  result.y += hsv.y;
  result.y = Clamp(0.f, result.y, 1.f);
  result.z += hsv.z;
  result.z = Clamp(0.f, result.z, 1.f);
  result = RGBFromHSV(result);
  return V4F32(result.r, result.g, result.b, col.a);
}

function Vec4F32 MulV4HSV(const Vec4F32 col, const Vec3F32 hsv)
{
  Vec3F32 result = V3F32(col.r, col.g, col.b);
  result = HSVFromRGB(result);
  // TODO - wrap functionality for the Hue?
  result.x *= hsv.x;
  result.y *= hsv.y;
  result.z *= hsv.z;
  result = RGBFromHSV(result);
  return V4F32(result.r, result.g, result.b, col.a);
}





// MATH OPERATOR OVERLOADS
#ifdef CPP
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
#endif