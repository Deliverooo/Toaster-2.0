#include "toast_lib/toast_assert.h"

#include <Windows.h>

auto printAssertFailed(const char *file, const int line, const char *function, const char *expression) -> void
{
	std::printf("Assertion failed in: [%s : %d] : %s -> expr: \"%s\"\n", file, line, function, expression);
	MessageBox(nullptr, "Assertion failed: check console", "Assertion failed", MB_ICONERROR);
}

auto printAssertFailedMsg(const char *file, const int line, const char *function, const char *expression, const char *message) -> void
{
	std::printf("%s\n", message);
	std::printf("Assertion failed in: [%s : %d] : %s -> expr: \"%s\"\n", file, line, function, expression);
	MessageBox(nullptr, "Assertion failed: check console", "Assertion failed", MB_ICONERROR);
}
