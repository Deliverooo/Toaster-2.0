#pragma once

#include "core_api.hpp"
#include "core_typedefs.hpp"

#include "layer.hpp"

#include <vector>

namespace tst
{
	class TST_CORE_API LayerStack
	{
	public:
		void pushLayer(ILayer *layer);
		void popLayer(ILayer *layer);

		[[nodiscard]] uint64 size() const { return m_layers.size(); }

		std::vector<ILayer *>::iterator begin() { return m_layers.begin(); }
		std::vector<ILayer *>::iterator end() { return m_layers.end(); }

	private:
		std::vector<ILayer *> m_layers;
		// This iterator points to the position where the next layer will be inserted
		uint32 m_layerInsertPos = 0u;
	};
}
