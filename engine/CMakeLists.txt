cmake_minimum_required(VERSION 3.10)


# Engine core executable
add_executable(ToasterEngine)

# Add platform-specific definitions
if (TST_PLATFORM_WINDOWS)
	target_compile_definitions(ToasterEngine PUBLIC TST_PLATFORM_WINDOWS)
elseif (TST_PLATFORM_MACOS)
	target_compile_definitions(ToasterEngine PUBLIC TST_PLATFORM_MACOS)
elseif (TST_PLATFORM_LINUX)
	target_compile_definitions(ToasterEngine PUBLIC TST_PLATFORM_LINUX)
endif ()

add_subdirectory(core)
add_subdirectory(platform)
add_subdirectory(main)
add_subdirectory(gpu)

target_include_directories(ToasterEngine PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")


target_link_libraries(ToasterEngine PRIVATE
		Toaster::Core
)

if (TST_ENABLE_OPENGL)
	target_link_libraries(ToasterEngine PRIVATE glad)
	target_compile_definitions(ToasterEngine PUBLIC TST_ENABLE_OPENGL)

endif ()

if (TST_ENABLE_VULKAN)
	message(STATUS "Found Vulkan: $ENV{VULKAN_SDK}")
	target_link_libraries(ToasterEngine PUBLIC Vulkan::Vulkan) # Vulkan::Vulkan does not work on all platforms
	target_compile_definitions(ToasterEngine PUBLIC TST_ENABLE_VULKAN)

endif ()

toaster_set_target_properties(ToasterEngine)
toaster_set_warnings(ToasterEngine)
