#include "layer.hpp"

#include "application.hpp"

namespace toaster
{
	IAppLayer::IAppLayer(Application *p_app) : m_appParent(p_app), m_renderCtx(p_app->m_renderContext)
	{
	}

	IAppLayer::~IAppLayer()
	{
	}

	auto IAppLayer::getApp() -> Application &
	{
		return *m_appParent;
	}
}
