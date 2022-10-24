#ifndef BASE_MATH_H
#define BASE_MATH_H

#define Mod(x, y) fmodf(x, y)

#define DegFromRad(v) ((180.f/PiF32) * (v))
#define RadFromDeg(v) ((PiF32/180.f) * (v))

#define AbsoluteValueS64(x) abs(x)
#define AbsoluteValueU64(x) (U64)abs(x)
#define SquareRoot(x)       sqrtf(x)
#define Sin(v) sinf(RadFromDeg(v))
#define Cos(v) cosf(RadFromDeg(v))
#define Tan(v) tanf(RadFromDeg(v))
#define Sin2(v) Pow(Sin(v), 2)
#define Cos2(v) Pow(Cos(v), 2)
#define Pow(b, exp) powf(b, exp)
#define Log10(v) log10(v)

////////////////////////////////
//~ rjf: Constants

read_only global F32 PiF32 = 3.1415926535897f;
read_only global F32 OneOverSquareRootOfTwoPiF32 = 0.3989422804f;
read_only global F32 EulersNumberF32 = 2.7182818284590452353602874713527f;

////////////////////////////////
//~ rjf: Vector Types

//- rjf: 2-vectors

typedef union Vec2F32 Vec2F32;
union Vec2F32
{
 struct
 {
  F32 x;
  F32 y;
 };
 F32 elements[2];
 F32 v[2];
};

typedef union Vec2S32 Vec2S32;
union Vec2S32
{
 struct
 {
  S32 x;
  S32 y;
 };
 S32 elements[2];
 S32 v[2];
};

typedef union Vec2S64 Vec2S64;
union Vec2S64
{
 struct
 {
  S64 x;
  S64 y;
 };
 S64 elements[2];
 S64 v[2];
};

//- rjf: 3-vectors

typedef union Vec3F32 Vec3F32;
union Vec3F32
{
 struct
 {
  F32 x;
  F32 y;
  F32 z;
 };
 
 struct
 {
  F32 r;
  F32 g;
  F32 b;
 };
 
 struct
 {
  F32 red;
  F32 green;
  F32 blue;
 };
 
 struct
 {
  Vec2F32 xy;
  F32 _z1;
 };
 
 struct
 {
  F32 _x1;
  Vec2F32 yz;
 };
 
 F32 elements[3];
 F32 v[3];
};

typedef union Vec3S32 Vec3S32;
union Vec3S32
{
 struct
 {
  S32 x;
  S32 y;
  S32 z;
 };
 
 struct
 {
  S32 r;
  S32 g;
  S32 b;
 };
 
 S32 elements[3];
 S32 v[3];
};

typedef union Vec3S64 Vec3S64;
union Vec3S64
{
 struct
 {
  S64 x;
  S64 y;
  S64 z;
 };
 
 struct
 {
  S64 r;
  S64 g;
  S64 b;
 };
 
 S64 elements[3];
 S64 v[3];
};

//- rjf: 4-vectors

typedef union Vec4F32 Vec4F32;
union Vec4F32
{
 struct
 {
  F32 x;
  F32 y;
  F32 z;
  F32 w;
 };
 
 struct
 {
  Vec2F32 xy;
  Vec2F32 zw;
 };
 
 struct
 {
  Vec3F32 xyz;
  F32 _w1;
 };
 
 struct
 {
  F32 _x1;
  Vec3F32 yzw;
 };
 
 struct
 {
  Vec3F32 rgb;
  F32 _w2;
 };
 
 struct
 {
  Vec3F32 gba;
  F32 _w3;
 };
 
 struct
 {
  F32 r;
  F32 g;
  F32 b;
  F32 a;
 };
 
 struct
 {
  F32 red;
  F32 green;
  F32 blue;
  F32 alpha;
 };
 
 struct
 {
  F32 left;
  F32 up;
  F32 right;
  F32 down;
 };
 
 F32 elements[4];
 F32 v[4];
 struct
 {
  F32 padding_[2];
  F32 dim[2];
 };
};

typedef union Vec4S32 Vec4S32;
union Vec4S32
{
 struct
 {
  S32 x;
  S32 y;
  S32 z;
  S32 w;
 };
 
 struct
 {
  Vec2S32 xy;
  Vec2S32 zw;
 };
 
 struct
 {
  Vec3S32 xyz;
  S32 _w1;
 };
 
 struct
 {
  S32 _x1;
  Vec3S32 yzw;
 };
 
 struct
 {
  Vec3S32 rgb;
  S32 _w2;
 };
 
 struct
 {
  Vec3S32 gba;
  S32 _w3;
 };
 
 struct
 {
  S32 r;
  S32 g;
  S32 b;
  S32 a;
 };
 
 struct
 {
  S32 red;
  S32 green;
  S32 blue;
  S32 alpha;
 };
 
 struct
 {
  S32 left;
  S32 up;
  S32 right;
  S32 down;
 };
 
 struct
 {
  S32 padding_[2];
  S32 dim[2];
 };
 
 S32 elements[4];
 S32 v[4];
};

typedef union Vec4S64 Vec4S64;
union Vec4S64
{
 struct
 {
  S64 x;
  S64 y;
  S64 z;
  S64 w;
 };
 
 struct
 {
  Vec2S64 xy;
  Vec2S64 zw;
 };
 
 struct
 {
  Vec3S64 xyz;
  S64 _w1;
 };
 
 struct
 {
  S64 _x1;
  Vec3S64 yzw;
 };
 
 struct
 {
  Vec3S64 rgb;
  S64 _w2;
 };
 
 struct
 {
  Vec3S64 gba;
  S64 _w3;
 };
 
 struct
 {
  S64 r;
  S64 g;
  S64 b;
  S64 a;
 };
 
 struct
 {
  S64 red;
  S64 green;
  S64 blue;
  S64 alpha;
 };
 
 struct
 {
  S64 left;
  S64 up;
  S64 right;
  S64 down;
 };
 
 struct
 {
  S64 padding_[2];
  S64 dim[2];
 };
 
 S64 elements[4];
 S64 v[4];
};

#define Vec2F32FromVec(v) V2F32((F32)(v).x, (F32)(v).y)
#define Vec2S32FromVec(v) V2S32((S32)(v).x, (S32)(v).y)
#define Vec2S64FromVec(v) V2S64((S64)(v).x, (S64)(v).y)
#define Vec3F32FromVec(v) V3F32((F32)(v).x, (F32)(v).y, (F32)(v).z)
#define Vec3S32FromVec(v) V3S32((S32)(v).x, (S32)(v).y, (S32)(v).z)
#define Vec3S64FromVec(v) V3S64((S64)(v).x, (S64)(v).y, (S64)(v).z)
#define Vec4F32FromVec(v) V4F32((F32)(v).x, (F32)(v).y, (F32)(v).z, (F32)(v).w)
#define Vec4S32FromVec(v) V4S32((S32)(v).x, (S32)(v).y, (S32)(v).z, (S32)(v).w)
#define Vec4S64FromVec(v) V4S64((S64)(v).x, (S64)(v).y, (S64)(v).z, (S64)(v).w)

// NOTE(rjf): Default Vector Types/Constructors (F32)
typedef Vec2F32 Vec2;
typedef Vec3F32 Vec3;
typedef Vec4F32 Vec4;
#define V2(...) V2F32(__VA_ARGS__)
#define V3(...) V3F32(__VA_ARGS__)
#define V4(...) V4F32(__VA_ARGS__)
#define Vec2FromVec(...) Vec2F32FromVec(__VA_ARGS__)
#define Vec3FromVec(...) Vec3F32FromVec(__VA_ARGS__)
#define Vec4FromVec(...) Vec4F32FromVec(__VA_ARGS__)

////////////////////////////////
//~ rjf: Matrix Types

typedef struct Matrix3x3F32 Matrix3x3F32;
struct Matrix3x3F32
{
 F32 elements[3][3];
};

typedef struct Matrix4x4F32 Matrix4x4F32;
struct Matrix4x4F32
{
 F32 elements[4][4];
};

// NOTE(rjf): Default Matrix Types/Constructors (F32)
// TODO(rjf): Enable these (maybe)
// typedef Matrix3x3F32 Matrix3x3;
// typedef Matrix4x4F32 Matrix4x4;

////////////////////////////////
//~ rjf: Interval Types

typedef struct Rng1F32 Rng1F32;
struct Rng1F32
{
 F32 min;
 F32 max;
};

typedef struct Rng1U64 Rng1U64;
struct Rng1U64
{
 U64 min;
 U64 max;
};

typedef struct Rng1S64 Rng1S64;
struct Rng1S64
{
 S64 min;
 S64 max;
};

typedef union Rng2F32 Rng2F32;
union Rng2F32
{
 struct
 {
  Vec2F32 min;
  Vec2F32 max;
 };
 struct
 {
  Vec2F32 p0;
  Vec2F32 p1;
 };
 struct
 {
  F32 x0;
  F32 y0;
  F32 x1;
  F32 y1;
 };
};

typedef union Rng2S64 Rng2S64;
union Rng2S64
{
 struct
 {
  Vec2S64 min;
  Vec2S64 max;
 };
 struct
 {
  Vec2S64 p0;
  Vec2S64 p1;
 };
 struct
 {
  S64 x0;
  S64 y0;
  S64 x1;
  S64 y1;
 };
};

typedef struct Rng1U64Node Rng1U64Node;
struct Rng1U64Node
{
 Rng1U64Node *next;
 Rng1U64 v;
};

typedef struct Rng1U64List Rng1U64List;
struct Rng1U64List
{
 Rng1U64Node *first;
 Rng1U64Node *last;
 U64 count;
 U64 total_count;
};

////////////////////////////////
//~ rjf: Vector Ops

core_function Vec2F32 V2F32(F32 x, F32 y);
core_function Vec2F32 Add2F32(Vec2F32 a, Vec2F32 b);
core_function Vec2F32 Sub2F32(Vec2F32 a, Vec2F32 b);
core_function Vec2F32 Mul2F32(Vec2F32 a, Vec2F32 b);
core_function Vec2F32 Div2F32(Vec2F32 a, Vec2F32 b);
core_function Vec2F32 Scale2F32(Vec2F32 a, F32 scale);
core_function F32 Dot2F32(Vec2F32 a, Vec2F32 b);
core_function F32 LengthSquared2F32(Vec2F32 v);
core_function F32 Length2F32(Vec2F32 v);
core_function Vec2F32 Normalize2F32(Vec2F32 v);
core_function Vec2F32 Mix2F32(Vec2F32 a, Vec2F32 b, F32 t);

core_function Vec2S32 V2S32(S32 x, S32 y);

core_function Vec3F32 V3F32(F32 x, F32 y, F32 z);
core_function Vec3F32 Add3F32(Vec3F32 a, Vec3F32 b);
core_function Vec3F32 Sub3F32(Vec3F32 a, Vec3F32 b);
core_function Vec3F32 Mul3F32(Vec3F32 a, Vec3F32 b);
core_function Vec3F32 Div3F32(Vec3F32 a, Vec3F32 b);
core_function Vec3F32 Scale3F32(Vec3F32 a, F32 scale);
core_function F32 Dot3F32(Vec3F32 a, Vec3F32 b);
core_function F32 LengthSquared3F32(Vec3F32 v);
core_function F32 Length3F32(Vec3F32 v);
core_function Vec3F32 Normalize3F32(Vec3F32 v);
core_function Vec3F32 Mix3F32(Vec3F32 a, Vec3F32 b, F32 t);
core_function Vec3F32 Cross3F32(Vec3F32 a, Vec3F32 b);
core_function Vec3F32 Transform3F32(Vec3F32 v, Matrix3x3F32 m);

core_function Vec4F32 V4F32(F32 x, F32 y, F32 z, F32 w);
core_function Vec4F32 Add4F32(Vec4F32 a, Vec4F32 b);
core_function Vec4F32 Sub4F32(Vec4F32 a, Vec4F32 b);
core_function Vec4F32 Mul4F32(Vec4F32 a, Vec4F32 b);
core_function Vec4F32 Div4F32(Vec4F32 a, Vec4F32 b);
core_function Vec4F32 Scale4F32(Vec4F32 a, F32 scale);
core_function F32 Dot4F32(Vec4F32 a, Vec4F32 b);
core_function F32 LengthSquared4F32(Vec4F32 v);
core_function F32 Length4F32(Vec4F32 v);
core_function Vec4F32 Normalize4F32(Vec4F32 v);
core_function Vec4F32 Mix4F32(Vec4F32 a, Vec4F32 b, F32 t);
core_function Vec4F32 Transform4F32(Vec4F32 v, Matrix4x4F32 m);

core_function Vec2S64 V2S64(S64 x, S64 y);
core_function Vec2S64 Add2S64(Vec2S64 a, Vec2S64 b);
core_function Vec2S64 Sub2S64(Vec2S64 a, Vec2S64 b);

//~ rjf: Range Ops

core_function Rng1F32 R1F32(F32 min, F32 max);
core_function Rng1F32 Shift1F32(Rng1F32 r, F32 v);
core_function Rng1F32 Pad1F32(Rng1F32 r, F32 x);
core_function F32 Center1F32(Rng1F32 r);
core_function B32 Contains1F32(Rng1F32 r, F32 v);
core_function F32 Dim1F32(Rng1F32 r);
core_function Rng1F32 Union1F32(Rng1F32 a, Rng1F32 b);
core_function Rng1F32 Intersection1F32(Rng1F32 a, Rng1F32 b);

core_function Rng1S64 R1S64(S64 min, S64 max);
core_function Rng1S64 Shift1S64(Rng1S64 r, S64 v);
core_function Rng1S64 Pad1S64(Rng1S64 r, S64 x);
core_function S64 Center1S64(Rng1S64 r);
core_function B32 Contains1S64(Rng1S64 r, S64 v);
core_function S64 Dim1S64(Rng1S64 r);
core_function Rng1S64 Union1S64(Rng1S64 a, Rng1S64 b);
core_function Rng1S64 Intersection1S64(Rng1S64 a, Rng1S64 b);

core_function Rng1U64 R1U64(U64 min, U64 max);
core_function Rng1U64 Shift1U64(Rng1U64 r, U64 v);
core_function Rng1U64 Pad1U64(Rng1U64 r, U64 x);
core_function U64 Center1U64(Rng1U64 r);
core_function B32 Contains1U64(Rng1U64 r, U64 v);
core_function U64 Dim1U64(Rng1U64 r);
core_function Rng1U64 Union1U64(Rng1U64 a, Rng1U64 b);
core_function Rng1U64 Intersection1U64(Rng1U64 a, Rng1U64 b);

core_function Rng2F32 R2F32(Vec2F32 min, Vec2F32 max);
core_function Rng2F32 Shift2F32(Rng2F32 r, Vec2F32 v);
core_function Rng2F32 Pad2F32(Rng2F32 r, F32 x);
core_function Vec2F32 Center2F32(Rng2F32 r);
core_function B32 Contains2F32(Rng2F32 r, Vec2F32 v);
core_function Vec2F32 Dim2F32(Rng2F32 r);
core_function Rng2F32 Union2F32(Rng2F32 a, Rng2F32 b);
core_function Rng2F32 Intersection2F32(Rng2F32 a, Rng2F32 b);

core_function Rng2S64 R2S64(Vec2S64 min, Vec2S64 max);
core_function Rng2S64 Shift2S64(Rng2F32 r, Vec2S64 v);
core_function Rng2S64 Pad2S64(Rng2S64 r, S64 x);
core_function Vec2S64 Center2S64(Rng2S64 r);
core_function B32 Contains2S64(Rng2S64 r, Vec2S64 v);
core_function Vec2S64 Dim2S64(Rng2S64 r);
core_function Rng2S64 Union2S64(Rng2S64 a, Rng2S64 b);
core_function Rng2S64 Intersection2S64(Rng2S64 a, Rng2S64 b);

////////////////////////////////
//~ rjf: Matrix Constructors

core_function Matrix3x3F32 MakeMatrix3x3F32(F32 d);
core_function Matrix3x3F32 MakeTranslate3x3F32(Vec2F32 translation);
core_function Matrix3x3F32 MakeScale3x3F32(Vec2F32 scale);
core_function Matrix3x3F32 MakeRotate3x3(F32 angle);

core_function Matrix4x4F32 MakeMatrix4x4F32(F32 d);
core_function Matrix4x4F32 MakeTranslate4x4F32(Vec3F32 translation);
core_function Matrix4x4F32 MakeScale4x4F32(Vec3F32 scale);
core_function Matrix4x4F32 MakePerspective4x4F32(F32 fov, F32 aspect_ratio, F32 near_z, F32 far_z);
core_function Matrix4x4F32 MakeOrthographic4x4F32(F32 left, F32 right, F32 bottom, F32 top, F32 near_z, F32 far_z);
core_function Matrix4x4F32 MakeLookAt4x4F32(Vec3F32 eye, Vec3F32 center, Vec3F32 up);

////////////////////////////////
//~ rjf: Matrix Ops

core_function Matrix3x3F32 Mul3x3F32(Matrix3x3F32 a, Matrix3x3F32 b);
core_function Matrix3x3F32 Scale3x3F32(Matrix3x3F32 m, F32 scale);
core_function Matrix4x4F32 Scale4x4F32(Matrix4x4F32 m, F32 scale);
core_function Matrix4x4F32 Mul4x4F32(Matrix4x4F32 a, Matrix4x4F32 b);
core_function Matrix4x4F32 Inverse4x4F32(Matrix4x4F32 m);
core_function Matrix4x4F32 RemoveRotation4x4F32(Matrix4x4F32 mat);

////////////////////////////////
//~ rjf: Lists



////////////////////////////////
//~ rjf: Miscellaneous Ops

core_function Vec3F32 HSVFromRGB(Vec3F32 rgb);
core_function Vec3F32 RGBFromHSV(Vec3F32 hsv);
core_function Vec4F32 Vec4F32FromHexRGBA(U32 hex);
core_function F32 MillisecondsFromMicroseconds(U64 microseconds);
core_function U64 MicrosecondsFromMilliseconds(F32 milliseconds);
core_function Vec2S64 SideVertexFromCorner(Corner corner);

#endif // BASE_MATH_H
