#pragma once

#include "ref_ptr.hpp"
#include "pipeline_spec_info.hpp"

namespace tst
{
	class Pipeline : public RefCounted
	{
	public:
		PipelineSpecInfo &      getSpecInfo() { return m_specification; }
		const PipelineSpecInfo &getSpecInfo() const { return m_specification; }

		nvrhi::GraphicsPipelineHandle getHandle() { return m_handle; }

		void invalidate();
		void _invalidate();

		RefPtr<Shader> getShader() const { return m_specification.shader; }

		bool isDynamicLineWidth() const;

	public:
		Pipeline(const PipelineSpecInfo &spec);

		virtual ~Pipeline() = default;

	private:
		nvrhi::GraphicsPipelineHandle m_handle = nullptr;
		PipelineSpecInfo              m_specification;
	};
}
