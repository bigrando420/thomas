#ifndef SPEED_H
#define SPEED_H

/*

## Design Philosophy
- Keep it (C)imple.
- Speed is king.
- lightweight and portable

### Why C++ over of C ?
- operator overloading for common math ops
- simple templated generics for array<> containers
- that's literally it. 95% of the codebase can be compiled into C

## Dump
- [ ] basic memory arenas that don't make use of 64-bit so I can be portable with WASM

*/

// I am speed.
#include "th_dump.h"
#include "th_array.h"
#include "th_memory.h"
// thomas_internal.cpp - define include

#endif