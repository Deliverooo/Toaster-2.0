
add_definitions(-DWIN32)

if (NOT MSVC)
	message(FATAL_ERROR "Compiler is unsupported")
endif ()

# Needed for some MSVC installations, example warning:
# `4099 : PDB {filename} was not found with {object/library}`.
string(APPEND CMAKE_EXE_LINKER_FLAGS " /SAFESEH:NO /ignore:4099")
string(APPEND CMAKE_SHARED_LINKER_FLAGS " /SAFESEH:NO /ignore:4099")
string(APPEND CMAKE_MODULE_LINKER_FLAGS " /SAFESEH:NO /ignore:4099")


list(APPEND PLATFORM_LINKLIBS
		ws2_32 vfw32 winmm kernel32 user32 gdi32 comdlg32 Comctl32 version
		advapi32 shfolder shell32 ole32 oleaut32 uuid psapi Dbghelp Shlwapi
		pathcch Shcore Dwmapi Crypt32 Bcrypt WindowsApp
)

add_definitions(
		-DNOMINMAX
		-D_CRT_NONSTDC_NO_DEPRECATE
		-D_CRT_SECURE_NO_DEPRECATE
		-D_SCL_SECURE_NO_DEPRECATE
		-D_CONSOLE
		-D_LIB
)

# MSVC11 needs _ALLOW_KEYWORD_MACROS to build
add_definitions(-D_ALLOW_KEYWORD_MACROS)

if (NOT MSVC_CLANG)
	string(APPEND CMAKE_CXX_FLAGS " /permissive- /Zc:__cplusplus /Zc:inline")
	string(APPEND CMAKE_C_FLAGS " /Zc:inline")

	# For VS2022+ we can enable the new preprocessor
	if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 19.30.30423)
		string(APPEND CMAKE_CXX_FLAGS " /Zc:preprocessor")
		string(APPEND CMAKE_C_FLAGS " /Zc:preprocessor")
	endif ()
endif ()

macro(windows_find_package package_name)
	find_package(${package_name})
endmacro()