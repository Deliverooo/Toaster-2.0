#include "toast_render/shader_reflection.hpp"

#include <iostream>
#include <spirv_cross/spirv_cross.hpp>
#include <spirv_cross/spirv_glsl.hpp>

namespace toaster::render::reflection
{
	auto reflectShader(const gpu::Shader &p_shader) -> ReflectionData
	{
		spirv_cross::CompilerGLSL compiler{p_shader.getBytecode()};

		const auto &ir{compiler.get_ir()};

		ReflectionData out_data;

		ir.for_each_typed_id<spirv_cross::SPIRType>([&](uint32 id, const spirv_cross::SPIRType &type) -> void
		{
			if (type.basetype == spirv_cross::SPIRType::Struct)
			{
				const String &name{compiler.get_name(id)};
				if (name.empty())
					return;
				if (!out_data.structs.contains(name))
				{
					ReflectedStruct &reflected_struct{out_data.structs[name]};
					reflected_struct.name = name;

					// Apparently for structs declared outside of uniform/storage buffers, the compiler cannot compute their size...
					try
					{
						reflected_struct.size = compiler.get_declared_struct_size(type);
					}
					catch (const std::exception &e)
					{
						// I don't need to report the "error" every time
						(void) e;
					}

					uint32 total_struct_members_size{0u};

					for (uint32 i{0u}; i < type.member_types.size(); ++i)
					{
						const String &member_name{compiler.get_member_name(id, i)};
						auto &        member{reflected_struct.members[member_name]};

						member.name   = member_name;
						member.offset = compiler.get_member_decoration(id, i, spv::DecorationOffset);
						member.size   = compiler.get_declared_struct_member_size(type, i);

						total_struct_members_size += member.size;
					}

					if (reflected_struct.size == 0)
						reflected_struct.size = total_struct_members_size;
				}
			}
		});

		return out_data;
	}
}
