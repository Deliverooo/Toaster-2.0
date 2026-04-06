# Render Graph - Quick Reference Card

## Class Hierarchy

```
┌─────────────────────────────┐
│     RenderGraphResource     │
│  (wraps vk::Image, etc.)    │
└──────────────┬──────────────┘
               │ many
               ↓
┌─────────────────────────────┐
│    ResourceRegistry         │
│  (lookup by handle)         │
└──────────────┬──────────────┘
               │ owns
               ↓
┌─────────────────────────────┐
│      RenderGraph            │
│  (main entry point)         │
└─────────────────────────────┘
               ↓ contains many
        ┌──────────────────────┐
        │    RenderPass        │
        │  (single task)       │
        └──────────────────────┘
               ↑ built by
        ┌──────────────────────┐
        │ RenderPassBuilder    │
        │  (fluent API)        │
        └──────────────────────┘
               │ uses
               ↓
┌─────────────────────────────┐
│  RenderPassContext          │
│  (execution context)        │
└─────────────────────────────┘
```

---

## API Summary

### Creating a Graph
```cpp
auto graph = std::make_unique<gpu::RenderGraph>(ctx);

// Register external resources (e.g., swapchain)
auto color = graph->registerExternalImage(
    vkImage, vkImageView, vk::Format::eR8G8B8A8Srgb, width, height,
    "SwapchainColor");

// Create internal resources
auto temp = graph->createImage(
    vk::Format::eR32Sfloat, width, height, "TempBuffer");
```

### Declaring Passes
```cpp
graph->addPass("GeometryPass")
    .writes(color)
    .writes(depth)
    .reads(meshTexture)
    .execute([this](gpu::RenderPassContext& ctx) {
        auto& cmd = ctx.getCommandBuffer();
        // Your rendering code here
        cmd.drawIndexed(...);
    });

graph->addPass("CompositePass")
    .reads(color)
    .reads(depth)
    .writes(swapchainColor)
    .execute([this](gpu::RenderPassContext& ctx) {
        // Composite the outputs
    });
```

### Executing the Graph
```cpp
graph->compile();  // Validate & optimize (call once)

// Per-frame
graph->execute(swapchain->getFrameIndex());
```

---

## Type Reference

### RenderGraphResourceHandle
```cpp
// Opaque handle to a resource
using RenderGraphResourceHandle = uint32_t;

// Usage
RenderGraphResourceHandle handle = graph->createImage(...);
auto* resource = registry.getResource(handle);
```

### ERenderGraphResourceType
```cpp
enum class ERenderGraphResourceType {
    eImage,    // Render target, texture, attachment
    eBuffer,   // Vertex, index, uniform buffer
    eExternal, // Swapchain image (unowned)
};
```

### EResourceAccess
```cpp
enum class EResourceAccess {
    eRead,     // Used in shader
    eWrite,    // Rendered to / written by compute
};
```

### ResourceAccessInfo
```cpp
struct ResourceAccessInfo {
    RenderGraphResourceHandle handle;
    EResourceAccess access;
    vk::AccessFlags2 accessMask;           // Vulkan access flags
    vk::ImageLayout imageLayout;            // Expected layout
};
```

---

## Method Reference

### RenderGraph
```cpp
// Building
RenderPassBuilder& addPass(const char* name);
RenderGraphResourceHandle createImage(vk::Format, uint32, uint32, const char*);
RenderGraphResourceHandle registerExternalImage(vk::Image, vk::ImageView, ...);

// Execution
void compile();              // Validate & optimize
void execute(uint32 frame);  // Execute all passes

// Query
bool isCompiled() const;
size_t getPassCount() const;
```

### RenderPassBuilder
```cpp
// Fluent API - all return *this for chaining
RenderPassBuilder& reads(RenderGraphResourceHandle, vk::AccessFlags2 = eShaderRead);
RenderPassBuilder& writes(RenderGraphResourceHandle, vk::AccessFlags2 = eColorAttachmentWrite);
RenderPassBuilder& execute(std::function<void(RenderPassContext&)>);

RenderPass& build();  // Returns the built pass
```

### RenderPassContext
```cpp
// Access during pass execution
vk::raii::CommandBuffer& getCommandBuffer();

// Resource access
RenderGraphResource* getResource(RenderGraphResourceHandle);
template<typename T> T* getResourceData(RenderGraphResourceHandle);
vk::ImageLayout getResourceLayout(RenderGraphResourceHandle);

// Pass metadata
uint32 getFrameIndex() const;
const char* getPassName() const;
```

### RenderGraphResource
```cpp
// Metadata
RenderGraphResourceHandle getHandle() const;
ERenderGraphResourceType getType() const;
vk::Format getFormat() const;
vk::Extent3D getExtent() const;

// State
vk::ImageLayout getCurrentLayout() const;
vk::AccessFlags2 getCurrentAccessMask() const;
void setCurrentLayout(vk::ImageLayout);
void setCurrentAccessMask(vk::AccessFlags2);

// Data storage
template<typename T> void setData(T* data);
template<typename T> T* getData() const;
void* getRawData() const;
```

---

## Common Patterns

### Multiple Render Targets
```cpp
auto color = graph->createImage(vk::Format::eR8G8B8A8Srgb, w, h, "Color");
auto normal = graph->createImage(vk::Format::eR8G8B8A8Snorm, w, h, "Normal");
auto position = graph->createImage(vk::Format::eR32G32B32A32Sfloat, w, h, "Position");

graph->addPass("GBuffer")
    .writes(color)
    .writes(normal)
    .writes(position)
    .reads(meshTexture)
    .execute([...](auto& ctx) {
        // Render to all three targets
    });
```

### Post-Processing Chain
```cpp
auto hdrBuffer = graph->createImage(..., "HDR");
auto bloomBuffer = graph->createImage(..., "Bloom");
auto finalImage = graph->createImage(..., "Final");

graph->addPass("Lighting")
    .writes(hdrBuffer)
    .reads(gBuffer)
    .execute([...](auto& ctx) { /* ... */ });

graph->addPass("Bloom")
    .reads(hdrBuffer)
    .writes(bloomBuffer)
    .execute([...](auto& ctx) { /* ... */ });

graph->addPass("Composite")
    .reads(hdrBuffer)
    .reads(bloomBuffer)
    .writes(finalImage)
    .execute([...](auto& ctx) { /* ... */ });
```

### Resource Reuse
```cpp
// Create a temp buffer used by multiple passes
auto temp = graph->createImage(..., "TempBuffer");

graph->addPass("Pass1")
    .writes(temp)
    .execute([...](auto& ctx) { /* Write to temp */ });

graph->addPass("Pass2")
    .reads(temp)
    .writes(output)
    .execute([...](auto& ctx) { /* Read from temp */ });
```

### External Resources (Swapchain)
```cpp
// Register swapchain images as external (graph doesn't own them)
auto swapColor = graph->registerExternalImage(
    swapchain->getImage(0),
    swapchain->getImageView(0),
    swapchain->getSurfaceFormat().format,
    swapchain->getExtent().width,
    swapchain->getExtent().height,
    "SwapchainColor");

auto swapDepth = graph->registerExternalImage(
    swapchain->getDepthImage(),
    swapchain->getDepthImageView(),
    swapchain->getDepthFormat(),
    swapchain->getExtent().width,
    swapchain->getExtent().height,
    "SwapchainDepth");

// Use like normal resources
graph->addPass("Geometry")
    .writes(swapColor)
    .writes(swapDepth)
    .execute([...](auto& ctx) { /* Render */ });
```

---

## Execution Model

### Phase 1: Build (onInit)
```
graph.addPass(...).execute([...]);
graph.addPass(...).execute([...]);
graph.compile();  ← Validates, optimizes, determines order
```

### Phase 2: Execute (onUpdate)
```
graph.execute(frameIndex)
  ├─ For each pass in order:
  │  ├─ Transition resources to needed layouts
  │  ├─ Create RenderPassContext
  │  └─ Call user's execute callback
  └─ Return (all synchronization handled)
```

---

## Data Flow

```
                Creation Phase
                      │
        ┌─────────────┴─────────────┐
        ↓                           ↓
   RenderGraphResource      RenderPassBuilder
        │                           │
        ├─ format                   ├─ reads(...)
        ├─ extent                   ├─ writes(...)
        ├─ layout                   └─ execute(callback)
        └─ accessMask                       │
                                            ↓
                                      RenderPass
                                            │
        ┌───────────────────────────────────┤
        │                                   │
   ResourceRegistry              Compilation Phase
        │                                   │
        └────────► RenderGraph ◄────────────┤
                        │
                        ├─ Validate dependencies
                        ├─ Topological sort
                        ├─ Plan transitions
                        └─ Execution order
                               │
                               ↓
                        Execution Phase
                               │
                        RenderPassContext
                               │
                        User callback
                               │
                        Command buffer
```

---

## Command Buffer Integration

### What the graph handles
```cpp
// User writes one callback per pass:
.execute([](auto& ctx) {
    auto& cmd = ctx.getCommandBuffer();
    
    // Just record commands
    cmd.bindPipeline(...);
    cmd.bindDescriptorSets(...);
    cmd.drawIndexed(...);
    
    // Don't handle:
    // - Layout transitions (graph does it)
    // - beginFrame/endFrame (swapchain does it)
    // - Synchronization (graph handles it)
})
```

### What you still manage
```cpp
// In your pass callback:
// ✓ Pipeline binding
// ✓ Descriptor sets
// ✓ Push constants
// ✓ Draw calls
// ✓ Viewport/scissor

// ✗ Layout transitions (graph handles)
// ✗ Attachment setup (pass declares via writes())
// ✗ Synchronization (graph handles)
// ✗ Frame boundaries (swapchain handles)
```

---

## Debugging Tips

### Enable Logging
```cpp
// In vk_render_graph.cpp execute():
LOG_INFO("Executing graph with {} passes", m_executionOrder.size());

for (size_t idx : m_executionOrder) {
    LOG_TRACE("Executing pass: {}", m_passes[idx].getName());
}
```

### Validate Compilation
```cpp
graph->compile();  // Will log errors if invalid

if (!graph->isCompiled()) {
    LOG_ERROR("Graph failed to compile!");
    return;
}
```

### Check Execution Order
```cpp
for (size_t i = 0; i < graph->getPassCount(); ++i) {
    LOG_INFO("Pass {}: {}", i, m_passes[m_executionOrder[i]].getName());
}
```

### Verify Resource States
```cpp
// In RenderPassContext::execute():
LOG_TRACE("Pass '{}' resources:", getPassName());
for (const auto& access : pass->getReads()) {
    auto* res = getResource(access.handle);
    LOG_TRACE("  reads: layout={}", vk::to_string(res->getCurrentLayout()));
}
for (const auto& access : pass->getWrites()) {
    auto* res = getResource(access.handle);
    LOG_TRACE("  writes: layout={}", vk::to_string(res->getCurrentLayout()));
}
```

---

## Dependencies Map

```
Phase 1: Types & Resources
├─ vk_render_graph_types.hpp
├─ vk_render_graph_resource.hpp
├─ vk_render_graph_resource_registry.hpp
└─ Depends: None

Phase 2: Passes
├─ vk_render_pass.hpp
├─ vk_render_pass_builder.hpp
├─ vk_render_pass_context.hpp
└─ Depends: Phase 1

Phase 3: Graph
├─ vk_render_graph.hpp
└─ Depends: Phase 1, Phase 2

Integration
├─ Modify: client_layer.hpp/cpp
├─ Modify: vk_swapchain.hpp/cpp (optional)
└─ Depends: Phase 1, 2, 3
```

---

## Access Flags Quick Reference

### Common Patterns
```cpp
// Writing to render target
.writes(target, vk::AccessFlagBits2::eColorAttachmentWrite)

// Reading from texture
.reads(texture, vk::AccessFlagBits2::eShaderRead)

// Depth/stencil
.writes(depth, vk::AccessFlagBits2::eDepthStencilAttachmentWrite)
.reads(depth, vk::AccessFlagBits2::eDepthStencilAttachmentRead)

// Compute
.writes(buffer, vk::AccessFlagBits2::eShaderStorageWrite)
.reads(buffer, vk::AccessFlagBits2::eShaderStorageRead)

// Transfer
.writes(target, vk::AccessFlagBits2::eTransferWrite)
.reads(source, vk::AccessFlagBits2::eTransferRead)
```

---

## Layout Quick Reference

```cpp
// Common layouts
vk::ImageLayout::eColorAttachmentOptimal      // For color writes
vk::ImageLayout::eDepthAttachmentOptimal      // For depth writes
vk::ImageLayout::eShaderReadOnlyOptimal       // For texture reads
vk::ImageLayout::eTransferDstOptimal          // For copy targets
vk::ImageLayout::eTransferSrcOptimal          // For copy sources
vk::ImageLayout::ePresentSrcKHR               // Before present
vk::ImageLayout::eUndefined                   // Uninitialized
```

---

## Testing Checklist

```
Phase 1
□ Types compile
□ RenderGraphResource instantiates
□ ResourceRegistry stores/retrieves

Phase 2  
□ RenderPass created
□ Builder chaining works
□ Callbacks stored

Phase 3
□ Graph created
□ Passes added
□ Graph compiles without errors
□ Passes execute in order

Phase 4
□ Swapchain integration works
□ ClientLayer uses graph
□ No breaking changes

Phase 5
□ One pass renders correctly
□ Output matches original
□ Layout transitions recorded

Phase 6
□ Code is clean
□ Documented
□ Ready for production
```

---

Keep this as a reference while implementing!
