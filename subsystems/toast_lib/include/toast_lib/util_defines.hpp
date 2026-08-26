/*!
 * @file util_defines.hpp
 */
#pragma once

// #define ts this

#define TST_BIT(__n) (1u << __n)
#define TST_ALIGN(__val, __alignment)  ((__val + (__alignment - 1u)) & ~(__alignment - 1u))
#define TST_ALIGN_PTR(__ptr, __alignment) static_cast<decltype(__ptr)>( (void*) (((uintptr)__ptr + (__alignment - 1u)) & ~(__alignment - 1u)))

#define TST_UNUSED [[maybe_unused]]
