#include "tstb.hpp"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include "assimp/Exporter.hpp"
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

	constexpr auto c_tsceneTemplate{
		R"(Scene:
	  Name: {0}
	  SceneEnvironmentAssetID: 5772156649
	  Entities:
	    - Entity: 2718281828459
	      TagComponent:
	        Tag: Camera Controller
	      TransformComponent:
	        Translation: [0, 0, 0]
	        Rotation: [1, 0, 0, 0]
	        Scale: [1, 1, 1]
	      ScriptComponent:
	        ClassName: Toaster.CameraController)"
	};

	constexpr auto c_tprojTemplate{
		R"(Project:
	  Name: {0}
	  AssetRegistryPath: resources/asset_registry.treg
	  ScriptDirectory: resources/scripts
	  SceneDirectory: resources/scenes
	  MeshDirectory: resources/meshes
	  TextureDirectory: resources/textures
	  StartupSceneName: {1})"
	};

	constexpr auto c_assetRegistryTemplate{
		R"(AssetRegistry:
	[])"
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

	constexpr uint32 s_MeshImportFlags{
		aiProcess_CalcTangentSpace | aiProcess_Triangulate | aiProcess_SortByPType | aiProcess_GenNormals | aiProcess_GenUVCoords | aiProcess_OptimizeMeshes |
		aiProcess_JoinIdenticalVertices | aiProcess_LimitBoneWeights | aiProcess_ValidateDataStructure | aiProcess_GlobalScale | aiProcess_FlipUVs
	};

	TstB::TstB(const io::filesystem::Path &p_tproj_path)
	{
		// If not, we are most likely creating a project
		if (io::filesystem::exists(p_tproj_path))
		{
			m_project = make_unique<Project>();
			ProjectSerializer project_serializer{m_project.get()};
			project_serializer.deserialize(p_tproj_path);
		}
	}

	auto TstB::tryGetTProjPath() -> io::filesystem::Path
	{
		const auto working_directory{std::filesystem::current_path()};

		io::filesystem::Path tproj_file_path{};
		for (const auto &dir_it: std::filesystem::directory_iterator{working_directory})
		{
			if (dir_it.is_regular_file())
			{
				if (dir_it.path().extension() == ".tproj")
				{
					LOG_INFO("Found tproj file: {}", dir_it.path());
					tproj_file_path = dir_it.path();
				}
			}
		}
		return tproj_file_path;
	}

	auto TstB::newProject(const argparse::ArgumentParser &p_new_command) -> int32
	{
		io::filesystem::Path toaster_dll{fmt::format("{0}/script/{1}/{2}/Toaster.dll", os::getBinaryDirectory(), c_dotnetProfile, c_dotnetFrameworkVersion)};
		if (!std::filesystem::exists(toaster_dll))
		{
			LOG_ERROR("Toaster.dll does not exist at '{}'. Please run build_scripts.bat", toaster_dll);
			return -1;
		}

		auto project_name{p_new_command.get<String>("--name")};
		auto scene_name{p_new_command.get<String>("--sceneName")};

		LOG_INFO("Creating new project: '{}'", project_name);

		#pragma region create directories

		#define CREATE_DIRECTORY(__path, __info)\
			LOG_INFO("Creating {} directory", #__info); do { if (!std::filesystem::create_directory(__path)) {\
			LOG_ERROR("Directory already exists or creation failed"); return -1; } } while(false)

		const io::filesystem::Path project_root{project_name};
		CREATE_DIRECTORY(project_root, project root);
		const io::filesystem::Path resource_directory{project_root / "resources"};

		CREATE_DIRECTORY(resource_directory, resources);
		CREATE_DIRECTORY(resource_directory / "scripts", scripts);
		CREATE_DIRECTORY(resource_directory / "scenes", scenes);
		CREATE_DIRECTORY(resource_directory / "meshes", meshes);
		CREATE_DIRECTORY(resource_directory / "textures", textures);
		CREATE_DIRECTORY(resource_directory / "environments", environments);

		#undef CREATE_DIRECTORY
		#pragma endregion

		// Serializing the project with the project serializer class would require a valid project, which would require a valid render::RenderContext.
		LOG_INFO("Creating .tproj");
		String project_data{fmt::format(c_tprojTemplate, project_name, scene_name)};
		io::filesystem::writeFile(fmt::format("{0}.tproj", project_root / project_name), project_data);

		LOG_INFO("Creating asset registry");
		io::filesystem::writeFile(fmt::format("{0}/resources/asset_registry.treg", project_root), c_assetRegistryTemplate);

		LOG_INFO("Creating .csproj");
		// Create the .csproj manually so I can use net48, then build with dotnet
		io::filesystem::writeFile(resource_directory / "scripts" / fmt::format("{0}.csproj", project_name),
								  fmt::format(c_csprojTemplate, c_dotnetFrameworkVersion, c_dotnetLanguageVersion, toaster_dll));

		LOG_INFO("Building C# assembly");
		String build_scripts_command{fmt::format("cd {0} && dotnet build", io::filesystem::Path{resource_directory / "scripts"})};
		int32  err{std::system(build_scripts_command.c_str())};
		if (err == -1)
		{
			LOG_ERROR("Failed to build script projects!");
			return -1;
		}

		LOG_INFO("Creating default scene");
		io::filesystem::writeFile(resource_directory / "scenes" / fmt::format("{}.tscene", scene_name), fmt::format(c_tsceneTemplate, scene_name));

		LOG_INFO("Creating utility build scripts");
		io::filesystem::writeFile(project_root / "build_assets.bat", "tstb buildAssets");

		LOG_INFO("Creating .gitignore");
		io::filesystem::writeFile(project_root / ".gitignore", c_gitignoreData);

		return 0;
	}

	auto TstB::removeProject(const argparse::ArgumentParser &p_remove_command) -> int32
	{
		io::filesystem::Path project_dir{p_remove_command.get("--name")};
		if (!io::filesystem::exists(project_dir))
		{
			LOG_ERROR("Project directory '{}' does not exist", project_dir);
		}

		String confirm{"N"};
		LOG_WARN("Are you sure?: (Y / N)");
		std::cin >> confirm;
		if (confirm == "N" || confirm == "n")
		{
			LOG_INFO("ok...");
			return 0;
		}
		if (confirm == "Y" || confirm == "y")
		{
			std::error_code err{};
			std::uintmax_t  remove_count{std::filesystem::remove_all(project_dir, err)};
			if (err)
			{
				LOG_ERROR("Failed to remove project | Error: {}", err.message());
				return -1;
			}

			LOG_INFO("Successfully removed project: {} files", remove_count);
			return 0;
		}
		LOG_ERROR("Invalid option.");
		return -1;
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

	auto TstB::newMesh(const argparse::ArgumentParser &p_mesh_cmd) -> int32
	{
		auto src_path{p_mesh_cmd.get<String>("--srcPath")};

		LOG_INFO("Creating new mesh: {}", src_path);

		Assimp::Importer importer;

		const aiScene *scene{importer.ReadFile(src_path, s_MeshImportFlags)};
		if (!scene)
		{
			LOG_ERROR("Failed to load mesh file: {0}", src_path);
			return -1;
		}

		if (scene->HasMaterials())
		{
			for (uint32 i{0u}; i < scene->mNumMaterials; ++i)
			{
				auto ai_material = scene->mMaterials[i];
				newMaterial(ai_material, src_path);
			}
		}

		Assimp::Exporter exporter;
		// Tmesh is secretly just GLTF...
		aiReturn ret{exporter.Export(scene, "gltf2", io::filesystem::Path{src_path}.replace_extension(".tmesh").string())};

		if (ret != aiReturn_SUCCESS)
		{
			LOG_ERROR("Failed to export mesh: {}", exporter.GetErrorString());
			return -1;
		}

		LOG_INFO("Successfully exported mesh file");

		return 0;
	}

	auto TstB::newMaterial(aiMaterial *p_mat, const io::filesystem::Path &p_parent_path) -> void
	{
		String material_name{p_mat->GetName().C_Str()};
		LOG_INFO("Creating new material: {}", material_name);

		glm::vec3 albedo_colour{0.8f};
		{
			if (aiColor3D ai_colour; p_mat->Get(AI_MATKEY_COLOR_DIFFUSE, ai_colour) == AI_SUCCESS)
				albedo_colour = {ai_colour.r, ai_colour.g, ai_colour.b};
		}

		float32 roughness{0.4f};
		p_mat->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughness);

		float32 metalness{0.0f};
		p_mat->Get(AI_MATKEY_REFLECTIVITY, metalness);

		LOG_TRACE("\tAlbedo colour: {}", albedo_colour);
		LOG_TRACE("\tRoughness: {}", roughness);
		LOG_TRACE("\tMetalness: {}", metalness);

		auto get_path_and_create_texture_if_exists{
			[p_parent_path](const aiString &p_ai_path, const String &p_tex_name) -> std::optional<io::filesystem::Path>
			{
				const io::filesystem::Path texture_path{p_ai_path.C_Str()};
				io::filesystem::Path       tex_map_path{p_parent_path.parent_path() / texture_path};

				if (!io::filesystem::exists(tex_map_path))
				{
					tex_map_path = p_parent_path.parent_path() / texture_path.filename();
					if (!io::filesystem::exists(tex_map_path))
					{
						LOG_ERROR("\tFailed to find {} map at: {}", p_tex_name, tex_map_path);
						return std::nullopt;
					}
				}

				LOG_INFO("\tSuccessfully found {} map: {}", p_tex_name, tex_map_path);
				return tex_map_path;
			}
		};

		// Load albedo map
		{
			aiString ai_albedo_map_path;
			bool     has_albedo_map{p_mat->GetTexture(AI_MATKEY_BASE_COLOR_TEXTURE, &ai_albedo_map_path) == AI_SUCCESS};
			if (!has_albedo_map)
				has_albedo_map = p_mat->GetTexture(aiTextureType_DIFFUSE, 0, &ai_albedo_map_path) == AI_SUCCESS;

			if (has_albedo_map)
			{
				auto albedo_map_path{get_path_and_create_texture_if_exists(ai_albedo_map_path, "albedo")};
				if (albedo_map_path.has_value())
				{
					newTexture(*albedo_map_path);
				}
			}
			else
				LOG_WARN("\tMaterial '{}' does not have an albedo map", material_name);
		}

		// Load normal map
		{
			aiString ai_normal_map_path;
			if (p_mat->GetTexture(aiTextureType_NORMALS, 0, &ai_normal_map_path) == AI_SUCCESS)
			{
				auto normal_map_path{get_path_and_create_texture_if_exists(ai_normal_map_path, "normal")};
				if (normal_map_path.has_value())
				{
					newTexture(*normal_map_path);
				}
			}
			else
				LOG_WARN("\tMaterial '{}' does not have a normal map", material_name);
		}
	}

	auto TstB::newTexture(const io::filesystem::Path &p_tex_path) -> void
	{
		// Texture stuff...
	}

	auto TstB::buildAssets(const argparse::ArgumentParser &p_build_assets_command) -> int32
	{
		if (!m_project)
		{
			LOG_ERROR("Current directory does not contain a .tproj file!");
			return -1;
		}

		LOG_INFO("Attempting to build assets");

		auto &asset_manager{m_project->getAssetManager()};
		asset_manager.deserializeFromFile(m_project->getFullAssetRegistryPath());

		// Remove any assets whose path don't exist if flag is specified
		if (p_build_assets_command.get<bool>("--removeInvalid"))
			asset_manager.removeAssetsWithInvalidPaths();

		// We don't want to add assets that are already in the registry, so skip them.
		for (const auto &dir_it: std::filesystem::recursive_directory_iterator{m_project->getRootDirectory()})
		{
			if (dir_it.is_regular_file())
			{
				// The asset registry stores paths relative to the root directory of the project
				io::filesystem::Path registry_asset_path{std::filesystem::relative(dir_it.path(), m_project->getRootDirectory())};
				if (asset_manager.hasAnyAssetsWithPath(registry_asset_path))
					continue;

				switch (io::filesystem::getFileType(dir_it.path()))
				{
					case io::filesystem::EFileType::eImage:
					{
						asset::AssetMetadata metadata{registry_asset_path, asset::EAssetType::eTexture2D};
						asset_manager.addAssetMetadata({}, metadata);
						LOG_INFO("Found image asset file: {}", metadata.path);
						break;
					}
					case io::filesystem::EFileType::eEnvironmentMap:
					{
						asset::AssetMetadata metadata{registry_asset_path, asset::EAssetType::eTexture3D};
						asset_manager.addAssetMetadata({}, metadata);
						LOG_INFO("Found environment map asset file: {}", metadata.path);
						break;
					}
					case io::filesystem::EFileType::eMesh:
					{
						asset::AssetMetadata metadata{registry_asset_path, asset::EAssetType::eMesh};
						asset_manager.addAssetMetadata({}, metadata);
						LOG_INFO("Found mesh asset file: {}", metadata.path);
						break;
					}
					default: break;
				}
			}
		}

		// Serialize the new registry to the original path
		asset_manager.serializeToFile(m_project->getFullAssetRegistryPath());

		// This is the new asset registry
		asset_manager.printAssetRegistry();

		return 0;
	}

	auto TstB::buildAssemblies(const argparse::ArgumentParser &p_build_command) -> int32
	{
		if (!m_project)
		{
			LOG_ERROR("Current directory does not contain a .tproj file!");
			return -1;
		}
		LOG_INFO("Building C# assembly");
		const String build_scripts_command{fmt::format("cd {0} && dotnet build", m_project->getFullScriptDirectory())};
		if (const int32 err{std::system(build_scripts_command.c_str())}; err == -1)
		{
			LOG_ERROR("Failed to build script projects!");
			return -1;
		}

		return buildAssets(p_build_command);
	}
}
