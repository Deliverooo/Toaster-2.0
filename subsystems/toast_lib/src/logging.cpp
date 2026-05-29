#include "toast_lib/logging.hpp"
#include "toast_lib/toast_assert.h"

namespace toaster::log
{
	std::FILE *s_outputFile{nullptr};

	auto getOutputFile() -> std::FILE *
	{
		return s_outputFile;
	}

	auto setOutputFile(const std::string &p_filepath) -> void
	{
		if (s_outputFile)
			std::fclose(s_outputFile);
		s_outputFile = std::fopen(p_filepath.c_str(), "w");
		TST_ASSERT(s_outputFile);
	}

	auto shutdown() -> void
	{
		if (s_outputFile)
			std::fclose(s_outputFile);
	}
}
