#include "toast_kernel/layer.hpp"

#include "toast_kernel/application.hpp"
#include "toast_render/render_context.hpp"

namespace toaster
{
	auto IAppLayer::_register(Application *p_app) -> void
	{
		m_app       = p_app;
		m_renderCtx = p_app->m_renderContext;
		m_globals   = p_app->m_renderContext->getGlobals();
		m_inputCtx  = p_app->getWindow().getInputContext();
	}
}
