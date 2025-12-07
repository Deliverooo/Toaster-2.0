# Collect sources

set(TST_PLATFORM_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/os.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/window.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/application.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/input.cpp"
)

# depending on the platform, we will link a different main entry point
if (TST_PLATFORM_WINDOWS)
    list(APPEND TST_PLATFORM_SOURCES
     "${CMAKE_CURRENT_SOURCE_DIR}/windows/windows_main.cpp"
     "${CMAKE_CURRENT_SOURCE_DIR}/windows/windows_render_thread.cpp"
     "${CMAKE_CURRENT_SOURCE_DIR}/windows/windows_os.cpp"
     )
    
elseif (TST_PLATFORM_MACOS)
    list(APPEND TST_PLATFORM_SOURCES
      "${CMAKE_CURRENT_SOURCE_DIR}/platform/src/macos/macos_main.cpp"
      "${CMAKE_CURRENT_SOURCE_DIR}/platform/src/macos/macos_os.cpp"
      )
endif ()


target_sources(ToasterEngine PRIVATE ${TST_PLATFORM_SOURCES})