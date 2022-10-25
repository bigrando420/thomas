#ifndef BASE_TYPES_H
#define BASE_TYPES_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <time.h>

#if COMPILER_CL
#include <intrin.h>
#endif

////////////////////////////////
//~ rjf: Meow Hash

// Unused and has confilcts with linux build
// #include "third_party/meow_hash_x64_aesni.h"

////////////////////////////////
//~ rjf: PCG

#define PCG_HAS_128BIT_OPS 0
#if COMPILER_CL && COMPILER_CL_YEAR < 2015
#define inline
#include "third_party/pcg/include/pcg_variants.h"
#undef inline
#else
#include "third_party/pcg/include/pcg_variants.h"
#endif

////////////////////////////////
//~ rjf: Macros

//- rjf: "keywords"

#define global         static
#define function       static
#define local_persist  static
#define fallthrough

#if CORE_BUILD
#define core_global exported extern
#else
#define core_global imported
#endif

#if CORE_BUILD
#define core_function exported
#else
#define core_function imported
#endif

#if LANG_CPP
#if OS_WINDOWS
#define exported extern "C" __declspec(dllexport)
#else
#define exported extern "C"
#endif
#else
#if OS_WINDOWS
#define exported __declspec(dllexport)
#else
#define exported
#endif
#endif

#if LANG_CPP
#if OS_WINDOWS
#define imported extern "C" __declspec(dllimport)
#else
#define imported extern "C"
#endif
#else
#if OS_WINDOWS
#define imported __declspec(dllimport)
#else
#define imported
#endif
#endif

#if COMPILER_CL
#define per_thread __declspec(thread)
#elif COMPILER_CLANG
#define per_thread __thread
#elif COMPILER_GCC
#define per_thread __thread
#endif

#if COMPILER_CL && COMPILER_CL_YEAR < 2015
# define inline_function static
#else
# define inline_function inline static
#endif

#if COMPILER_CL && COMPILER_CL_YEAR < 2015
# define this_function_name "unknown"
#else
# define this_function_name __func__
#endif

#if COMPILER_CL && COMPILER_CL_YEAR < 2015
# define snprintf _snprintf
#endif

#if OS_WINDOWS
#pragma section(".roglob", read)
#define read_only __declspec(allocate(".roglob"))
#else
// TODO(rjf): figure out if this benefit is possible on non-Windows
#define read_only
#endif

//- rjf: memory copy/set operations

#define MemoryCopy memcpy
#define MemoryMove memmove
#define MemorySet  memset

#define MemoryCopyStruct(dst, src) do { Assert(sizeof(*(dst)) == sizeof(*(src))); MemoryCopy((dst), (src), sizeof(*(dst))); } while(0)
#define MemoryCopyArray(dst, src)  do { Assert(sizeof(dst) == sizeof(src)); MemoryCopy((dst), (src), sizeof(src)); }while(0)

#define MemoryZero(ptr, size) MemorySet((ptr), 0, (size))
#define MemoryZeroStruct(ptr) MemoryZero((ptr), sizeof(*(ptr)))
#define MemoryZeroArray(arr)  MemoryZero((arr), sizeof(arr))

//- rjf: compiler, shut up! helpers

#define UnusedVariable(name) (void)name

//- rjf: integer/pointer/array/type manipulations

#define ArrayCount(a) (sizeof(a) / sizeof((a)[0]))
#define IntFromPtr(p) (U64)(((U8*)p) - 0)
#define PtrFromInt(i) (void*)(((U8*)0) + i)
#define Member(type, member_name) ((type *)0)->member_name
#define OffsetOf(type, member_name) IntFromPtr(&Member(type, member_name))

#define Bytes(n)      (n)
#define Kilobytes(n)  (n << 10)
#define Megabytes(n)  (n << 20)
#define Gigabytes(n)  (((U64)n) << 30)
#define Terabytes(n)  (((U64)n) << 40)

#define Thousand(n) ((n)*1000)
#define Million(n)  ((n)*1000000)
#define Billion(n)  ((n)*1000000000LL)

#define Swap(type, a, b) do{ type _swapper_ = a; a = b; b = _swapper_; }while(0)

//- rjf: linked list helpers

#define CheckNull(p) ((p)==0)
#define SetNull(p) ((p)=0)

#define QueuePush_NZ(f,l,n,next,zchk,zset) (zchk(f)?\
(((f)=(l)=(n)), zset((n)->next)):\
((l)->next=(n),(l)=(n),zset((n)->next)))
#define QueuePushFront_NZ(f,l,n,next,zchk,zset) (zchk(f) ? (((f) = (l) = (n)), zset((n)->next)) :\
((n)->next = (f)), ((f) = (n)))
#define QueuePop_NZ(f,l,next,zset) ((f)==(l)?\
(zset(f),zset(l)):\
((f)=(f)->next))
#define StackPush_N(f,n,next) ((n)->next=(f),(f)=(n))
#define StackPop_NZ(f,next,zchk) (zchk(f)?0:((f)=(f)->next))

#define DLLInsert_NPZ(f,l,p,n,next,prev,zchk,zset) \
(zchk(f) ? (((f) = (l) = (n)), zset((n)->next), zset((n)->prev)) :\
zchk(p) ? (zset((n)->prev), (n)->next = (f), (zchk(f) ? (0) : ((f)->prev = (n))), (f) = (n)) :\
((zchk((p)->next) ? (0) : (((p)->next->prev) = (n))), (n)->next = (p)->next, (n)->prev = (p), (p)->next = (n),\
((p) == (l) ? (l) = (n) : (0))))
#define DLLPushBack_NPZ(f,l,n,next,prev,zchk,zset) DLLInsert_NPZ(f,l,l,n,next,prev,zchk,zset)
#define DLLRemove_NPZ(f,l,n,next,prev,zchk,zset) (((f)==(n))?\
((f)=(f)->next, (zchk(f) ? (zset(l)) : zset((f)->prev))):\
((l)==(n))?\
((l)=(l)->prev, (zchk(l) ? (zset(f)) : zset((l)->next))):\
((zchk((n)->next) ? (0) : ((n)->next->prev=(n)->prev)),\
(zchk((n)->prev) ? (0) : ((n)->prev->next=(n)->next))))


#define QueuePush(f,l,n)         QueuePush_NZ(f,l,n,next,CheckNull,SetNull)
#define QueuePushFront(f,l,n)    QueuePushFront_NZ(f,l,n,next,CheckNull,SetNull)
#define QueuePop(f,l)            QueuePop_NZ(f,l,next,SetNull)
#define StackPush(f,n)           StackPush_N(f,n,next)
#define StackPop(f)              StackPop_NZ(f,next,CheckNull)
#define DLLPushBack(f,l,n)       DLLPushBack_NPZ(f,l,n,next,prev,CheckNull,SetNull)
#define DLLPushFront(f,l,n)      DLLPushBack_NPZ(l,f,n,prev,next,CheckNull,SetNull)
#define DLLInsert(f,l,p,n)       DLLInsert_NPZ(f,l,p,n,next,prev,CheckNull,SetNull)
#define DLLRemove(f,l,n)         DLLRemove_NPZ(f,l,n,next,prev,CheckNull,SetNull)

//- rjf: clamps, mins, maxes

#define Min(a, b) (((a)<(b)) ? (a) : (b))
#define Max(a, b) (((a)>(b)) ? (a) : (b))
#define ClampTop(x, a) Min(x,a)
#define ClampBot(a, x) Max(a,x)
#define Clamp(a, x, b) (((a)>(x))?(a):((b)<(x))?(b):(x))

//- rjf: defer-loop

#define DeferLoop(start, end) for(int _i_ = ((start), 0); _i_ == 0; _i_ += 1, (end))
#define DeferLoopChecked(begin, end) for(int _i_ = 2 * !(begin); (_i_ == 2 ? ((end), 0) : !_i_); _i_ += 1, (end))

////////////////////////////////
//~ rjf: Base-Types

typedef int8_t   S8;
typedef int16_t  S16;
typedef int32_t  S32;
typedef int64_t  S64;
typedef uint8_t  U8;
typedef uint16_t U16;
typedef uint32_t U32;
typedef uint64_t U64;
typedef S8       B8;
typedef S16      B16;
typedef S32      B32;
typedef S64      B64;
typedef float    F32;
typedef double   F64;
typedef void VoidFunction(void);

////////////////////////////////
//~ rjf: Limits

read_only global U8 U8Max = 0xFF;
read_only global U8 U8Min = 0;

read_only global U16 U16Max = 0xFFFF;
read_only global U16 U16Min = 0;

read_only global U32 U32Max = 0xFFFFFFFF;
read_only global U32 U32Min = 0;

read_only global U64 U64Max = 0xFFFFFFFFFFFFFFFF;
read_only global U64 U64Min = 0;

read_only global S8 S8Max = 0x7F;
read_only global S8 S8Min = -1 - 0x7F;

read_only global S16 S16Max = 0x7FFF;
read_only global S16 S16Min = -1 - 0x7FFF;

read_only global S32 S32Max = 0x7FFFFFFF;
read_only global S32 S32Min = -1 - 0x7FFFFFFF;

read_only global S64 S64Max = 0x7FFFFFFFFFFFFFFF;
read_only global S64 S64Min = -1 - 0x7FFFFFFFFFFFFFFF;

read_only global U32 SignF32 = 0x80000000;
read_only global U32 ExponentF32 = 0x7F800000;
read_only global U32 MantissaF32 = 0x7FFFFF;

////////////////////////////////
//~ rjf: Constants

read_only global F32 F32Max = 3.4028234664e+38;
read_only global F32 F32Min = -3.4028234664e+38;
read_only global F32 F32SmallestPositive = 1.1754943508e-38;
read_only global F32 F32Epsilon = 5.96046448e-8;

////////////////////////////////
//~ rjf: Base Enums

typedef enum Side
{
 Side_Invalid = -1,
 Side_Min,
 Side_Max,
 Side_COUNT
}
Side;

typedef enum Axis2
{
 Axis2_Invalid = -1,
 Axis2_X,
 Axis2_Y,
 Axis2_COUNT
}
Axis2;
#define Axis2_Flip(a) ((Axis2)(!(a)))

typedef enum Axis3
{
 Axis3_Invalid = -1,
 Axis3_X,
 Axis3_Y,
 Axis3_Z,
 Axis3_COUNT
}
Axis3;

typedef enum Axis4
{
 Axis4_Invalid = -1,
 Axis4_X,
 Axis4_Y,
 Axis4_Z,
 Axis4_W,
 Axis4_COUNT
}
Axis4;

typedef enum Corner
{
 Corner_Invalid = -1,
 Corner_00,
 Corner_01,
 Corner_10,
 Corner_11,
 Corner_COUNT
}
Corner;

typedef enum Comparison
{
 Comparison_Null,
 Comparison_EqualTo,
 Comparison_NotEqualTo,
 Comparison_LessThan,
 Comparison_LessThanOrEqualTo,
 Comparison_GreaterThan,
 Comparison_GreaterThanOrEqualTo,
 Comparison_COUNT
}
Comparison;

////////////////////////////////
//~ rjf: Member Offset Helper

typedef struct MemberOffset MemberOffset;
struct MemberOffset
{
 U64 v;
};

#define MemberOff(S, member) (MemberOffset){OffsetOf(S, member)}
#define MemberFromOff(ptr, type, memoff) (*(type *)((U8 *)ptr + memoff.v))

////////////////////////////////
//~ rjf: Text Coordinates

typedef struct TxtPt TxtPt;
struct TxtPt
{
 S64 line;
 S64 column;
};

typedef struct TxtRng TxtRng;
struct TxtRng
{
 TxtPt min;
 TxtPt max;
};

inline_function TxtPt
TxtPtMake(S64 line, S64 column)
{
 TxtPt result = {line, column};
 return result;
}

inline_function B32
TxtPtMatch(TxtPt a, TxtPt b)
{
 return a.line == b.line && a.column == b.column;
}

inline_function B32
TxtPtLessThan(TxtPt a, TxtPt b)
{
 B32 result = (a.line < b.line ? 1 :
               b.line < a.line ? 0 :
               (a.column < b.column));
 return result;
}

inline_function TxtRng
TxtRngMake(TxtPt min, TxtPt max)
{
 TxtRng rng = {0};
 rng.min = min;
 rng.max = max;
 if(TxtPtLessThan(rng.max, rng.min))
 {
  Swap(TxtPt, rng.min, rng.max);
 }
 return rng;
}

////////////////////////////////
//~ rjf: Assertions

#if OS_WINDOWS
#define BreakDebugger() __debugbreak()
#else
#define BreakDebugger() (*(volatile int *)0 = 0)
#endif

#undef Assert
#define Assert(b) do { if(!(b)) { BreakDebugger(); } } while(0)
#define StaticAssert(c,label) U8 static_assert_##label[(c)?(1):(-1)]
#define NotImplemented Assert(!"Not Implemented")
#define InvalidPath Assert(!"Invalid Path")

////////////////////////////////
//~ rjf: Bit Patterns

core_function U32 UpToPow2_32(U32 x);
core_function U64 UpToPow2_64(U64 x);
core_function U32 SearchFirstOneBit_32_BinarySearch(U32 x);
core_function U32 SearchFirstOneBit_64_BinarySearch(U64 x);

inline_function F32
F32Inf(void)
{
 union { U32 u; F32 f; } x;
 x.u = 0x7F800000;
 return x.f;
}

inline_function F32
F32NegInf(void)
{
 union { U32 u; F32 f; } x;
 x.u = 0xFF800000;
 return x.f;
}

inline_function B32
F32IsNan(F32 f)
{
 union { U32 u; F32 f; } x;
 x.f = f;
 return ((x.u & ExponentF32) == ExponentF32) && ((x.u & MantissaF32) != 0);
}

inline_function B32
F32IsDenorm(F32 f)
{
 union { U32 u; F32 f; } x;
 x.f = f;
 return ((x.u & ExponentF32) == 0) && ((x.u & MantissaF32) != 0);
}

inline_function F32
AbsoluteValueF32(F32 f)
{
 union { U32 u; F32 f; } x;
 x.f = f;
 x.u = x.u & ~SignF32;
 return x.f;
}

////////////////////////////////
//~ rjf: Comparisons

core_function B32 Compare_U64(U64 a, U64 b, Comparison comparison);

#endif // BASE_TYPES_H
