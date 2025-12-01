#pragma once

#include "memory/ref_ptr.hpp"
#include "pipeline_spec_info.hpp"

namespace tst::gpu
{
	class Pipeline : public RefCounted
	{
	public:
		Pipeline(const PipelineSpecInfo &pipeline_spec_info);
	};
}
