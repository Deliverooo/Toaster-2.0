#pragma once

#include "gpu/uniform_buffer_set.hpp"
#include "gpu/storage_buffer_set.hpp"
#include "gpu/image.hpp"
#include "gpu/texture.hpp"

#include "vulkan_shader.hpp"

#include <set>

namespace tst
{
	struct RenderPassInput
	{
		static constexpr uint32 MAX_ARRAY_ELEMENTS = 32;
		using InputArray                           = nvrhi::static_vector<RefPtr<RefCounted>, MAX_ARRAY_ELEMENTS>;

		ERenderResourceType Type        = ERenderResourceType::eNone;
		bool                IsWriteable = false;
		InputArray          Input;

		RenderPassInput() = default;

		RenderPassInput(RefPtr<UniformBuffer> uniformBuffer)
			: Type(ERenderResourceType::eUniformBuffer), Input(InputArray{uniformBuffer})
		{
		}

		RenderPassInput(RefPtr<UniformBufferSet> uniformBufferSet)
			: Type(ERenderResourceType::eUniformBufferSet), Input(InputArray{uniformBufferSet})
		{
		}

		RenderPassInput(RefPtr<StorageBuffer> storageBuffer)
			: Type(ERenderResourceType::eStorageBuffer), Input(InputArray{storageBuffer})
		{
		}

		RenderPassInput(RefPtr<StorageBufferSet> storageBufferSet)
			: Type(ERenderResourceType::eStorageBufferSet), Input(InputArray{storageBufferSet})
		{
		}

		RenderPassInput(RefPtr<Texture2D> texture)
			: Type(ERenderResourceType::eTexture2D), Input(InputArray{texture})
		{
		}

		// RenderPassInput(RefPtr<TextureCube> texture)
		// 	: Type(ERenderResourceType::TextureCube), Input(InputArray{texture})
		// {
		// }

		RenderPassInput(RefPtr<Image2D> image)
			: Type(ERenderResourceType::eImage2D), Input(InputArray{image})
		{
		}

		void Set(RefPtr<UniformBuffer> uniformBuffer, uint32 arrayIndex = 0)
		{
			Type              = ERenderResourceType::eUniformBuffer;
			Input[arrayIndex] = uniformBuffer;
		}

		void Set(RefPtr<UniformBufferSet> uniformBufferSet, uint32 arrayIndex = 0)
		{
			Type              = ERenderResourceType::eUniformBufferSet;
			Input[arrayIndex] = uniformBufferSet;
		}

		void Set(RefPtr<StorageBuffer> storageBuffer, uint32 arrayIndex = 0)
		{
			Type              = ERenderResourceType::eStorageBuffer;
			Input[arrayIndex] = storageBuffer;
		}

		void Set(RefPtr<StorageBufferSet> storageBufferSet, uint32 arrayIndex = 0)
		{
			Type              = ERenderResourceType::eStorageBufferSet;
			Input[arrayIndex] = storageBufferSet;
		}

		void Set(RefPtr<Texture2D> texture, uint32 arrayIndex = 0)
		{
			Type              = ERenderResourceType::eTexture2D;
			Input[arrayIndex] = texture;
		}

		// void Set(RefPtr<TextureCube> texture, uint32 arrayIndex = 0)
		// {
		// 	Type              = ERenderResourceType::TextureCube;
		// 	Input[arrayIndex] = texture;
		// }

		void Set(RefPtr<Image2D> image, uint32 arrayIndex = 0)
		{
			Type              = ERenderResourceType::eImage2D;
			Input[arrayIndex] = image;
		}

		void Set(RefPtr<ImageView> image, uint32 arrayIndex = 0)
		{
			Type              = ERenderResourceType::eImage2D;
			Input[arrayIndex] = image;
		}

		void Set(RefPtr<Sampler> sampler, uint32 arrayIndex = 0)
		{
			Type              = ERenderResourceType::eSampler;
			Input[arrayIndex] = sampler;
		}
	};

	inline bool IsCompatibleInput(ERenderResourceType inputResource, ERenderInputType inputType)
	{
		switch (inputType)
		{
			case ERenderInputType::eImageSampler:
			{
				return inputResource == ERenderResourceType::eSampler;
			}
			case ERenderInputType::eImageSampler2D:
			{
				return inputResource == ERenderResourceType::eTexture2D || inputResource == ERenderResourceType::eImage2D;
			}
			case ERenderInputType::eImageSampler3D:
			{
				return inputResource == ERenderResourceType::eTextureCube;
			}
			case ERenderInputType::eStorageImage2D:
			{
				return inputResource == ERenderResourceType::eImage2D;
			}
			case ERenderInputType::eStorageImage3D:
			{
				return inputResource == ERenderResourceType::eTextureCube;
			}
			case ERenderInputType::eUniformBuffer:
			{
				return inputResource == ERenderResourceType::eUniformBuffer || inputResource == ERenderResourceType::eUniformBufferSet;
			}
			case ERenderInputType::eStorageBuffer:
			{
				return inputResource == ERenderResourceType::eStorageBuffer || inputResource == ERenderResourceType::eStorageBufferSet;
			}
		}
		return false;
	}

	struct DescriptorSetManagerSpecification
	{
		RefPtr<VulkanShader> shader;
		std::string          debugName;

		// Which descriptor sets should be managed
		uint32 startSet = 0;
		uint32   EndSet   = 3;

		bool IsDynamic        = true; // Automatically check resources for change
		bool DefaultResources = false;
	};

	struct DescriptorSetManager
	{
		enum class EState
		{
			eNone = 0, ePending = 1, eReady = 2
		};

		//
		// Input Resources (map of set->binding->resource)
		//
		// Invalidated input resources will attempt to be assigned on Renderer::BeginRenderPass
		// This is useful for resources that may not exist at RenderPass creation but will be
		// present during actual rendering
		std::map<uint32, std::map<uint32, RenderPassInput> > InputResources;
		std::map<uint32, std::map<uint32, RenderPassInput> > InvalidatedInputResources;
		std::map<std::string, RenderInputDeclaration>            InputDeclarations;

		DescriptorSetManager() = default;
		DescriptorSetManager(const DescriptorSetManager &other);
		DescriptorSetManager(const DescriptorSetManagerSpecification &specification);
		static DescriptorSetManager Copy(const DescriptorSetManager &other);

		void SetInput(std::string_view name, RefPtr<UniformBufferSet> uniformBufferSet);
		void SetInput(std::string_view name, RefPtr<UniformBuffer> uniformBuffer);
		void SetInput(std::string_view name, RefPtr<StorageBufferSet> storageBufferSet);
		void SetInput(std::string_view name, RefPtr<StorageBuffer> storageBuffer);
		void SetInput(std::string_view name, RefPtr<Texture2D> texture, uint32 arrayIndex = 0);
		// void SetInput(std::string_view name, RefPtr<TextureCube> textureCube);
		void SetInput(std::string_view name, RefPtr<Image2D> image, uint32 arrayIndex = 0);
		void SetInput(std::string_view name, RefPtr<ImageView> image, uint32 arrayIndex = 0);
		void SetInput(std::string_view name, RefPtr<Sampler> sampler);

		template<typename T>
		RefPtr<T> GetInput(std::string_view name)
		{
			const RenderInputDeclaration *decl = GetInputDeclaration(name);
			if (decl)
			{
				auto setIt = InputResources.find(decl->set);
				if (setIt != InputResources.end())
				{
					auto resourceIt = setIt->second.find(decl->binding);
					if (resourceIt != setIt->second.end())
						return resourceIt->second.Input[0].as<T>();
				}
			}
			return nullptr;
		}

		bool IsInvalidated(uint32 set, uint32 binding) const;
		bool Validate();
		void Bake();

		std::set<uint32> HasBufferSets() const;
		void               InvalidateAndUpdate();

		bool                          HasDescriptorSets() const;
		uint32                      GetFirstSetIndex() const;
		nvrhi::BindingSetHandle       GetBindingSet(uint32 frameIndex) const;
		nvrhi::BindingSetVector       GetBindingSets(uint32 frameIndex) const;
		bool                          IsInputValid(std::string_view name) const;
		const RenderInputDeclaration *GetInputDeclaration(std::string_view name) const;

	private:
		void Init();

	private:
		DescriptorSetManagerSpecification m_Specification;
		EState                             m_State = EState::eNone;

		// Per-frame in flight
		nvrhi::static_vector<nvrhi::static_vector<nvrhi::BindingSetHandle, nvrhi::c_MaxBindingLayouts>, 3> m_BindingSets;
		// Frame->set->binding
		static constexpr uint32                                                                                                          MAX_ARRAY_ELEMENTS = 32;
		nvrhi::static_vector<std::map<uint32, std::map<uint32, nvrhi::static_vector<nvrhi::ResourceHandle, MAX_ARRAY_ELEMENTS> > >, 3> m_BindingSetHandles;
	};
}
