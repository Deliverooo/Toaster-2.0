#pragma once

#include "gpu/shader.hpp"
#include "gpu/texture.hpp"
// #include "gpu/vulkan/descriptor_set_manager.hpp"

#include <unordered_set>

#include "math/matrix.hpp"
#include "vulkan/descriptor_set_manager.hpp"

namespace tst
{
	enum class MaterialFlag
	{
		None                 = BIT(0),
		DepthTest            = BIT(1),
		Blend                = BIT(2),
		TwoSided             = BIT(3),
		DisableShadowCasting = BIT(4)
	};

	class Material : public RefCounted
	{
	public:
		Material(RefPtr<Shader> shader, const std::string &name = "");
		Material(RefPtr<Material> material, const std::string &name = "");

		void invalidate();
		void onShaderReloaded();

		const ShaderUniform *            findUniformDeclaration(const std::string &name);
		const ShaderResourceDeclaration *findResourceDeclaration(const std::string &name);

		void set(const std::string &name, float value);
		void set(const std::string &name, int value);
		void set(const std::string &name, uint32_t value);
		void set(const std::string &name, bool value);
		void set(const std::string &name, const tsm::vec2f &value);
		void set(const std::string &name, const tsm::vec3f &value);
		void set(const std::string &name, const tsm::vec4f &value);
		void set(const std::string &name, const tsm::vec2i &value);
		void set(const std::string &name, const tsm::vec3i &value);
		void set(const std::string &name, const tsm::vec4i &value);

		void set(const std::string &name, const tsm::mat3x3f &value);
		void set(const std::string &name, const tsm::mat4x4f &value);

		void set(const std::string &name, RefPtr<Texture2D> texture);
		void set(const std::string &name, RefPtr<Texture2D> texture, uint32_t arrayIndex);
		// void Set(const std::string &name, RefPtr<TextureCube> texture);
		void set(const std::string &name, RefPtr<Image2D> image);
		void set(const std::string &name, RefPtr<Image2D> image, uint32_t arrayIndex);
		void set(const std::string &name, RefPtr<ImageView> image);
		void set(const std::string &name, RefPtr<ImageView> image, uint32_t arrayIndex);

		float &       getFloat(const std::string &name);
		int32_t &     getInt(const std::string &name);
		uint32_t &    getUInt(const std::string &name);
		bool &        getBool(const std::string &name);
		tsm::vec2f &  getVector2(const std::string &name);
		tsm::vec3f &  getVector3(const std::string &name);
		tsm::vec4f &  getVector4(const std::string &name);
		tsm::mat3x3f &getMatrix3(const std::string &name);
		tsm::mat4x4f &getMatrix4(const std::string &name);

		RefPtr<Texture2D> getTexture2D(const std::string &name);
		// RefPtr<TextureCube> GetTextureCube(const std::string &name);

		RefPtr<Texture2D> tryGetTexture2D(const std::string &name);
		// Ref<TextureCube> TryGetTextureCube(const std::string &name);

		template<typename T>
		void set(const std::string &name, const T &value)
		{
			auto decl = findUniformDeclaration(name);
			TST_ASSERT(decl);
			if (!decl)
				return;

			auto &buffer = m_uniformStorageBuffer;
			buffer.write((uint8 *) &value, decl->getSize(), decl->getOffset());
		}

		template<typename T>
		T &get(const std::string &name)
		{
			auto decl = findUniformDeclaration(name);
			TST_ASSERT(decl);
			auto &buffer = m_uniformStorageBuffer;
			return buffer.read<T>(decl->getOffset());
		}

		template<typename T>
		RefPtr<T> getResource(const std::string &name)
		{
			return nullptr; // m_DescriptorSetManager.GetInput<T>(name);
		}

		template<typename T>
		RefPtr<T> tryGetResource(const std::string &name)
		{
			return nullptr; //m_DescriptorSetManager.GetInput<T>(name);
		}

		uint32_t getFlags() const { return m_materialFlags; }
		void     setFlags(uint32_t flags) { m_materialFlags = flags; }
		bool     getFlag(MaterialFlag flag) const { return (uint32_t) flag & m_materialFlags; }

		void setFlag(MaterialFlag flag, bool value = true)
		{
			if (value)
			{
				m_materialFlags |= (uint32_t) flag;
			}
			else
			{
				m_materialFlags &= ~(uint32_t) flag;
			}
		}

		RefPtr<Shader>     getShader() { return m_shader; }
		const std::string &getName() const { return m_name; }

		void                    prepare();
		nvrhi::BindingSetHandle getBindingSet(uint32_t frameIndex) const;

		Buffer getUniformStorageBuffer() const { return m_uniformStorageBuffer; }

	private:
		void init();
		void allocateStorage();

		RefPtr<Shader> m_shader;
		std::string    m_name;
		uint32_t       m_materialFlags = 0;

		Buffer m_uniformStorageBuffer;

		DescriptorSetManager m_descriptorSetManager;
	};
}
