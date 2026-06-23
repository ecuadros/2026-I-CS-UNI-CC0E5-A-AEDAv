#ifndef __TYPES_H__
#define __TYPES_H__

#include <cstddef>

// C/C++
// typedef int Type;

// C++11, C++14, C++17, C++20, C++23 ...
using Type = int;

// T1 must be int for 32-bit architecture and long long for 64-bit architecture
// It must work for windows, linux, iOS, macOS, android, etc.

using T1 = int;

using Ref = long;

// P1 Tarea Traits: aliases para evitar tipos nativos
using Size   = std::size_t;
using Index  = long;
using Flag   = bool;
using Depth  = std::size_t;
using Token  = char;
using Byte   = unsigned char;

#endif // __TYPES_H__
