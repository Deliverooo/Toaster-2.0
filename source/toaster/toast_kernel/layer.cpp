#include "layer.hpp"

#include "application.hpp"
#include "toast_render/render_context.hpp"

namespace toaster
{
	IAppLayer::IAppLayer(Application *p_app) : m_app(p_app), m_renderCtx(p_app->m_renderContext), m_globals(p_app->m_renderContext->getGlobals()),
											   m_inputCtx(p_app->getWindow().getInputContext())
	{
	}

	IAppLayer::~IAppLayer()
	{
	}
}
