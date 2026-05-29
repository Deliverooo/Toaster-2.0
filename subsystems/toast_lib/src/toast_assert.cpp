#include "toast_lib/toast_assert.h"
#include "toast_lib/logging.hpp"

auto printAssertFailed(const char *file, const int line, const char *function, const char *expression) -> void
{
	LOG_FATAL("Assertion failed in: [{} : {}] : {} -> expr: \"{}\"\n", file, line, function, expression);
}

auto printAssertFailedMsg(const char *file, const int line, const char *function, const char *expression, const char *message) -> void
{
	LOG_FATAL("{}", message);
	LOG_FATAL("Assertion failed in: [{} : {}] : {} -> expr: \"{}\"\n \t", file, line, function, expression);
}
