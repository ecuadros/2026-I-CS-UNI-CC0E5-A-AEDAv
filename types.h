#ifndef __TYPES_H__
#define __TYPES_H__

// C/C++
// typedef int Type;

// C++11, C++14, C++17, C++20, C++23 ...
using Type = int;

// T1 must be int for 32-bit architecture and long long for 64-bit architecture
// It must work for windows, linux, iOS, macOS, android, etc.

using T1 = int;

using Ref = long;

// Token: alias del char usado en el parseo (separadores en operator>>). Mismo
// criterio que T1/Ref: el nativo va aliaseado para cambiarlo en un solo lugar,
// porque el tamaño de char puede variar según arquitectura/encoding.
using Token = char;

#endif // __TYPES_H__
