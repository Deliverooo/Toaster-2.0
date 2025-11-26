#pragma once
#include <vcruntime.h>

namespace tst
{
}

#if TST_TRACK_ALLOCATIONS

#ifdef TST_PLATFORM_WINDOWS

_NODISCARD _Ret_notnull_ _Post_writable_byte_size_(size) _VCRT_ALLOCATOR void * __CRTDECL operator new(size_t size);
_NODISCARD _Ret_notnull_ _Post_writable_byte_size_(size) _VCRT_ALLOCATOR void * __CRTDECL operator new[](size_t size);
_NODISCARD _Ret_notnull_ _Post_writable_byte_size_(size) _VCRT_ALLOCATOR void * __CRTDECL operator new(size_t size, const char *desc);
_NODISCARD _Ret_notnull_ _Post_writable_byte_size_(size) _VCRT_ALLOCATOR void * __CRTDECL operator new[](size_t size, const char *desc);
_NODISCARD _Ret_notnull_ _Post_writable_byte_size_(size) _VCRT_ALLOCATOR void * __CRTDECL operator new(size_t size, const char *file, int line);
_NODISCARD _Ret_notnull_ _Post_writable_byte_size_(size) _VCRT_ALLOCATOR void * __CRTDECL operator new[](size_t size, const char *file, int line);
void __CRTDECL                                                                            operator delete(void *memory);
void __CRTDECL                                                                            operator delete(void *memory, const char *desc);
void __CRTDECL                                                                            operator delete(void *memory, const char *file, int line);
void __CRTDECL                                                                            operator delete[](void *memory);
void __CRTDECL                                                                            operator delete[](void *memory, const char *desc);
void __CRTDECL                                                                            operator delete[](void *memory, const char *file, int line);

#define tnew new(__FILE__, __LINE__)
#define tdelete delete

#else
#warning "Memory tracking not available on non-Windows platform"
#define tnew new
#define tdelete delete

#endif

#else

#define tnew new
#define tdelete delete

#endif
