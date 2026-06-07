/*!
 * @file util_defines.hpp
 */
#pragma once

// #define ts this

#define BIT(__n) (1u << __n)

#define ALIGN(__ptr, __alignment) ((__ptr + (__alignment - 1u)) & ~(__alignment - 1u))