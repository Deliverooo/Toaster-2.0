include(GNUInstallDirs)

# Install targets
install(TARGETS ToasterEngine ToasterCore ToasterGPU
        EXPORT ToasterTargets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# Install headers
install(DIRECTORY engine/core/include/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# Export targets
install(EXPORT ToasterTargets
        FILE ToasterTargets.cmake
        NAMESPACE Toaster::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Toaster
)

# Config file
include(CMakePackageConfigHelpers)
configure_package_config_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/ToasterConfig.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/ToasterConfig.cmake"
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Toaster
)

install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/ToasterConfig.cmake"
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Toaster
)