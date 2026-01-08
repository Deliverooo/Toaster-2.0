set(INCLUDE_DIRS "")

set(LIB

		PRIVATE tst::toast_kernel
)




set(SOURCE_FILES "main.cpp")

add_executable(toaster ${SOURCE_FILES})

target_link_libraries(toaster PRIVATE ${LIB})

