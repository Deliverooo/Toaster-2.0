# Render Graph - Code Skeleton to Start With

Use this as a template. Copy these files and fill in the implementations as you follow the checklist.

---

## File 1: vk_render_graph_types.hpp

Save as: `source/toaster/toast_gpu/vk/vk_render_graph_types.hpp`

```cpp
#pragma once

#include <cstdint>
#include <vulkan/vulkan_raii.hpp>

namespace toaster::gpu
{
	// Opaque handle to a render graph resource
	using RenderGraphResourceHandle = uint32_t;
	constexpr RenderGraphResourceHandle InvalidResourceHandle = UINT32_MAX;

	// Resource type classification
	enum class ERenderGraphResourceType : uint8_t
	{
		eImage,    // vk::raii::Image or vk::Image
		eBuffer,   // vk::raii::Buffer
		eExternal, // Unowned resource (e.g., swapchain image)
	};

	// Resource access mode
	enum class EResourceAccess : uint8_t
	{
		eRead,
		eWrite,
	};

	// Combined access information
	struct ResourceAccessInfo
	{
		RenderGraphResourceHandle handle;
		EResourceAccess access;
		vk::AccessFlags2 accessMask;
		vk::ImageLayout imageLayout; // Expected layout for this access
	};
}
```

---

## File 2: vk_render_graph_resource.hpp

Save as: `source/toaster/toast_gpu/vk/vk_render_graph_resource.hpp`

```cpp
#pragma once

#include "vk_render_graph_types.hpp"
#include "toast_lib/system_types.h"

namespace toaster::gpu
{
	class RenderGraphResource
	{
	public:
		RenderGraphResource(RenderGraphResourceHandle handle, ERenderGraphResourceType type, vk::Format format = vk::Format::eUndefined);
		~RenderGraphResource() = default;

		// Prevent copying
		RenderGraphResource(const RenderGraphResource &)            = delete;
		RenderGraphResource &operator=(const RenderGraphResource &) = delete;

		// Allow moving
		RenderGraphResource(RenderGraphResource &&) noexcept            = default;
		RenderGraphResource &operator=(RenderGraphResource &&) noexcept = default;

		// Handle & Type
		[[nodiscard]] RenderGraphResourceHandle getHandle() const { return m_handle; }
		[[nodiscard]] ERenderGraphResourceType getType() const { return m_type; }
		[[nodiscard]] vk::Format getFormat() const { return m_format; }
		[[nodiscard]] vk::Extent3D getExtent() const { return m_extent; }

		// Current state
		[[nodiscard]] vk::ImageLayout getCurrentLayout() const { return m_currentLayout; }
		[[nodiscard]] vk::AccessFlags2 getCurrentAccessMask() const { return m_currentAccessMask; }

		void setCurrentLayout(vk::ImageLayout layout) { m_currentLayout = layout; }
		void setCurrentAccessMask(vk::AccessFlags2 mask) { m_currentAccessMask = mask; }

		void setExtent(vk::Extent3D extent) { m_extent = extent; }

		// Generic data storage (holds void* to actual Vulkan resource)
		template <typename T>
		void setData(T *data)
		{
			m_data = static_cast<void *>(data);
		}

		template <typename T>
		[[nodiscard]] T *getData() const
		{
			return static_cast<T *>(m_data);
		}

		[[nodiscard]] void *getRawData() const { return m_data; }

	private:
		RenderGraphResourceHandle m_handle;
		ERenderGraphResourceType m_type;
		vk::Format m_format;
		vk::Extent3D m_extent{0, 0, 0};
		vk::ImageLayout m_currentLayout{vk::ImageLayout::eUndefined};
		vk::AccessFlags2 m_currentAccessMask{vk::AccessFlagBits2::eNone};
		void *m_data{nullptr};
	};
}
```

---

## File 3: vk_render_pass.hpp

Save as: `source/toaster/toast_gpu/vk/vk_render_pass.hpp`

```cpp
#pragma once

#include "vk_render_graph_types.hpp"
#include <vector>
#include <string>
#include <functional>

namespace toaster::gpu
{
	// Forward declaration
	class RenderPassContext;

	class RenderPass
	{
	public:
		using ExecuteCallback = std::function<void(RenderPassContext &)>;

		explicit RenderPass(const char *name);
		~RenderPass() = default;

		// Prevent copying, allow moving
		RenderPass(const RenderPass &)            = delete;
		RenderPass &operator=(const RenderPass &) = delete;
		RenderPass(RenderPass &&) noexcept            = default;
		RenderPass &operator=(RenderPass &&) noexcept = default;

		// Metadata
		[[nodiscard]] const char *getName() const { return m_name.c_str(); }
		[[nodiscard]] const std::vector<ResourceAccessInfo> &getReads() const { return m_reads; }
		[[nodiscard]] const std::vector<ResourceAccessInfo> &getWrites() const { return m_writes; }

		// Internal: Called by builder
		void _addRead(const ResourceAccessInfo &access) { m_reads.push_back(access); }
		void _addWrite(const ResourceAccessInfo &access) { m_writes.push_back(access); }
		void _setExecuteCallback(ExecuteCallback callback) { m_executeCallback = callback; }

		// Execution
		void execute(RenderPassContext &ctx) const;

	private:
		std::string m_name;
		std::vector<ResourceAccessInfo> m_reads;
		std::vector<ResourceAccessInfo> m_writes;
		ExecuteCallback m_executeCallback;
	};
}
```

---

## File 4: vk_render_pass_builder.hpp

Save as: `source/toaster/toast_gpu/vk/vk_render_pass_builder.hpp`

```cpp
#pragma once

#include "vk_render_graph_types.hpp"
#include "vk_render_pass.hpp"
#include <functional>

namespace toaster::gpu
{
	class ResourceRegistry;

	class RenderPassBuilder
	{
	public:
		RenderPassBuilder(RenderPass &pass, ResourceRegistry &registry);

		// Fluent API
		RenderPassBuilder &reads(RenderGraphResourceHandle handle,
								 vk::AccessFlags2 accessMask = vk::AccessFlagBits2::eShaderRead);

		RenderPassBuilder &writes(RenderGraphResourceHandle handle,
								  vk::AccessFlags2 accessMask = vk::AccessFlagBits2::eColorAttachmentWrite);

		RenderPassBuilder &execute(std::function<void(RenderPassContext &)> callback);

		// Build (return reference to pass for chaining if needed)
		[[nodiscard]] RenderPass &build() { return m_pass; }

	private:
		RenderPass &m_pass;
		ResourceRegistry &m_registry;
	};
}
```

---

## File 5: vk_render_pass_context.hpp

Save as: `source/toaster/toast_gpu/vk/vk_render_pass_context.hpp`

```cpp
#pragma once

#include "vk_render_graph_types.hpp"
#include <vulkan/vulkan_raii.hpp>
#include "toast_lib/system_types.h"

namespace toaster::gpu
{
	class RenderPass;
	class ResourceRegistry;

	// Context passed to pass execution callback
	class RenderPassContext
	{
	public:
		RenderPassContext(const RenderPass *pass, vk::raii::CommandBuffer &cmdBuffer, ResourceRegistry &registry, uint32 frameIndex);

		// Command buffer access
		[[nodiscard]] vk::raii::CommandBuffer &getCommandBuffer() { return m_cmdBuffer; }

		// Frame information
		[[nodiscard]] uint32 getFrameIndex() const { return m_frameIndex; }
		[[nodiscard]] const char *getPassName() const;

		// Resource access
		[[nodiscard]] RenderGraphResource *getResource(RenderGraphResourceHandle handle);
		[[nodiscard]] const RenderGraphResource *getResource(RenderGraphResourceHandle handle) const;

		template <typename T>
		[[nodiscard]] T *getResourceData(RenderGraphResourceHandle handle)
		{
			auto *res = getResource(handle);
			return res ? res->getData<T>() : nullptr;
		}

		// Layout query (for debugging or manual use)
		[[nodiscard]] vk::ImageLayout getResourceLayout(RenderGraphResourceHandle handle) const;

	private:
		const RenderPass *m_pass;
		vk::raii::CommandBuffer &m_cmdBuffer;
		ResourceRegistry &m_registry;
		uint32 m_frameIndex;
	};
}
```

---

## File 6: vk_render_graph_resource_registry.hpp

Save as: `source/toaster/toast_gpu/vk/vk_render_graph_resource_registry.hpp`

```cpp
#pragma once

#include "vk_render_graph_resource.hpp"
#include "toast_lib/system_types.h"
#include <unordered_map>
#include <string>

namespace toaster::gpu
{
	class VKGPUContext;

	class ResourceRegistry
	{
	public:
		explicit ResourceRegistry(VKGPUContext *ctx);
		~ResourceRegistry() = default;

		// Prevent copying
		ResourceRegistry(const ResourceRegistry &)            = delete;
		ResourceRegistry &operator=(const ResourceRegistry &) = delete;

		// Resource registration
		[[nodiscard]] RenderGraphResourceHandle registerImage(vk::Format format, uint32 width, uint32 height, const char *debugName = "");

		[[nodiscard]] RenderGraphResourceHandle registerExternalImage(vk::Image image, vk::ImageView imageView, vk::Format format, uint32 width, uint32 height,
																	  const char *debugName = "");

		// Lookup
		[[nodiscard]] RenderGraphResource *getResource(RenderGraphResourceHandle handle);
		[[nodiscard]] const RenderGraphResource *getResource(RenderGraphResourceHandle handle) const;

		// Lifecycle
		void clear();

		[[nodiscard]] size_t getResourceCount() const { return m_resources.size(); }

	private:
		VKGPUContext *m_ctx;
		std::unordered_map<uint32, RenderGraphResource> m_resources;
		uint32 m_nextHandle{0};
	};
}
```

---

## File 7: vk_render_graph.hpp

Save as: `source/toaster/toast_gpu/vk/vk_render_graph.hpp`

```cpp
#pragma once

#include "vk_render_graph_resource_registry.hpp"
#include "vk_render_pass.hpp"
#include "vk_render_pass_builder.hpp"
#include "toast_lib/system_types.h"
#include <vector>
#include <memory>
#include <vulkan/vulkan_raii.hpp>

namespace toaster::gpu
{
	class VKGPUContext;

	class RenderGraph
	{
	public:
		explicit RenderGraph(VKGPUContext *ctx);
		~RenderGraph() = default;

		// Prevent copying
		RenderGraph(const RenderGraph &)            = delete;
		RenderGraph &operator=(const RenderGraph &) = delete;

		// Allow moving
		RenderGraph(RenderGraph &&) noexcept            = default;
		RenderGraph &operator=(RenderGraph &&) noexcept = default;

		// === Building Phase ===

		// Add a rendering pass
		[[nodiscard]] RenderPassBuilder &addPass(const char *passName);

		// Resource management
		[[nodiscard]] RenderGraphResourceHandle createImage(vk::Format format, uint32 width, uint32 height, const char *debugName = "");

		[[nodiscard]] RenderGraphResourceHandle registerExternalImage(vk::Image image, vk::ImageView imageView, vk::Format format, uint32 width, uint32 height,
																	  const char *debugName = "");

		// === Compilation Phase ===

		void compile();

		// === Execution Phase ===

		void execute(uint32 frameIndex);

		// === Query ===

		[[nodiscard]] bool isCompiled() const { return m_compiled; }
		[[nodiscard]] size_t getPassCount() const { return m_passes.size(); }
		[[nodiscard]] size_t getResourceCount() const { return m_resources.getResourceCount(); }

	private:
		VKGPUContext *m_ctx;
		ResourceRegistry m_resources;
		std::vector<RenderPass> m_passes;
		std::vector<size_t> m_executionOrder; // Indices of passes in execution order
		bool m_compiled{false};

		// Compilation helpers
		void _validateDependencies();
		void _buildExecutionOrder();

		// Execution helpers
		void _recordImageTransition(vk::raii::CommandBuffer &cmdBuffer, RenderGraphResource &resource, vk::ImageLayout oldLayout, vk::ImageLayout newLayout,
								   vk::AccessFlags2 oldAccessMask, vk::AccessFlags2 newAccessMask);

		[[nodiscard]] vk::ImageLayout _getLayoutForAccessMask(vk::AccessFlags2 accessMask) const;
	};
}
```

---

## File 8: vk_render_graph.cpp (Implementation Stubs)

Save as: `source/toaster/toast_gpu/vk/vk_render_graph.cpp`

```cpp
#include "vk_render_graph.hpp"
#include "vk_render_pass_context.hpp"
#include "vk_gpu_context.hpp"
#include "toast_lib/logging.hpp"
#include "toast_lib/toast_assert.h"

namespace toaster::gpu
{
	RenderGraph::RenderGraph(VKGPUContext *ctx) : m_ctx(ctx), m_resources(ctx)
	{
		TST_ASSERT_MSG(ctx, "VKGPUContext cannot be null");
	}

	RenderPassBuilder &RenderGraph::addPass(const char *passName)
	{
		TST_ASSERT_MSG(!m_compiled, "Cannot add passes after compilation");
		m_passes.emplace_back(passName);
		return *new RenderPassBuilder(m_passes.back(), m_resources);
		// TODO: Proper memory management for builder
	}

	RenderGraphResourceHandle RenderGraph::createImage(vk::Format format, uint32 width, uint32 height, const char *debugName)
	{
		return m_resources.registerImage(format, width, height, debugName);
	}

	RenderGraphResourceHandle RenderGraph::registerExternalImage(vk::Image image, vk::ImageView imageView, vk::Format format, uint32 width, uint32 height,
															   const char *debugName)
	{
		return m_resources.registerExternalImage(image, imageView, format, width, height, debugName);
	}

	void RenderGraph::compile()
	{
		_validateDependencies();
		_buildExecutionOrder();
		m_compiled = true;

		LOG_INFO("Render graph compiled: {} passes, {} resources", m_passes.size(), m_resources.getResourceCount());
	}

	void RenderGraph::_validateDependencies()
	{
		// TODO: Implement dependency validation
		// For each pass, ensure all reads have a producer
		// Ensure no circular dependencies
		// Ensure resource format/extent are consistent
	}

	void RenderGraph::_buildExecutionOrder()
	{
		// TODO: Implement topological sort
		// Order passes by their dependencies
		// If pass B reads from pass A, ensure A comes before B

		// For now, just use linear order
		m_executionOrder.resize(m_passes.size());
		for (size_t i = 0; i < m_passes.size(); ++i)
		{
			m_executionOrder[i] = i;
		}
	}

	void RenderGraph::execute(uint32 frameIndex)
	{
		TST_ASSERT_MSG(m_compiled, "Graph must be compiled before execution");

		// TODO: Get command buffer from swapchain
		// auto& cmdBuffer = m_ctx->getDevice().allocateCommandBuffers(...)[0];

		// TODO: For each pass in execution order:
		//   - Insert layout transitions
		//   - Create RenderPassContext
		//   - Call pass.execute(ctx)

		// For now, just iterate to test structure
		for (size_t passIdx : m_executionOrder)
		{
			RenderPass &pass = m_passes[passIdx];
			LOG_TRACE("Would execute pass: {}", pass.getName());
		}
	}

	void RenderGraph::_recordImageTransition(vk::raii::CommandBuffer &cmdBuffer, RenderGraphResource &resource, vk::ImageLayout oldLayout,
											 vk::ImageLayout newLayout, vk::AccessFlags2 oldAccessMask, vk::AccessFlags2 newAccessMask)
	{
		// TODO: Implement image layout transition
		// Similar to VKGPUContext::transitionImageLayout but simpler
	}

	vk::ImageLayout RenderGraph::_getLayoutForAccessMask(vk::AccessFlags2 accessMask) const
	{
		// TODO: Map access masks to appropriate image layouts
		// eColorAttachmentWrite -> eColorAttachmentOptimal
		// eColorAttachmentRead -> eColorAttachmentOptimal
		// eShaderRead -> eShaderReadOnlyOptimal
		// eDepthStencilAttachmentWrite -> eDepthAttachmentOptimal
		// etc.

		return vk::ImageLayout::eGeneral; // Placeholder
	}
}
```

---

## File 9: vk_render_pass_context.cpp

Save as: `source/toaster/toast_gpu/vk/vk_render_pass_context.cpp`

```cpp
#include "vk_render_pass_context.hpp"
#include "vk_render_pass.hpp"
#include "vk_render_graph_resource_registry.hpp"
#include "toast_lib/logging.hpp"

namespace toaster::gpu
{
	RenderPassContext::RenderPassContext(const RenderPass *pass, vk::raii::CommandBuffer &cmdBuffer, ResourceRegistry &registry, uint32 frameIndex)
		: m_pass(pass), m_cmdBuffer(cmdBuffer), m_registry(registry), m_frameIndex(frameIndex)
	{
	}

	const char *RenderPassContext::getPassName() const
	{
		return m_pass ? m_pass->getName() : "Unknown";
	}

	RenderGraphResource *RenderPassContext::getResource(RenderGraphResourceHandle handle)
	{
		return m_registry.getResource(handle);
	}

	const RenderGraphResource *RenderPassContext::getResource(RenderGraphResourceHandle handle) const
	{
		return m_registry.getResource(handle);
	}

	vk::ImageLayout RenderPassContext::getResourceLayout(RenderGraphResourceHandle handle) const
	{
		auto *res = getResource(handle);
		return res ? res->getCurrentLayout() : vk::ImageLayout::eUndefined;
	}
}
```

---

## File 10: vk_render_pass.cpp

Save as: `source/toaster/toast_gpu/vk/vk_render_pass.cpp`

```cpp
#include "vk_render_pass.hpp"
#include "vk_render_pass_context.hpp"
#include "toast_lib/logging.hpp"

namespace toaster::gpu
{
	RenderPass::RenderPass(const char *name) : m_name(name)
	{
	}

	void RenderPass::execute(RenderPassContext &ctx) const
	{
		if (!m_executeCallback)
		{
			LOG_WARN("Pass '{}' has no execute callback", m_name);
			return;
		}

		m_executeCallback(ctx);
	}
}
```

---

## File 11: vk_render_pass_builder.cpp

Save as: `source/toaster/toast_gpu/vk/vk_render_pass_builder.cpp`

```cpp
#include "vk_render_pass_builder.hpp"
#include "vk_render_graph_resource_registry.hpp"
#include "toast_lib/logging.hpp"

namespace toaster::gpu
{
	RenderPassBuilder::RenderPassBuilder(RenderPass &pass, ResourceRegistry &registry) : m_pass(pass), m_registry(registry)
	{
	}

	RenderPassBuilder &RenderPassBuilder::reads(RenderGraphResourceHandle handle, vk::AccessFlags2 accessMask)
	{
		auto *resource = m_registry.getResource(handle);
		if (!resource)
		{
			LOG_WARN("Pass '{}': Reading from non-existent resource", m_pass.getName());
			return *this;
		}

		ResourceAccessInfo access{handle, EResourceAccess::eRead, accessMask, vk::ImageLayout::eShaderReadOnlyOptimal};
		m_pass._addRead(access);

		return *this;
	}

	RenderPassBuilder &RenderPassBuilder::writes(RenderGraphResourceHandle handle, vk::AccessFlags2 accessMask)
	{
		auto *resource = m_registry.getResource(handle);
		if (!resource)
		{
			LOG_WARN("Pass '{}': Writing to non-existent resource", m_pass.getName());
			return *this;
		}

		ResourceAccessInfo access{handle, EResourceAccess::eWrite, accessMask, vk::ImageLayout::eColorAttachmentOptimal};
		m_pass._addWrite(access);

		return *this;
	}

	RenderPassBuilder &RenderPassBuilder::execute(std::function<void(RenderPassContext &)> callback)
	{
		m_pass._setExecuteCallback(callback);
		return *this;
	}
}
```

---

## File 12: vk_render_graph_resource_registry.cpp

Save as: `source/toaster/toast_gpu/vk/vk_render_graph_resource_registry.cpp`

```cpp
#include "vk_render_graph_resource_registry.hpp"
#include "vk_gpu_context.hpp"
#include "toast_lib/logging.hpp"
#include "toast_lib/toast_assert.h"

namespace toaster::gpu
{
	ResourceRegistry::ResourceRegistry(VKGPUContext *ctx) : m_ctx(ctx)
	{
		TST_ASSERT_MSG(ctx, "VKGPUContext cannot be null");
	}

	RenderGraphResourceHandle ResourceRegistry::registerImage(vk::Format format, uint32 width, uint32 height, const char *debugName)
	{
		// TODO: Allocate actual vk::raii::Image via m_ctx->createImage(...)

		uint32 handle = m_nextHandle++;
		RenderGraphResource resource(handle, ERenderGraphResourceType::eImage, format);
		resource.setExtent({width, height, 1});

		LOG_TRACE("Registered image '{}' handle={}", debugName ? debugName : "unnamed", handle);

		m_resources.emplace(handle, std::move(resource));
		return handle;
	}

	RenderGraphResourceHandle ResourceRegistry::registerExternalImage(vk::Image image, vk::ImageView imageView, vk::Format format, uint32 width, uint32 height,
																	   const char *debugName)
	{
		uint32 handle = m_nextHandle++;
		RenderGraphResource resource(handle, ERenderGraphResourceType::eExternal, format);
		resource.setExtent({width, height, 1});
		resource.setData(new vk::Image(image)); // Store the handle

		LOG_TRACE("Registered external image '{}' handle={}", debugName ? debugName : "unnamed", handle);

		m_resources.emplace(handle, std::move(resource));
		return handle;
	}

	RenderGraphResource *ResourceRegistry::getResource(RenderGraphResourceHandle handle)
	{
		auto it = m_resources.find(handle);
		return it != m_resources.end() ? &it->second : nullptr;
	}

	const RenderGraphResource *ResourceRegistry::getResource(RenderGraphResourceHandle handle) const
	{
		auto it = m_resources.find(handle);
		return it != m_resources.end() ? &it->second : nullptr;
	}

	void ResourceRegistry::clear()
	{
		m_resources.clear();
		m_nextHandle = 0;
	}
}
```

---

## Integration Steps

1. **Copy all 12 files** above into your workspace
2. **Add include directories** to your CMakeLists.txt
3. **Compile** - Fix any include/syntax issues
4. **Test** - Create a simple render graph in ClientLayer
5. **Iterate** - Follow the checklist to fill in implementations

---

## Minimal CMakeLists.txt Addition

Add to your GPU library CMakeLists.txt:

```cmake
set(VK_RENDER_GRAPH_SOURCES
    vk/vk_render_graph.hpp
    vk/vk_render_graph.cpp
    vk/vk_render_graph_types.hpp
    vk/vk_render_graph_resource.hpp
    vk/vk_render_graph_resource_registry.hpp
    vk/vk_render_graph_resource_registry.cpp
    vk/vk_render_pass.hpp
    vk/vk_render_pass.cpp
    vk/vk_render_pass_builder.hpp
    vk/vk_render_pass_builder.cpp
    vk/vk_render_pass_context.hpp
    vk/vk_render_pass_context.cpp
)

target_sources(YourGPULibraryName PRIVATE ${VK_RENDER_GRAPH_SOURCES})
```

---

Good luck! Start with Phase 1 (types), ensure it compiles, then move to Phase 2.
