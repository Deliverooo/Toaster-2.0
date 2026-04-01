/*!
 * @file gl_shader_uniform.hpp
 */
#pragma once

#include "../shader_common.hpp"
#include <unordered_map>

namespace toaster::gpu
{
	class GLShaderUniformDeclaration : public ShaderUniformDeclaration
	{
	private:
		friend class GLShader;
		friend class GLShaderUniformBufferDeclaration;

	public:
		GLShaderUniformDeclaration(
			EShaderDomain domain,
			EShaderUniformType type,
			const String& name,
			uint32 count = 1);

		const String& getName() const override { return m_name; }
		uint32 getSize() const override { return m_size; }
		uint32 getCount() const override { return m_count; }
		uint32 getOffset() const override { return m_offset; }
		EShaderDomain getDomain() const override { return m_domain; }
		EShaderUniformType getType() const override { return m_type; }
		int32 getLocation() const override { return m_location; }

		bool isArray() const { return m_count > 1; }

		// Converts GL type enum to our EShaderUniformType
		static EShaderUniformType glTypeToUniformType(uint32 glType);
		static uint32 sizeOfUniformType(EShaderUniformType type);

	protected:
		void setOffset(uint32 offset) override { m_offset = offset; }

	private:
		String m_name;
		uint32 m_size{0};
		uint32 m_count{1};
		uint32 m_offset{0};
		EShaderDomain m_domain;
		EShaderUniformType m_type;
		mutable int32 m_location{-1};
	};

	class GLShaderResourceDeclaration : public ShaderResourceDeclaration
	{
	private:
		friend class GLShader;

	public:
		GLShaderResourceDeclaration(
			EShaderResourceType type,
			const String& name,
			uint32 count = 1);

		const String& getName() const override { return m_name; }
		uint32 getRegister() const override { return m_register; }
		uint32 getCount() const override { return m_count; }
		EShaderResourceType getType() const override { return m_type; }

		// Converts GL type enum to our EShaderResourceType
		static EShaderResourceType glTypeToResourceType(uint32 glType);

	private:
		String m_name;
		uint32 m_register{0};
		uint32 m_count{1};
		EShaderResourceType m_type;
	};

	class GLShaderUniformBufferDeclaration : public ShaderUniformBufferDeclaration
	{
	private:
		friend class GLShader;

	public:
		GLShaderUniformBufferDeclaration(
			const String& name,
			EShaderDomain domain);

		~GLShaderUniformBufferDeclaration() override;

		const String& getName() const override { return m_name; }
		uint32 getRegister() const override { return m_register; }
		uint32 getSize() const override { return m_size; }
		EShaderDomain getDomain() const override { return m_domain; }
		const ShaderUniformList& getUniformDeclarations() const override { return m_uniforms; }

		ShaderUniformDeclaration* findUniform(const String& name) override;

		void pushUniform(GLShaderUniformDeclaration* uniform);

	private:
		String m_name;
		EShaderDomain m_domain;
		uint32 m_register{0};
		uint32 m_size{0};
		ShaderUniformList m_uniforms;
	};
}
