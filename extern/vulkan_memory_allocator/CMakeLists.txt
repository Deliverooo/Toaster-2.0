set(SRC
  vk_mem_alloc_impl.cc
  vk_mem_alloc.h
)

add_library(vulkan_memory_allocator STATIC)
target_include_directories(vulkan_memory_allocator PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_sources(vulkan_memory_allocator PRIVATE ${SRC})
target_link_libraries(vulkan_memory_allocator PUBLIC Vulkan::Vulkan)
