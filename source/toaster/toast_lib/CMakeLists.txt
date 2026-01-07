set(INC
		PUBLIC .
)


set(INC_SYS "")


set(SRC

		logging.cpp
		logging.hpp

		toast_assert.cpp
		toast_assert.h

		math/math_vector.cpp
		math/math_vector.hpp
		math/math_constants.hpp

		util_defines.hpp

		toast_exception.cpp
		toast_exception.hpp
)

set(LIB
		PUBLIC fmt
		PUBLIC glm
)

if (WIN32)
	list(APPEND LIB dxgi)
endif ()

add_library(toast_lib STATIC)
target_include_directories(toast_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(toast_lib ${LIB})
target_sources(toast_lib PRIVATE ${SRC})

add_library(tst::toast_lib ALIAS toast_lib)