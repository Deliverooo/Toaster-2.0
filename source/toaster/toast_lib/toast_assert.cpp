#include "toast_assert.h"
#include "logging.hpp"

auto printAssertFailed(const char *file, const int line, const char *function, const char *expression) -> void
{
	LOG_FATAL("Assertion failed in: [{} : {}] : {} -> expr: \"{}\"\n", file, line, function, expression);
}

auto printAssertFailedMsg(const char *file, const int line, const char *function, const char *expression, const char *message) -> void
{
	LOG_FATAL("Assertion failed in: [{} : {}] : {} -> expr: \"{}\"\n \t{}\n", file, line, function, expression, message);
}
