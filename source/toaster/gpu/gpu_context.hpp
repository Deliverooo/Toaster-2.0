#pragma once

#include "core_basic.hpp"
#include "gpu_common.hpp"

#include <unordered_set>

struct GLFWwindow;

namespace toaster
{
	inline std::vector<const char *> stringSetToVector(const std::unordered_set<std::string> &set)
	{
		std::vector<const char *> ret;
		for (const auto &s: set)
		{
			ret.push_back(s.c_str());
		}

		return ret;
	}
}

namespace toaster::gpu
{
	class GPUContext
	{
	public:
		static GPUContext *create(GLFWwindow *p_window);
		virtual            ~GPUContext() = default;
	};
}
