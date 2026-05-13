#pragma once

#include <cstdlib> // Silences a warning saying that stdlib.h is deprecated when using with c++

auto printAssertFailed(const char *file, int line, const char *function, const char *expression) -> void;
auto printAssertFailedMsg(const char *file, int line, const char *function, const char *expression, const char *message) -> void;

#ifndef NDEBUG

#define TST_ASSERT(__expr)\
	do { if(!(__expr))\
	{\
		printAssertFailed(__FILE__, __LINE__, __func__, #__expr);\
		__debugbreak();\
	} } while(false)

#define TST_ASSERT_MSG(__expr, __msg)\
	do { if(!(__expr))\
	{\
		printAssertFailedMsg(__FILE__, __LINE__, __func__, #__expr, __msg);\
		__debugbreak();\
	} } while(false)

#else

#define TST_ASSERT(_expr)
#define TST_ASSERT_MSG(_expr, _msg)

#endif

#define TST_PERMA_ASSERT(__expr)\
		do { if(!(__expr))\
		{\
		printAssertFailed(__FILE__, __LINE__, __func__, #__expr);\
		std::abort();\
		} } while(false)
#define TST_PERMA_ASSERT_MSG(__expr, __msg)\
		do { if(!(__expr))\
		{\
		printAssertFailedMsg(__FILE__, __LINE__, __func__, #__expr, __msg);\
		std::abort();\
		} } while(false)
