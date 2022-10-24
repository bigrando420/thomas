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
- that's literally it.

## Dump
- [ ] basic memory arenas that don't make use of 64-bit so I can be portable with WASM
- [ ] change TH_NamingConventionBackToThis
- [ ] custom string types + operations

*/

// external
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// I am speed.
#include "th_dump.h"
#include "th_array.h"
#include "th_memory.h"
// thomas_internal.cpp - define include

#endif