#pragma once

#include <cstdlib> // Silences a warning saying that stdlib.h is deprecated when using with c++

void printAssertFailed(const char *file, int line, const char *function, const char *expression);
void printAssertFailedMsg(const char *file, int line, const char *function, const char *expression, const char *message);

#ifndef NDEBUG

#define TST_ASSERT(_expr)\
	do { if(!(_expr))\
	{\
		printAssertFailed(__FILE__, __LINE__, __func__, #_expr);\
		__debugbreak();\
	} } while(false)

#define TST_ASSERT_MSG(_expr, _msg)\
	do { if(!(_expr))\
	{\
		printAssertFailedMsg(__FILE__, __LINE__, __func__, #_expr, _msg);\
		__debugbreak();\
	} } while(false)

#else

#define TST_ASSERT(_expr)
#define TST_ASSERT_MSG(_expr, _msg)

#endif
