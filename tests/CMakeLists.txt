# In tests/CMakeLists.txt
if(FALSE)
    find_package(Catch2 REQUIRED)
    
    add_executable(ToasterTests
        unit/core_tests.cpp
        unit/renderer_tests.cpp
    )
    
    target_link_libraries(ToasterTests
        PRIVATE
            Toaster::Core
            Toaster::Renderer
            Catch2::Catch2WithMain
    )
    
    include(CTest)
    include(Catch)
    catch_discover_tests(ToasterTests)
endif()