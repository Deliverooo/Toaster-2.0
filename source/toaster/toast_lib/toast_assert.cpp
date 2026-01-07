#include "toast_assert.h"
#include "logging.hpp"

void printAssertFailed(const char *file, const int line, const char *function, const char *expression)
{
	LOG_FATAL("Assertion failed in: [{} : {}] : {} -> expr: \"{}\"\n", file, line, function, expression);
}

void printAssertFailedMsg(const char *file, const int line, const char *function, const char *expression, const char *message)
{
	LOG_FATAL("Assertion failed in: [{} : {}] : {} -> expr: \"{}\"\n \t{}\n", file, line, function, expression, message);
}
