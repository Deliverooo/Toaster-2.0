#include "toast_render/shader_reflection.hpp"

#include <iostream>
#include <spirv_cross/spirv_cross.hpp>
#include <spirv_cross/spirv_glsl.hpp>

namespace toaster::render
{
	struct ReflectedStruct
	{
		String name;
		uint32 size;

		spirv_cross::SPIRType type{spv::OpTypeStruct};
	};
}

template<>
struct std::hash<toaster::render::ReflectedStruct>
{
	size_t operator()(const toaster::render::ReflectedStruct &p_struct) const noexcept
	{
		return hash<string>{}(p_struct.name);
	}
};

namespace toaster::render
{
	auto reflectShader(const gpu::DynamicShader &p_shader) -> void
	{
		spirv_cross::CompilerGLSL compiler{p_shader.getBytecode()};

		const auto &ir{compiler.get_ir()};

		std::unordered_map<String, ReflectedStruct> reflected_structs;

		ir.for_each_typed_id<spirv_cross::SPIRType>([&](uint32 id, const spirv_cross::SPIRType &type) -> void
		{
			if (type.basetype == spirv_cross::SPIRType::Struct)
			{
				String name{compiler.get_name(id)};
				if (name.empty())
					return;
				if (!reflected_structs.contains(name))
				{
					ReflectedStruct &reflected_struct{reflected_structs[name]};
					reflected_struct.name = name;

					// Apparently for structs declared outside of uniform/storage buffers, the compiler cannot compute their size...
					try
					{
						reflected_struct.size = compiler.get_declared_struct_size(type);
					}
					catch (const std::exception &e)
					{
						LOG_WARN("{}", e.what());
					}
				}
			}
		});

		auto it{
			std::ranges::find_if(reflected_structs, [](const auto &pair) -> bool
			{
				if (pair.first.starts_with("TST__"))
					return true;
				return false;
			})
		};

		if (it != reflected_structs.end())
		{
			String name{it->second.name};
			auto & _struct{it->second};
			_struct.name = name.substr(std::strlen("TST__"));

			LOG_INFO("{}", _struct.name);
			LOG_INFO("{}", _struct.size);
		}

		// for (auto &[name, _struct]: reflected_structs)
		// {
		// 	if (name.starts_with("TST__"))
		// 	{
		// 		LOG_INFO("Found toaster material declaration: {}", name.substr(std::strlen("TST__")));
		//
		// 		_struct.size = compiler.get_declared_struct_size(_struct.type);
		// 		LOG_INFO("Size: {}", _struct.size);
		// 	}
		//
		// 	// LOG_INFO("Name: {}", name);
		// 	//
		// 	// try
		// 	// {
		// 	// 	_struct.size = compiler.get_declared_struct_size(_struct.type);
		// 	// }
		// 	// catch (const std::exception &e)
		// 	// {
		// 	// 	LOG_INFO("{}", e.what());
		// 	// }
		// 	// LOG_INFO("Size: {}", _struct.size);
		// 	//
		// 	// LOG_INFO("");
		// }
	}
}
