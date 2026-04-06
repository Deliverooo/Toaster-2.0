# Architecture Diagram: Current vs. Render Graph

## Current Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                        ClientLayer::onUpdate()                  │
│                                                                   │
│  1. Get swapchain context                                        │
│  2. Get command buffer: swapchain->getCurrentCommandBuffer()     │
│  3. Manually:                                                    │
│     - Transition image layouts                                   │
│     - Create attachment infos                                   │
│     - Begin rendering                                           │
│     - Bind pipeline                                             │
│     - Bind descriptors                                          │
│     - Draw calls                                                │
│     - End rendering                                             │
│  4. Return to VKSwapchain::endFrame()                           │
└─────────────────────────────────────────────────────────────────┘
         ↑
         └─ Everything is manual and error-prone
         └─ Hard to reorder/optimize passes
         └─ Difficult to add new render targets
         └─ Layout transition bugs are common
```

### Data Flow Currently

```
ClientLayer                VKGPUContext            VKSwapchain
    │                            │                     │
    ├─ beginFrame()──────────────┼─────────────────────►
    │                            │
    ├─ onUpdate()               │
    │  ├─ getCommandBuffer()────┼──────────────────────►
    │  │                        │
    │  ├─ transitionImageLayout()
    │  │  └─ (Manual calls)─────►
    │  │
    │  ├─ beginRendering()
    │  ├─ bindPipeline()
    │  ├─ bindDescriptors()
    │  ├─ drawIndexed()
    │  └─ endRendering()
    │
    └─ endFrame()──────────────┼─────────────────────►
```

---

## With Render Graph

```
┌─────────────────────────────────────────────────────────────────┐
│                    ClientLayer::onInit()                        │
│                                                                   │
│  m_renderGraph = RenderGraph(ctx)                               │
│                                                                   │
│  m_renderGraph.addPass("Geometry")                              │
│      .writes(swapchainColor)       ─┐                           │
│      .writes(swapchainDepth)       ─┤─ Declarative             │
│      .reads(meshTexture)           ─┤  Dependencies            │
│      .setPipeline(geometryPipe)    ─┤  Auto-Managed            │
│      .execute([](ctx) { ... })     ─┘                          │
│                                                                   │
│  m_renderGraph.compile()           ─ Validates everything       │
└─────────────────────────────────────────────────────────────────┘
         ↓
         Compilation Phase: Graph validates & optimizes
         ↓
┌─────────────────────────────────────────────────────────────────┐
│                   ClientLayer::onUpdate()                       │
│                                                                   │
│  updateCameraUBO(m_time)                                        │
│  m_renderGraph.execute(frameIndex)  ─ Execute all passes        │
│                                       ─ Auto layout transitions  │
│                                       ─ Auto synchronization     │
└─────────────────────────────────────────────────────────────────┘
```

### Data Flow With Render Graph

```
                        ┌─── Compilation Phase (onInit) ───┐
                        │                                    │
RenderGraph Builder     RenderGraph Compiler                │
    │                           │                           │
    ├─ addPass()────────►  Graph Validates                 │
    │  ├─ writes()         Dependencies                    │
    │  ├─ reads()          Layouts                         │
    │  └─ execute()        Ordering                        │
    │                           │                          │
    └─ compile()────────────────┘                          │
                        │
                        └────────────────────────────┐
                                                     │
                         ┌─── Execution Phase (onUpdate) ───┐
                         │                                    │
VKSwapchain             RenderGraph Executor              ClientLayer
    │                           │                           │
    ├─ beginFrame()─────────────►                          │
    │                           │                          │
    │ ◄─ execute()──────────────┼─────────────────────────┤
    │   - Record command buffer │                         │
    │   - Transition layouts    │                         │
    │   - Execute passes        │                         │
    │                           │                         │
    ├─ endFrame()──────────────►                          │
    │   - Submit to GPU         │                         │
    │   - Present               │                         │
```

---

## Integration Point Detailed View

### Current: Manual Control (Tight Coupling)

```
ClientLayer                          GPU Memory            Swapchain
    │                                   │                     │
    ├─ Command Buffer ───────────────────┼─────────────────────►
    │  └─ reset()                        │
    │  └─ begin()                        │
    │      │                             │
    │      └─ transitionImageLayout() ────► Image: Undefined
    │                                       ├─ Barrier
    │                                       └─ ColorAttachmentOptimal
    │
    │  └─ transitionImageLayout()        ──► Depth: Undefined
    │                                       ├─ Barrier
    │                                       └─ DepthAttachmentOptimal
    │
    │  └─ beginRendering()
    │  └─ bindPipeline()
    │  └─ setViewport()
    │  └─ setScissor()
    │  └─ bindDescriptorSets()
    │  └─ drawIndexed()
    │  └─ endRendering()
    │
    │  └─ end()
    │
    └─ Returns to endFrame()
       └─ Submit to graphics queue
```

**Problems:**
- You manage every detail
- Easy to miss layout transitions
- Hard to add new passes
- Difficult to optimize ordering
- Manual synchronization

### New: Declarative (Loose Coupling)

```
RenderGraph ◄─ Declarations ─ ClientLayer
    │
    ├─ Pass "Geometry"
    │  ├─ writes: SwapchainColor
    │  ├─ writes: SwapchainDepth
    │  ├─ reads: MeshTextures
    │  └─ execute: [user callback]
    │
    └─ Compiler
       ├─ Validates dependencies
       ├─ Determines optimal order
       ├─ Inserts layout transitions
       └─ Generates execution plan
           │
           └─ Execution Engine
              ├─ beginFrame() from swapchain
              │
              ├─ For each pass in order:
              │  ├─ Auto-transition read images
              │  ├─ Auto-transition write targets
              │  ├─ Record command buffer
              │  ├─ Execute user callback
              │  └─ Track image state
              │
              ├─ endFrame() to swapchain
              │
              └─ Return execution stats
```

**Benefits:**
- Graph tracks everything
- Auto layout transitions
- Easy to add/remove passes
- Automatic synchronization
- Reorderable passes
- Performance analysis built-in

---

## Integration Timeline

```
Week 1: Infrastructure
├─ vk_render_graph_resource.hpp  [Define handles & resource types]
├─ vk_render_pass.hpp            [Pass abstraction & builder]
├─ vk_render_graph.hpp           [Graph container]
└─ Tests compile, NO functional changes yet

Week 2: Runtime Foundation
├─ ResourceRegistry implementation
├─ PassBuilder implementation
├─ Graph compilation logic
└─ Can create & compile graphs, still no rendering

Week 3: Execution Engine
├─ Command buffer recording
├─ Layout transition generation
├─ Dependency resolver
├─ Pass executor
└─ First test: Simple geometry pass works

Week 4: Integration & Migration
├─ Connect to VKSwapchain
├─ Migrate ClientLayer renders
├─ Performance testing
└─ Optimize resource pooling
```

---

## Key Classes You'll Create

```
┌─────────────────────────────────────────────────────────────────┐
│                    RenderGraphResource                          │
│                                                                   │
│  - handle: ResourceHandle                                       │
│  - type: ResourceType (Image, Buffer)                          │
│  - format: vk::Format                                          │
│  - extent: vk::Extent3D                                        │
│  - layout: vk::ImageLayout (current)                           │
│  - accessMask: vk::AccessFlags2 (current)                      │
│  - data: void* (vk::raii::Image, vk::raii::Buffer, etc.)      │
│                                                                   │
│  Usage: Wraps actual Vulkan resources with metadata            │
└─────────────────────────────────────────────────────────────────┘
                         ↑
                         │ (owns/manages)
                         │
┌─────────────────────────────────────────────────────────────────┐
│                      ResourceRegistry                           │
│                                                                   │
│  - resources: map<ResourceHandle, RenderGraphResource>         │
│  - nextHandle: uint32                                          │
│                                                                   │
│  + createImage(): ResourceHandle                               │
│  + createBuffer(): ResourceHandle                              │
│  + getResource(handle): RenderGraphResource*                  │
│  + updateLayout(handle, newLayout)                             │
│                                                                   │
│  Usage: Central registry for all graph resources               │
└─────────────────────────────────────────────────────────────────┘
                         ↑
                         │ (uses)
                         │
┌─────────────────────────────────────────────────────────────────┐
│                      RenderPass                                 │
│                                                                   │
│  - name: std::string                                           │
│  - reads: vector<ResourceHandle>                               │
│  - writes: vector<ResourceHandle>                              │
│  - pipeline: RefPtr<VKPipeline>                                │
│  - executeCallback: function<void(PassContext&)>               │
│                                                                   │
│  Usage: Single node in the render graph                        │
└─────────────────────────────────────────────────────────────────┘
              ↑           ↑           ↑
              │           │           │
              │           └─ (builds) ─ RenderPassBuilder
              │                       (fluent API)
              │
              └─ (contains multiple)
                         │
                         ↓
┌─────────────────────────────────────────────────────────────────┐
│                      RenderGraph                                │
│                                                                   │
│  - passes: vector<RenderPass>                                  │
│  - resources: ResourceRegistry                                 │
│  - compiled: bool                                              │
│  - executionPlan: vector<PassExecutionInfo>                    │
│                                                                   │
│  + addPass(name): RenderPassBuilder                            │
│  + compile(): void                                             │
│  + execute(frameIndex): void                                   │
│                                                                   │
│  Usage: Main interface for declaring and executing rendering   │
└─────────────────────────────────────────────────────────────────┘
```

---

## How to Read This Guide

1. **Current Architecture** - Understand what you do now (manual)
2. **Integration Points** - See exactly where graph hooks in
3. **Implementation Strategy** - Plan your work in phases
4. **Key Classes** - Know what you'll build
5. **Example Code** - See before/after side-by-side

**For implementation details**, refer to `RENDER_GRAPH_INTEGRATION_GUIDE.md`.
