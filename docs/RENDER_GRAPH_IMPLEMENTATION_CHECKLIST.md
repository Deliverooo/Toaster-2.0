# Render Graph Implementation Checklist

## Phase 1: Core Abstractions & Types (2 hours, ~300 LOC)

### Step 1.1: Create Resource Handle System
**File:** `source/toaster/toast_gpu/vk/vk_render_graph_types.hpp`

**What to implement:**
```cpp
namespace toaster::gpu {

// Opaque handle to a render graph resource
using RenderGraphResourceHandle = uint32;
constexpr RenderGraphResourceHandle InvalidResourceHandle = UINT32_MAX;

// Resource type enumeration
enum class ERenderGraphResourceType : uint8 {
    eImage,    // vk::raii::Image
    eBuffer,   // vk::raii::Buffer
    eExternal, // External resource (e.g., swapchain image)
};

// Resource access intent
enum class EResourceAccess : uint8 {
    eRead,
    eWrite,
};

// Paired read/write with a pass
struct ResourceAccessInfo {
    RenderGraphResourceHandle handle;
    EResourceAccess access;
    vk::AccessFlags2 accessMask;
    vk::ImageLayout imageLayout;
};

}
```

**Checklist:**
- [ ] Create types header file
- [ ] Define all opaque types
- [ ] Compile without errors
- [ ] NO functionality yet, just types

---

### Step 1.2: Create RenderGraphResource Wrapper
**File:** `source/toaster/toast_gpu/vk/vk_render_graph_resource.hpp`

**What to implement:**
```cpp
class RenderGraphResource {
public:
    // Construction
    RenderGraphResource(RenderGraphResourceHandle handle, 
                       ERenderGraphResourceType type,
                       vk::Format format = vk::Format::eUndefined);

    // Accessors
    [[nodiscard]] RenderGraphResourceHandle getHandle() const;
    [[nodiscard]] ERenderGraphResourceType getType() const;
    [[nodiscard]] vk::Format getFormat() const;
    [[nodiscard]] vk::Extent3D getExtent() const;
    
    // State tracking
    [[nodiscard]] vk::ImageLayout getCurrentLayout() const;
    [[nodiscard]] vk::AccessFlags2 getCurrentAccessMask() const;
    
    void setCurrentLayout(vk::ImageLayout layout);
    void setCurrentAccessMask(vk::AccessFlags2 mask);
    
    // Data storage (holds reference to actual Vulkan resource)
    template<typename T>
    void setData(T* data);
    
    template<typename T>
    [[nodiscard]] T* getData();

private:
    RenderGraphResourceHandle m_handle;
    ERenderGraphResourceType m_type;
    vk::Format m_format;
    vk::Extent3D m_extent;
    vk::ImageLayout m_currentLayout{vk::ImageLayout::eUndefined};
    vk::AccessFlags2 m_currentAccessMask{vk::AccessFlagBits2::eNone};
    void* m_data{nullptr};
};
```

**Checklist:**
- [ ] Create resource wrapper header
- [ ] Implement basic accessors
- [ ] Add layout/access tracking
- [ ] Support generic data storage
- [ ] Compile without errors

---

### Step 1.3: Create ResourceRegistry
**File:** `source/toaster/toast_gpu/vk/vk_render_graph_resource_registry.hpp`

**What to implement:**
```cpp
class ResourceRegistry {
public:
    explicit ResourceRegistry(VKGPUContext* ctx);

    // Registration
    RenderGraphResourceHandle registerImage(vk::Format format, 
                                           vk::Extent3D extent,
                                           const char* debugName = "");
    
    RenderGraphResourceHandle registerExternalImage(vk::Image image,
                                                   vk::ImageView imageView,
                                                   vk::Format format,
                                                   vk::Extent3D extent,
                                                   const char* debugName = "");

    // Lookup
    [[nodiscard]] RenderGraphResource* getResource(RenderGraphResourceHandle handle);
    [[nodiscard]] const RenderGraphResource* getResource(RenderGraphResourceHandle handle) const;

    // Lifecycle
    void clear();

private:
    VKGPUContext* m_ctx;
    std::unordered_map<RenderGraphResourceHandle, RenderGraphResource> m_resources;
    RenderGraphResourceHandle m_nextHandle{0};
};
```

**Checklist:**
- [ ] Create registry header
- [ ] Implement handle generation
- [ ] Implement resource lookup
- [ ] Add debug names for validation
- [ ] Compile without errors

---

## Phase 2: Pass Definition System (3 hours, ~400 LOC)

### Step 2.1: Create RenderPassContext
**File:** `source/toaster/toast_gpu/vk/vk_render_pass_context.hpp`

**What to implement:**
```cpp
// Forward declaration
class RenderPass;

// Execution context passed to pass callback
class RenderPassContext {
public:
    RenderPassContext(const RenderPass* pass,
                     vk::raii::CommandBuffer& cmdBuffer,
                     ResourceRegistry& registry,
                     uint32 frameIndex);

    // Command buffer recording
    [[nodiscard]] vk::raii::CommandBuffer& getCommandBuffer();

    // Resource access during pass execution
    template<typename T>
    [[nodiscard]] T* getResource(RenderGraphResourceHandle handle);

    // Image layout getter
    [[nodiscard]] vk::ImageLayout getResourceLayout(RenderGraphResourceHandle handle) const;

    // Pass metadata
    [[nodiscard]] uint32 getFrameIndex() const;
    [[nodiscard]] const char* getPassName() const;

private:
    const RenderPass* m_pass;
    vk::raii::CommandBuffer& m_cmdBuffer;
    ResourceRegistry& m_registry;
    uint32 m_frameIndex;
};
```

**Checklist:**
- [ ] Create context header
- [ ] Implement command buffer access
- [ ] Implement resource retrieval
- [ ] Add frame index tracking
- [ ] Compile without errors

---

### Step 2.2: Create RenderPass Definition
**File:** `source/toaster/toast_gpu/vk/vk_render_pass.hpp`

**What to implement:**
```cpp
class RenderPass {
public:
    using ExecuteCallback = std::function<void(RenderPassContext&)>;

    explicit RenderPass(const char* name);

    // Metadata
    [[nodiscard]] const char* getName() const;
    [[nodiscard]] const std::vector<ResourceAccessInfo>& getReads() const;
    [[nodiscard]] const std::vector<ResourceAccessInfo>& getWrites() const;

    // Builder-set properties
    void setExecuteCallback(ExecuteCallback callback);
    
    // Execution
    void execute(RenderPassContext& ctx) const;

private:
    std::string m_name;
    std::vector<ResourceAccessInfo> m_reads;
    std::vector<ResourceAccessInfo> m_writes;
    ExecuteCallback m_executeCallback;

    friend class RenderPassBuilder;
};
```

**Checklist:**
- [ ] Create pass header
- [ ] Store pass metadata
- [ ] Implement execute method
- [ ] Add friend access for builder
- [ ] Compile without errors

---

### Step 2.3: Create RenderPassBuilder (Fluent API)
**File:** `source/toaster/toast_gpu/vk/vk_render_pass_builder.hpp`

**What to implement:**
```cpp
class RenderPassBuilder {
public:
    explicit RenderPassBuilder(RenderPass& pass,
                              ResourceRegistry& registry);

    // Fluent API for declaring resource access
    RenderPassBuilder& reads(RenderGraphResourceHandle handle,
                            vk::AccessFlags2 accessMask = vk::AccessFlagBits2::eShaderRead);
    
    RenderPassBuilder& writes(RenderGraphResourceHandle handle,
                             vk::AccessFlags2 accessMask = vk::AccessFlagBits2::eColorAttachmentWrite);

    // Set execution callback
    RenderPassBuilder& execute(std::function<void(RenderPassContext&)> callback);

    // Convert builder to actual RenderPass
    [[nodiscard]] RenderPass& build();

private:
    RenderPass& m_pass;
    ResourceRegistry& m_registry;
};
```

**Checklist:**
- [ ] Create builder header
- [ ] Implement fluent methods
- [ ] Track resource accesses
- [ ] Return *this for chaining
- [ ] Compile without errors

---

## Phase 3: Graph Container (2 hours, ~350 LOC)

### Step 3.1: Create RenderGraph Main Class
**File:** `source/toaster/toast_gpu/vk/vk_render_graph.hpp`

**What to implement:**
```cpp
class RenderGraph {
public:
    explicit RenderGraph(VKGPUContext* ctx);
    ~RenderGraph();

    // Graph building
    RenderPassBuilder& addPass(const char* passName);

    // Resource management
    RenderGraphResourceHandle createImage(vk::Format format,
                                         uint32 width,
                                         uint32 height,
                                         const char* debugName = "");
    
    RenderGraphResourceHandle registerExternalImage(vk::Image image,
                                                   vk::ImageView imageView,
                                                   vk::Format format,
                                                   uint32 width,
                                                   uint32 height,
                                                   const char* debugName = "");

    // Compilation & execution
    void compile();
    void execute(uint32 frameIndex);

    // Debug
    [[nodiscard]] bool isCompiled() const { return m_compiled; }
    [[nodiscard]] size_t getPassCount() const { return m_passes.size(); }

private:
    VKGPUContext* m_ctx;
    ResourceRegistry m_resources;
    std::vector<RenderPass> m_passes;
    std::vector<size_t> m_executionOrder; // Optimized pass order
    bool m_compiled{false};

    void _validateDependencies();
    void _buildExecutionOrder();
};
```

**Checklist:**
- [ ] Create graph header
- [ ] Implement pass addition
- [ ] Implement resource creation
- [ ] Add compile/execute stubs
- [ ] Compile without errors
- [ ] DON'T implement logic yet, just structure

---

### Step 3.2: Implement Graph Compilation
**File:** `source/toaster/toast_gpu/vk/vk_render_graph.cpp` (part 1)

**What to implement:**
```cpp
void RenderGraph::compile() {
    // Validation: Check all writes come before reads
    // Ordering: Topological sort of dependency graph
    // Layout planning: Determine optimal transitions
    
    _validateDependencies();  // Error if circular, missing writes, etc.
    _buildExecutionOrder();   // Topological sort
    
    m_compiled = true;
}

void RenderGraph::_validateDependencies() {
    // For each pass:
    //   For each resource it reads:
    //     Ensure some earlier pass writes it (or it's external)
    //     Set expected input layout
    //   For each resource it writes:
    //     Ensure no other pass also writes it
    //     Record this as producer for future reads
}

void RenderGraph::_buildExecutionOrder() {
    // Topological sort: order passes by dependencies
    // Example: 
    //   If Composite reads from Geometry's output,
    //   Geometry must come before Composite
}
```

**Checklist:**
- [ ] Implement _validateDependencies()
- [ ] Implement _buildExecutionOrder()
- [ ] Add error messages for validation failures
- [ ] Test with simple graphs (compile, check order)
- [ ] Compile and run basic tests

---

### Step 3.3: Implement Graph Execution
**File:** `source/toaster/toast_gpu/vk/vk_render_graph.cpp` (part 2)

**What to implement:**
```cpp
void RenderGraph::execute(uint32 frameIndex) {
    TST_ASSERT(m_compiled && "Graph must be compiled before execution");
    
    // For each pass in execution order:
    for (size_t passIdx : m_executionOrder) {
        RenderPass& pass = m_passes[passIdx];
        
        // Create context
        // TODO: Get command buffer from swapchain
        // RenderPassContext ctx(&pass, cmdBuffer, m_resources, frameIndex);
        
        // Execute the pass
        // pass.execute(ctx);
    }
}
```

**Checklist:**
- [ ] Implement execute loop
- [ ] Create RenderPassContext per pass
- [ ] Call pass.execute()
- [ ] Handle errors gracefully
- [ ] Compile without errors

---

## Phase 4: Integration with VKSwapchain (2 hours, ~200 LOC)

### Step 4.1: Add Graph Execution to VKSwapchain
**File:** `source/toaster/toast_gpu/vk/vk_swapchain.hpp` (modify)

**What to add:**
```cpp
class VKSwapchain {
    // ... existing code ...

    // New: Support for render graph execution
    vk::raii::CommandBuffer& getCommandBufferForRecording(uint32 frameIndex);
    
    // Graph can call this to get the correct command buffer
    // during execution phase
};
```

**Checklist:**
- [ ] Decide: Does swapchain manage command buffers, or pass them to graph?
- [ ] Option A: Graph gets owned command buffer from swapchain
- [ ] Option B: Graph records to swapchain's pre-allocated buffers
- [ ] Implement chosen approach
- [ ] Keep backward compatible (don't break existing code)

---

### Step 4.2: Add Render Graph to ClientLayer
**File:** `client/source/client_layer.hpp` (modify)

**What to add:**
```cpp
class ClientLayer final : public IAppLayer {
    // ... existing code ...

private:
    std::unique_ptr<gpu::RenderGraph> m_renderGraph;
    
    // Graph resource handles (instead of direct Vulkan objects)
    gpu::RenderGraphResourceHandle m_swapchainColorHandle;
    gpu::RenderGraphResourceHandle m_swapchainDepthHandle;
    // ... etc ...
};
```

**Checklist:**
- [ ] Add m_renderGraph member
- [ ] Add resource handles for key targets
- [ ] Keep old members for now (migration gradual)
- [ ] Compile without errors

---

## Phase 5: Test & Migrate (3 hours, ~500 LOC)

### Step 5.1: Create Simple Test Pass
**File:** `client/source/client_layer.cpp` (modify onInit)

**What to do:**
```cpp
void ClientLayer::onInit() {
    // ... existing setup code ...
    
    // NEW: Initialize render graph
    m_renderGraph = std::make_unique<gpu::RenderGraph>(ctx);
    
    // Register swapchain targets as external
    m_swapchainColorHandle = m_renderGraph->registerExternalImage(
        swapchain->getImage(0),  // Need to expose this
        swapchain->getImageView(0),
        swapchain->getSurfaceFormat().format,
        swapchain->getExtent().width,
        swapchain->getExtent().height,
        "SwapchainColor"
    );
    
    // ... similar for depth ...
    
    // NEW: Create test pass (geometry)
    m_renderGraph->addPass("GeometryPass")
        .writes(m_swapchainColorHandle)
        .writes(m_swapchainDepthHandle)
        .execute([this](gpu::RenderPassContext& ctx) {
            // Current rendering code goes here
        });
    
    m_renderGraph->compile();
}
```

**Checklist:**
- [ ] Initialize graph in onInit
- [ ] Register swapchain images
- [ ] Create ONE test pass
- [ ] Compile code
- [ ] Test: Does graph execute? (add logging)

---

### Step 5.2: Implement Layout Transitions in Graph
**File:** `source/toaster/toast_gpu/vk/vk_render_graph.cpp`

**What to implement:**
```cpp
void RenderGraph::execute(uint32 frameIndex) {
    // Get command buffer (from swapchain or create one)
    auto& cmdBuffer = ...; // TODO: Get from swapchain
    
    cmdBuffer.begin(...);
    
    for (size_t passIdx : m_executionOrder) {
        RenderPass& pass = m_passes[passIdx];
        
        // Auto-transition images for this pass's reads/writes
        for (const auto& writeAccess : pass.getWrites()) {
            auto* resource = m_resources.getResource(writeAccess.handle);
            
            // Get expected layout for this access type
            vk::ImageLayout newLayout = _getLayoutForAccess(writeAccess.accessMask);
            
            // Insert transition if needed
            if (resource->getCurrentLayout() != newLayout) {
                _recordImageTransition(cmdBuffer, *resource,
                    resource->getCurrentLayout(), newLayout,
                    resource->getCurrentAccessMask(), writeAccess.accessMask);
                
                resource->setCurrentLayout(newLayout);
                resource->setCurrentAccessMask(writeAccess.accessMask);
            }
        }
        
        // Similar for reads...
        
        // Execute pass
        RenderPassContext ctx(&pass, cmdBuffer, m_resources, frameIndex);
        pass.execute(ctx);
    }
    
    cmdBuffer.end();
}
```

**Checklist:**
- [ ] Implement layout transition logic
- [ ] Implement access mask tracking
- [ ] Helper: _getLayoutForAccess()
- [ ] Helper: _recordImageTransition()
- [ ] Test: Transitions recorded correctly?

---

### Step 5.3: Migrate Geometry Rendering to Graph
**File:** `client/source/client_layer.cpp` (modify onUpdate)

**Before:**
```cpp
void ClientLayer::onUpdate(const float32 p_dt) {
    auto &command_buffer = swapchain->getCurrentCommandBuffer();
    
    // Manual setup, transitions, rendering...
    command_buffer.reset();
    command_buffer.begin(...);
    // ... transitions ...
    command_buffer.beginRendering(...);
    // ... draw calls ...
    command_buffer.endRendering();
}
```

**After:**
```cpp
void ClientLayer::onUpdate(const float32 p_dt) {
    m_time += p_dt;
    
    // Update data
    updateCameraUBO(m_time);
    
    // Execute graph (everything else is automatic)
    m_renderGraph->execute(swapchain->getFrameIndex());
}
```

**Checklist:**
- [ ] Move rendering logic into graph pass callback
- [ ] Remove manual layout transitions from onUpdate
- [ ] Remove manual attachment setup
- [ ] Test: Geometry still renders correctly?
- [ ] Compare output with old method

---

### Step 5.4: Performance Testing
**What to measure:**
- Frame time before/after
- Memory allocations
- Command buffer record time
- GPU frame time

**Checklist:**
- [ ] Add timing instrumentation
- [ ] Measure baseline (old method)
- [ ] Measure with graph
- [ ] Compare results
- [ ] Profile if slower (optimize)

---

## Phase 6: Cleanup & Polish (1 hour, ~100 LOC)

### Step 6.1: Remove Old Code
**What to do:**
- Remove manual layout transition calls from ClientLayer
- Remove manual attachment setup
- Remove duplicate resource management
- Keep code clean and minimal

**Checklist:**
- [ ] Delete unused code
- [ ] Test still compiles
- [ ] Test still runs

---

### Step 6.2: Add Documentation
**What to add:**
- Usage examples in comments
- Explain graph declaration pattern
- Document resource management
- Explain when to use what

**Checklist:**
- [ ] Add class documentation
- [ ] Add method documentation
- [ ] Add example usage
- [ ] Compile final time

---

## Verification Checklist - Each Phase

**After Phase 1:**
- [ ] Code compiles
- [ ] Types are defined
- [ ] Can construct RenderGraphResource
- [ ] Can construct ResourceRegistry

**After Phase 2:**
- [ ] Code compiles
- [ ] Can create RenderPass instances
- [ ] Builder pattern works (chaining)
- [ ] Pass callbacks execute (empty lambdas)

**After Phase 3:**
- [ ] Code compiles
- [ ] RenderGraph stores passes
- [ ] compile() validates (or prints why it fails)
- [ ] execute() runs without errors

**After Phase 4:**
- [ ] ClientLayer compiles
- [ ] Graph can access swapchain resources
- [ ] No breaking changes to existing renders

**After Phase 5:**
- [ ] Test pass renders correctly
- [ ] Layout transitions appear in command buffer
- [ ] Performance comparable to old method
- [ ] Output matches old rendering

**After Phase 6:**
- [ ] Code is clean
- [ ] All systems documented
- [ ] Ready for production use

---

## Time Estimate Summary

| Phase | Task | Time | LOC |
|-------|------|------|-----|
| 1 | Core types & resources | 2h | 300 |
| 2 | Pass definitions | 3h | 400 |
| 3 | Graph container | 2h | 350 |
| 4 | Swapchain integration | 2h | 200 |
| 5 | Testing & migration | 3h | 500 |
| 6 | Cleanup & docs | 1h | 100 |
| **Total** | | **13h** | **1,850** |

**Plus:** Debugging, iteration, optimization (2-4 hours)

**Total estimate:** 15-17 hours for a complete, tested implementation.

---

## Common Pitfalls to Avoid

1. **Don't** try to handle all edge cases in phase 1
   - Start simple, add features as needed

2. **Don't** change ClientLayer too early
   - Build graph separately first, integrate last

3. **Don't** over-engineer layout transitions
   - Start with basic transitions, optimize later

4. **Don't** forget to validate dependencies
   - Circular reads/writes will cause subtle bugs

5. **Don't** assume Vulkan handles are cheap
   - Cache what you need, don't allocate per-frame

6. **Don't** ignore frame index
   - Multi-buffering requires proper per-frame resource handling

---

## Questions to Ask Yourself

- **Where does the command buffer come from?**
  - Option A: Swapchain allocates, graph records into it
  - Option B: Graph allocates, submits to Swapchain
  - → Recommend Option A (simpler integration)

- **How do multi-buffered resources work?**
  - Each frame needs separate UBOs, attachment images, etc.
  - Graph must track per-frame variant
  - → Store array[MAX_FRAMES_IN_FLIGHT] of resources

- **What about external resources (swapchain)?**
  - Can't allocate/own them
  - Must register as "external"
  - → Add RenderGraphResourceHandle for each swapchain image

- **How to handle dynamic pass count?**
  - Can you add/remove passes between frames?
  - Probably not initially
  - → Assume static graph, recompile if topology changes

---

## Success Criteria

You're done when:

- [ ] Graph compiles without warnings
- [ ] Old rendering path still works (backward compatible)
- [ ] New graph-based rendering produces same output
- [ ] Layout transitions are automatic and correct
- [ ] Performance is comparable or better
- [ ] Code is documented
- [ ] At least one complex pass (geometry) uses the graph
- [ ] Easy to add new passes (e.g., post-processing)

Good luck! Follow the checklist systematically and you'll have a solid render graph system.
