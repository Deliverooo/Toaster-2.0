#include "globals.hpp"

#include "io/filesystem.hpp"

namespace toaster::gpu
{
	struct GlobalData
	{
		RefPtr<Shader>   g_defaultShader;
		RefPtr<Material> g_defaultMaterial;
	};

	static GlobalData *s_globalData = nullptr;

	void Globals::init()
	{
		s_globalData = new GlobalData{};

		io::filesystem::setWorkingDirectory("../../../source/toaster/toast_shaders");

		s_globalData->g_defaultShader = Shader::create("Default", {
														   {EShaderType::eVertex, io::filesystem::readFile("triangle.vert.glsl").c_str()},
														   {EShaderType::ePixel, io::filesystem::readFile("triangle.pixel.glsl").c_str()}
													   });
		s_globalData->g_defaultMaterial = Material::create(s_globalData->g_defaultShader, "Default");
	}

	void Globals::shutdown()
	{
		delete s_globalData;
	}

	RefPtr<Shader> Globals::defaultShader()
	{
		return s_globalData->g_defaultShader;
	}

	RefPtr<Material> Globals::defaultMaterial()
	{
		return s_globalData->g_defaultMaterial;
	}
}
