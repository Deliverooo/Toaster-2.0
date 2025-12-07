#include "layer_stack.hpp"

namespace tst
{
	void LayerStack::pushLayer(ILayer *layer)
	{
		// Insert the layer at the current insert position
		m_layers.emplace(m_layers.begin() + m_layerInsertPos, layer);
		m_layerInsertPos++;
	}

	void LayerStack::pushOverlay(ILayer *overlay)
	{
		m_layers.emplace_back(overlay);
	}

	void LayerStack::popLayer(ILayer *layer)
	{
		// Find the layer in the stack and remove it
		auto it = std::ranges::find(m_layers, layer);
		if (it != m_layers.end())
		{
			m_layers.erase(it);
			m_layerInsertPos--;
		}
	}

	void LayerStack::popOverlay(ILayer *overlay)
	{
		auto it = std::ranges::find(m_layers, overlay);
		if (it != m_layers.end())
			m_layers.erase(it);
	}
}
