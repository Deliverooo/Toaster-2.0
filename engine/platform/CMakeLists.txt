# Collect sources

set(TST_PLATFORM_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/os.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/window_manager.cpp"
)

# depending on the platform, we will link a different main entry point
if (TST_PLATFORM_WINDOWS)
    list(APPEND TST_PLATFORM_SOURCES
     "${CMAKE_CURRENT_SOURCE_DIR}/windows/windows_main.cpp"
     "${CMAKE_CURRENT_SOURCE_DIR}/windows/windows_platform.cpp"
     "${CMAKE_CURRENT_SOURCE_DIR}/windows/windows_os.cpp"
     "${CMAKE_CURRENT_SOURCE_DIR}/windows/windows_window_manager.cpp"
     )
    
elseif (TST_PLATFORM_MACOS)
    list(APPEND TST_PLATFORM_SOURCES
      "${CMAKE_CURRENT_SOURCE_DIR}/platform/src/macos/macos_main.cpp"
      "${CMAKE_CURRENT_SOURCE_DIR}/platform/src/macos/macos_platform.cpp"
      "${CMAKE_CURRENT_SOURCE_DIR}/platform/src/macos/macos_os.cpp"
      )
endif ()


target_sources(ToasterEngine PRIVATE ${TST_PLATFORM_SOURCES})