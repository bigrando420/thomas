#ifndef TH_BASE_H
#define TH_BASE_H

// TODO - format

#define PRINT_STRING(_str) OutputDebugString(_str)
// @string - if you overrun this with a long string, you will die.
#define LOG(_str, ...) {char output[256] = { 0 }; sprintf(output, _str"\n", __VA_ARGS__); PRINT_STRING(output);}

#define ForEachNode(name, first_node, type) for (type name = first_node; name != 0; name = name->next)

#define ForEachFlat(name, flat_array, type) for (type name = flat_array; (name - flat_array) < ArrayCount(flat_array); name += 1)
#define TH_ARRAY_PUSH(flat_array, count) &flat_array[count++]; Assert((count) + 1 < ArrayCount(flat_array))

#define SIGN(x) (((x) > 0) - ((x) < 0))
#define SQUARE(a) ((a) * (a))
#define V4_EXPAND(vec) vec.x, vec.y, vec.z, vec.w


function B8 F32EqualsEpsilon(F32 a, F32 b, F32 epsilon)
{
	return (fabsf(a - b) < epsilon);
}

function B8 F32Equals(F32 a, F32 b)
{
	return F32EqualsEpsilon(a, b, 0.00001f);
}

function bool F32IsZero(F32 a)
{
	return F32Equals(a, 0.0f);
}

function F32 FractF32(F32 x)
{
	return (x - floorf(x));
}

#define AnimateToTarget(value, target, rate) value += ((target) - value) * (1 - Pow(2.f, -(rate) * APP_DT()))

// @robust - test this properly with a slider visual to see what happens when it goes out of bounds
function F32 LerpF32(F32 x, F32 a, F32 b)
{
	return a * (1.f - x) + (b * x);
}

function F32 MapF32(F32 x, F32 x_min, F32 x_max, F32 map_min, F32 map_max)
{
	return ((x - x_min) / (x_max - x_min) * (map_max - map_min) + map_min);
}

// Progress of x in the range of begin -> end. Guarenteed to be 0 -> 1
// is this just a normalised lerp?
function F32 AlphaF32(F32 x, F32 begin, F32 end)
{
	if (F32Equals(begin, end))
		return begin;
	F32 delta = end - begin;
	F32 result = (x - begin) / delta;
	Assert(result >= 0.0f && result <= 1.f);
	return result;
}

// https://easings.net/
typedef enum Ease Ease;
enum Ease
{
	TH_EASE_linear,
	TH_EASE_cubic_in_out,
};

function F32 float_alpha_cubic_in_out(F32 alpha)
{
	return (alpha < 0.5f ? 4.f * alpha * alpha * alpha : 1.f - powf(-2.f * alpha + 2.f, 3.f) / 2.f);
}

function F32 float_alpha_ease(F32 alpha, Ease type)
{
	switch (type) {
	case TH_EASE_linear:
		return alpha;
	case TH_EASE_cubic_in_out:
		return float_alpha_cubic_in_out(alpha);
	default:
		return 0;
	}
}

function F32 float_random_alpha()
{
	return (F32)rand() / (F32)RAND_MAX;
}

function F32 float_random_range(F32 min, F32 max)
{
	F32 result = float_random_alpha();
	result = LerpF32(result, min, max);
	return result;
}

function F32 float_alpha_sin_mid(F32 alpha)
{
	F32 sin = alpha;
	sin = (sinf((alpha - .25f) * 2.f * PiF32) / 2.f) + 0.5f;
	return sin;
}

function Rng2F32 range2_center_bottom(Rng2F32 range) {
	Rng2F32 result = range;
	Vec2 size = Dim2F32(range);
	result.min.x -= size.x * 0.5f;
	result.max.x -= size.x * 0.5f;
	return result;
}

function Rng2F32 range2_center_middle(Rng2F32 range) {
	Rng2F32 result = range;
	Vec2 size = Dim2F32(range);
	result.min.x -= size.x * 0.5f;
	result.min.y -= size.y * 0.5f;
	result.max.x -= size.x * 0.5f;
	result.max.y -= size.y * 0.5f;
	return result;
}

function Rng2F32 range2_center_left(Rng2F32 range) {
	Rng2F32 result = range;
	Vec2 size = Dim2F32(range);
	result.min.y -= size.y * 0.5f;
	result.max.y -= size.y * 0.5f;
	return result;
}

function Rng2F32 range2_scale(Rng2F32 range, F32 scale) {
	Rng2F32 result = range;
	result.min = Scale2F32(result.min, scale);
	result.max = Scale2F32(result.max, scale);
	return result;
}

function Rng2F32 range2_remove_offset(Rng2F32 range) {
	Rng2F32 result = {0};
	Vec2 size = Dim2F32(range);
	result.max = size;
	return result;
}

#endif