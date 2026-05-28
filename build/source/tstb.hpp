#pragma once

#include "toast_project/project.hpp"

#include <argparse/argparse.hpp>
#include <assimp/material.h>

namespace toaster::tstb
{
	// Toaster Build
	class TstB final
	{
	public:
		TstB(const io::filesystem::Path &p_tproj_path);

		static auto tryGetTProjPath() -> io::filesystem::Path;

		auto newProject(const argparse::ArgumentParser &p_new_command) -> int32;
		auto removeProject(const argparse::ArgumentParser &p_remove_command) -> int32;

		auto newMesh(const argparse::ArgumentParser &p_mesh_cmd) -> int32;
		auto newMaterial(aiMaterial *p_mat, const io::filesystem::Path &p_parent_path) -> void;
		auto newTexture(const io::filesystem::Path &p_tex_path) -> void;

		auto buildAssets(const argparse::ArgumentParser &p_build_assets_command) -> int32;
		auto buildAssemblies(const argparse::ArgumentParser &p_build_command) -> int32;

	private:
		UniquePtr<Project> m_project{nullptr};
	};
}
