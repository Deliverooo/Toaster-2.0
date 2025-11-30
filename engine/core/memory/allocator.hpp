/*
*  	Copyright 2025 Orbo Stetson
 *  	Licensed under the Apache License, Version 2.0 (the "License");
 *  	you may not use this file except in compliance with the License.
 *  	You may obtain a copy of the License at
 *
 *			http://www.apache.org/licenses/LICENSE-2.0
 *
 *    	Unless required by applicable law or agreed to in writing, software
 *    	distributed under the License is distributed on an "AS IS" BASIS,
 *    	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    	See the License for the specific language governing permissions and
 *    	limitations under the License.
 */
#pragma once

#include <memory>
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
