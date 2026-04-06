# Render Graph System - Integration Points & Implementation Guide

## Current Architecture Overview

Your rendering pipeline currently works like this:

```
VKSwapchain::beginFrame()
    ↓
ClientLayer::onUpdate()
    ↓
Manually manage:
    - Image layout transitions
    - Rendering attachments
    - Command buffer recording
    - Descriptor set binding
    ↓
VKSwapchain::endFrame()
```

## Key Integration Points

### 1. **Frame Boundary Integration** (VKSwapchain)
- **Current**: `beginFrame()` and `endFrame()` bracket all rendering
- **Integration Point**: Render graph execution fits perfectly between these calls
- **Location**: `source/toaster/toast_gpu/vk/vk_swapchain.hpp` lines 17-18

**Before:**
```cpp
void beginFrame();
// Manual command buffer recording here
void endFrame();
```

**After:**
```cpp
void beginFrame();
m_renderGraph->execute(swapchain->getCurrentCommandBuffer(), frameIndex);
void endFrame();
```

---

### 2. **Command Buffer Access** (VKSwapchain)
- **Current**: Direct access via `swapchain->getCurrentCommandBuffer()` (line 123 in client_layer.cpp)
- **Problem**: You're manually managing command buffer state, layout transitions, and synchronization
- **Solution**: Render graph abstracts this away

**Integration:**
```cpp
// OLD: Manual management
auto &command_buffer = swapchain->getCurrentCommandBuffer();
command_buffer.reset();
// ... manually transition layouts, set up attachments ...
command_buffer.begin(begin_info);
// ... record commands ...

// NEW: Declarative via render graph
renderGraph.addPass("GeometryPass")
    .writes(swapchainColorTarget)
    .writes(swapchainDepthTarget)
    .execute([](auto& ctx) { /* render geometry */ });
```

---

### 3. **Resource Management** (ClientLayer)
- **Current**: Direct creation and lifetime management
  - `m_ubos` (line 37 in client_layer.hpp)
  - `m_geometryAttachmentImage` (line 33)
  - `m_descriptorSets` (line 47)
- **Integration Point**: These become render graph resources with automatic lifetime tracking

**What Changes:**
```cpp
// OLD: Manual lifetime
RefPtr<gpu::VKUniformBufferPFF> m_ubos;
std::vector<void *>             m_mappedUniformBuffers;

// NEW: Render graph owns them
RenderGraphResourceHandle m_cameraUBOHandle;
// Automatically managed, retrieved by reference during execution
```

---

### 4. **Attachment & Image Transitions** (ClientLayer::onUpdate)
- **Current**: Manual layout transitions (lines 135-160 in client_layer.cpp)
  ```cpp
  m_ctx->transitionImageLayout(command_buffer, 
      m_swapchainImages[m_imageIndex],
      vk::ImageLayout::eUndefined, 
      vk::ImageLayout::eColorAttachmentOptimal,
      ...);
  ```
- **Problem**: Prone to errors, hard to reorder passes
- **Solution**: Render graph tracks dependencies automatically

**Integration:**
```cpp
// OLD: Explicit transitions
command_buffer.transitionImageLayout(image, oldLayout, newLayout);
vk::RenderingAttachmentInfo colour_attachment_info{};
colour_attachment_info.imageView = swapchain->getImageView(image_index);

// NEW: Declarative
renderGraph.addPass("GeometryPass")
    .writes(swapchainColorImage)  // Automatically transitions to ColorAttachmentOptimal
    .reads(inputTexture)           // Automatically transitions to ShaderReadOnly
    .execute([](auto& pass) { /* auto transitions happen before execution */ });
```

---

### 5. **Pipeline & Shader Binding** (ClientLayer::onUpdate)
- **Current**: Manual pipeline binding (lines 181-182)
  ```cpp
  command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, 
                              m_geometryPipeline->getPipeline());
  command_buffer.setViewport(0, viewport);
  command_buffer.setScissor(0, scissor);
  command_buffer.bindDescriptorSets(...);
  ```
- **Integration**: Render graph can encapsulate this

**Integration:**
```cpp
// NEW: Cleaner abstraction
pass.setPipeline(m_geometryPipeline)
    .setViewport(viewport)
    .setScissor(scissor)
    .bindDescriptorSet(0, descriptorSet);
```

---

### 6. **Dependency Management** (Implicit in current code)
- **Current**: You manually ensure correct ordering
  - Geometry pass writes color attachment
  - Composite pass reads that color attachment (commented out)
- **Problem**: No automatic validation of dependencies
- **Solution**: Render graph validates all read/write relationships

**Integration:**
```cpp
// NEW: Graph validates this automatically
auto geometryPass = renderGraph.addPass("GeometryPass")
    .writes(colorAttachment);

auto compositePass = renderGraph.addPass("CompositePass")
    .reads(colorAttachment)  // Graph knows Geometry must run first
    .writes(swapchainImage);

renderGraph.compile();  // Validates: colorAttachment is written before read
```

---

## Implementation Strategy for You

### **Phase 1: Foundation (1-2 hours)**
Build the core abstractions without changing existing code.

**Files to Create:**
1. `vk_render_graph.hpp` - Core RenderGraph class and handles
2. `vk_render_pass.hpp` - RenderPass abstraction
3. `vk_render_graph_resource.hpp` - Resource wrapper

**What You'll Do:**
- Define `RenderGraphResourceHandle` (opaque handle to resources)
- Define `RenderPass` interface with builder pattern
- Define `RenderGraph` container
- NO changes to existing code yet

### **Phase 2: Resource Abstraction (1-2 hours)**
Create wrappers around existing resources.

**What You'll Do:**
- `RenderGraphResource` wraps `vk::raii::Image`, `vk::raii::Buffer`, etc.
- Track resource properties (format, size, layout, access masks)
- Implement `ResourceRegistry` for centralized management
- Still NO changes to ClientLayer

### **Phase 3: Graph Building API (2-3 hours)**
Implement the fluent builder pattern for declaring passes.

**What You'll Do:**
```cpp
RenderGraph graph(ctx);

auto geometryPassHandle = graph.addPass("GeometryPass")
    .writes(swapchainColorImage)
    .writes(swapchainDepthImage)
    .reads(meshTextures)
    .execute([this](RenderPassContext& passCtx) {
        // Your rendering logic here
    });
```

### **Phase 4: Compilation & Execution (2-3 hours)**
Build the runtime that actually executes the graph.

**What You'll Do:**
- Graph compiler validates dependencies
- Automatic layout transition insertion
- Execution engine that records command buffers
- Integration with VKSwapchain

### **Phase 5: ClientLayer Migration (1-2 hours)**
Refactor your current rendering to use the graph.

**What Changes:**
- Move current `onUpdate` rendering logic into graph passes
- Remove manual layout transitions (graph does it)
- Remove manual attachment setup (graph does it)
- Simpler, cleaner code

---

## Integration Points Summary Table

| Component | Current Pattern | Integration Point | After Migration |
|-----------|-----------------|-------------------|-----------------|
| **VKSwapchain** | beginFrame/endFrame bracket rendering | Execute graph between calls | `beginFrame(); graph.execute(); endFrame();` |
| **Command Buffer** | Manual access & recording | Pass to execution engine | Graph owns command buffer recording |
| **Image Layout** | Explicit transitions | Dependency tracking | Automatic based on read/write access |
| **Attachments** | Manual VkRenderingAttachmentInfo | Declared in pass definition | `pass.writes(attachment)` |
| **Resources** | Direct creation in ClientLayer | Render graph registry | `graph.createImage(...)` or resource handles |
| **Pipeline** | Manual bindPipeline calls | Encapsulated in pass | `pass.setPipeline(...)` |
| **Descriptors** | Manual binding | Pass-local context | `pass.bindDescriptorSet(...)` |
| **Sync** | Manual semaphores/fences | Graph-managed | Automatic via dependency resolution |

---

## Minimal Integration Path (Start Small)

If you want to start with **minimal disruption**:

### Step 1: Create render graph, DON'T change ClientLayer yet
- Just build the infrastructure alongside existing code

### Step 2: Create ONE test pass
- Move just the geometry rendering into a graph pass
- Keep everything else manual for now

### Step 3: Expand incrementally
- Add more passes as you become comfortable
- Eventually migrate all rendering

### Step 4: Refactor ClientLayer
- Once graph is solid, clean up ClientLayer

---

## Key Files to Review Before Starting

1. **`vk_swapchain.hpp` (lines 17-31)** - Frame boundary & command buffer management
2. **`client_layer.cpp` (lines 110-160)** - Current rendering logic to eventually migrate
3. **`vk_gpu_context.hpp` (lines 60-80)** - Image/buffer creation patterns to reuse
4. **`vk_render_attachment.hpp`** - Check if exists; understand attachment model

---

## Example: What Your Final Code Will Look Like

**Before (Current):**
```cpp
void ClientLayer::onUpdate(const float32 p_dt) {
    auto &command_buffer = swapchain->getCurrentCommandBuffer();
    
    // Manual setup
    command_buffer.reset();
    command_buffer.begin(begin_info);
    
    // Manual transitions
    m_ctx->transitionImageLayout(command_buffer, m_swapchainImages[m_imageIndex], 
        vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal, ...);
    
    // Manual attachment setup
    vk::RenderingAttachmentInfo colour_attachment_info{};
    colour_attachment_info.imageView = swapchain->getImageView(image_index);
    
    // Manual pipeline binding
    command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_geometryPipeline->getPipeline());
    
    // Rendering commands
    command_buffer.drawIndexed(...);
    command_buffer.endRendering();
}
```

**After (With Render Graph):**
```cpp
void ClientLayer::onInit() {
    m_renderGraph = std::make_unique<gpu::RenderGraph>(ctx);
    
    m_renderGraph->addPass("Geometry")
        .writes(swapchain->getSwapchainColorImage())
        .writes(swapchain->getSwapchainDepthImage())
        .reads(m_texture->getImage())
        .setPipeline(m_geometryPipeline)
        .setViewport(viewport)
        .setScissor(scissor)
        .execute([this](gpu::RenderPassContext& pass) {
            pass.bindDescriptorSet(0, m_descriptorSets[frameIndex]);
            pass.drawIndexed(m_mesh->getIndices().size(), 1, 0, 0, 0);
        });
    
    m_renderGraph->compile();
}

void ClientLayer::onUpdate(const float32 p_dt) {
    m_time += p_dt;
    // Just update data; graph handles rendering
    updateCameraUBO(m_time);
    
    auto swapchain = app.getWindow().getSwapchain();
    uint32 frameIndex = swapchain->getFrameIndex();
    
    m_renderGraph->execute(frameIndex);  // Everything else is automatic!
}
```

Much simpler and safer!

---

## Next Steps

1. **Read this guide completely** to understand integration points
2. **Review the code paths** mentioned above in your actual codebase
3. **Start Phase 1** creating core abstractions
4. **Test incrementally** before changing existing code
5. **Migrate ClientLayer gradually** pass by pass

Good luck! This will significantly improve your rendering architecture.
