set(TST_GPU_SOURCES

)

# Add platform-specific sources
if (TST_ENABLE_OPENGL)
    list(APPEND TST_GPU_SOURCES 
    "${CMAKE_CURRENT_SOURCE_DIR}/opengl/opengl_render_api.cpp"
    )
endif ()

if (TST_ENABLE_VULKAN)
    list(APPEND TST_GPU_SOURCES 
    "${CMAKE_CURRENT_SOURCE_DIR}/vulkan/vulkan_render_api.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/vulkan/vulkan_rendering_context.cpp"
    )
endif ()

target_sources(ToasterEngine PRIVATE ${TST_GPU_SOURCES})