#pragma once

#include <argparse/argparse.hpp>

#include "toast_lib/io/filesystem.hpp"

namespace toaster::tstb
{
	// Toaster Build
	class TstB final
	{
	public:
		TstB();

		auto newCppProj(const argparse::ArgumentParser &p_new_cpp_proj_cmd) -> int32;

	private:
	};
}
