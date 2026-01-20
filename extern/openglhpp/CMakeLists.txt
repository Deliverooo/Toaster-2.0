project(openglhpp)

add_subdirectory(extern/glad)

set(GL_HPP_SRC
		src/opengl.cpp
)

set(GL_HPP_LIB glad)

option(GL_HPP_BUILD_SHARED "Build as a dll" ON)

if (${GL_HPP_BUILD_SHARED})
	add_library(openglhpp SHARED)
else ()
	add_library(openglhpp STATIC)
endif ()

target_sources(openglhpp PRIVATE ${GL_HPP_SRC})
target_include_directories(openglhpp PUBLIC include)

target_link_libraries(openglhpp PUBLIC ${GL_HPP_LIB})

if (${GL_HPP_BUILD_SHARED})
	target_compile_definitions(openglhpp PUBLIC GL_HPP_BUILD_DLL=1)

else ()
	target_compile_definitions(openglhpp PUBLIC GL_HPP_BUILD_DLL=0)
endif ()

target_compile_definitions(openglhpp PRIVATE GL_HPP_DLL_EXPORT)