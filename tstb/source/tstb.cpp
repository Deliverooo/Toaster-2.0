#include "tstb.hpp"

#include "toast_lib/logging.hpp"
#include "toast_lib/os/terminal.hpp"

namespace toaster::tstb
{
	constexpr auto c_dotnetFrameworkVersion{"net48"};
	constexpr auto c_dotnetLanguageVersion{"10.0"};
	constexpr auto c_dotnetProfile{"Debug"};

	constexpr auto c_CMakeVersion{"3.30"};
	constexpr auto c_cppStandard{"23"};

	constexpr auto c_csprojTemplate{
		R"(<Project Sdk="Microsoft.NET.Sdk">
		<PropertyGroup>
			<TargetFramework>{0}</TargetFramework>
			<LangVersion>{1}</LangVersion>
			<ImplicitUsings>enable</ImplicitUsings>
			<Nullable>enable</Nullable>
		</PropertyGroup>
	<ItemGroup>
	  <Reference Include="Toaster">
	    <HintPath>{2}</HintPath>
	  </Reference>
	</ItemGroup>
	</Project>)"
	};

	constexpr auto c_gitignoreData{
		R"(.vs/
.idea/
.vscode/
resources/scripts/bin/
resources/scripts/obj/
	)"
	};

	constexpr auto c_RootCMakeListsTxtData{
		R"(cmake_minimum_required(VERSION {0})
set(CMAKE_CXX_STANDARD {1})
set(CMAKE_CXX_STANDARD_REQUIRED ON)

cmake_policy(SET CMP0077 NEW)

if (MSVC)
	add_compile_options(/MP)
endif ()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${{CMAKE_SOURCE_DIR}}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${{CMAKE_SOURCE_DIR}}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${{CMAKE_SOURCE_DIR}}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${{CMAKE_RUNTIME_OUTPUT_DIRECTORY}}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${{CMAKE_RUNTIME_OUTPUT_DIRECTORY}}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${{CMAKE_RUNTIME_OUTPUT_DIRECTORY}}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${{CMAKE_RUNTIME_OUTPUT_DIRECTORY}}")
# Prevent in-source builds
if (${{CMAKE_SOURCE_DIR}} STREQUAL ${{CMAKE_BINARY_DIR}})
	message(FATAL_ERROR "In-source builds are not allowed. Please create a separate build directory.")
endif ()

# Project definition
project("{2}")

find_package(Toaster REQUIRED)

# Select C++23 as the standard for C++ projects.
# If C++23 is not available, downgrading to an earlier standard is NOT OK.
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# Do not enable compiler specific language extensions.
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(src)
)"
	};

	constexpr auto c_srcCMakeListsTxtData{
		R"(set(SOURCE_FILES
		main.cpp
)

add_executable("{0}" ${{SOURCE_FILES}})

target_link_libraries("{0}" PRIVATE Toaster)
)"
	};

	constexpr auto c_mainCppData{R"(int main() { return 0; })"};

	TstB::TstB()
	{
	}

	auto TstB::newCppProj(const argparse::ArgumentParser &p_new_cpp_proj_cmd) -> int32
	{
		auto project_name{p_new_cpp_proj_cmd.get<String>("--name")};
		LOG_INFO("Creating new C++ project: {}", project_name);

		#pragma region create directories
		#define CREATE_DIRECTORY(__path, __info)\
		LOG_INFO("Creating {} directory", #__info); do { if (!std::filesystem::create_directory(__path)) {\
		LOG_ERROR("Directory already exists or creation failed"); return -1; } } while(false)

		const io::filesystem::Path project_root{project_name};
		CREATE_DIRECTORY(project_root, project root);

		const io::filesystem::Path source_dir{project_root / "src"};
		// Where the main file and the actual c++ code is
		CREATE_DIRECTORY(source_dir, src);

		// It is useful to have resources
		const io::filesystem::Path resource_directory{project_root / "resources"};
		CREATE_DIRECTORY(resource_directory, resources);
		CREATE_DIRECTORY(resource_directory / "scenes", scenes);
		CREATE_DIRECTORY(resource_directory / "meshes", meshes);
		CREATE_DIRECTORY(resource_directory / "textures", textures);
		CREATE_DIRECTORY(resource_directory / "environments", environments);

		#undef CREATE_DIRECTORY
		#pragma endregion

		LOG_INFO("Creating CMakeLists.txt(s)");

		{
			const String root_cmake_file_data{fmt::format(c_RootCMakeListsTxtData, c_CMakeVersion, c_cppStandard, project_name)};
			io::filesystem::writeFile(project_root / "CMakeLists.txt", root_cmake_file_data);
		}

		{
			const String src_cmake_file_data{fmt::format(c_srcCMakeListsTxtData, project_name)};
			io::filesystem::writeFile(source_dir / "CMakeLists.txt", src_cmake_file_data);
		}

		LOG_INFO("Creating main.cpp");
		{
			const String main_cpp_file_data{c_mainCppData};
			io::filesystem::writeFile(source_dir / "main.cpp", main_cpp_file_data);
		}

		LOG_INFO("Creating .gitignore");
		io::filesystem::writeFile(project_root / ".gitignore", c_gitignoreData);

		return 0;
	}
}
