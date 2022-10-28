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


static B8 float_equals(const F32& a, const F32& b, const F32& epsilon = 0.00001f) {
	return (fabsf(a - b) < epsilon);
}

static bool float_is_zero(const F32& a) {
	return float_equals(a, 0.0f);
}

// @robust - test this properly with a slider visual to see what happens when it goes out of bounds
static F32 float_lerp(const F32& x, const F32& a, const F32& b) {
	return a * (1.f - x) + (b * x);
}

static F32 float_map(const F32& x, const F32& x_min, const F32& x_max, const F32& map_min, const F32& map_max) {
	return ((x - x_min) / (x_max - x_min) * (map_max - map_min) + map_min);
}

// Progress of x in the range of begin -> end. Guarenteed to be 0 -> 1
// is this just a normalised lerp?
static F32 float_alpha(const F32& x, const F32& begin, const F32& end) {
	if (float_equals(begin, end))
		return begin;
	F32 delta = end - begin;
	F32 result = (x - begin) / delta;
	Assert(result >= 0.0f && result <= 1.f);
	return result;
}

// https://easings.net/
enum TH_Ease {
	TH_EASE_linear,
	TH_EASE_cubic_in_out,
};

static F32 float_alpha_cubic_in_out(const F32& alpha)
{
	return (alpha < 0.5f ? 4.f * alpha * alpha * alpha : 1.f - powf(-2.f * alpha + 2.f, 3.f) / 2.f);
}

static F32 float_alpha_ease(const F32& alpha, const TH_Ease type) {
	switch (type) {
	case TH_EASE_linear:
		return alpha;
	case TH_EASE_cubic_in_out:
		return float_alpha_cubic_in_out(alpha);
	default:
		return 0;
	}
}

static F32 float_random_alpha() {
	return (F32)rand() / (F32)RAND_MAX;
}

static F32 float_random_range(const F32& min, const F32& max) {
	F32 result = float_random_alpha();
	result = float_lerp(result, min, max);
	return result;
}

static F32 float_alpha_sin_mid(const F32& alpha)
{
	F32 sin = alpha;
	sin = (sinf((alpha - .25f) * 2.f * PiF32) / 2.f) + 0.5f;
	return sin;
}

static Rng2F32 range2_center_bottom(Rng2F32 range) {
	Rng2F32 result = range;
	Vec2 size = Dim2F32(range);
	result.min.x -= size.x * 0.5f;
	result.max.x -= size.x * 0.5f;
	return result;
}

static Rng2F32 range2_center_middle(Rng2F32 range) {
	Rng2F32 result = range;
	Vec2 size = Dim2F32(range);
	result.min.x -= size.x * 0.5f;
	result.min.y -= size.y * 0.5f;
	result.max.x -= size.x * 0.5f;
	result.max.y -= size.y * 0.5f;
	return result;
}

static Rng2F32 range2_center_left(Rng2F32 range) {
	Rng2F32 result = range;
	Vec2 size = Dim2F32(range);
	result.min.y -= size.y * 0.5f;
	result.max.y -= size.y * 0.5f;
	return result;
}

static Rng2F32 range2_scale(Rng2F32 range, F32 scale) {
	Rng2F32 result = range;
	result.min = result.min * scale;
	result.max = result.max * scale;
	return result;
}

static Rng2F32 range2_remove_offset(Rng2F32 range) {
	Rng2F32 result = {0};
	Vec2 size = Dim2F32(range);
	result.max = size;
	return result;
}

#endif