set(SRC
		main.cpp
)

add_executable(toaster_test ${SRC})

target_link_libraries(toaster_test PRIVATE Toaster::ToastKernel)
target_link_libraries(toaster_test PRIVATE Toaster::ToastAsset)
