#include "descriptor_set_manager.hpp"

#include "platform/application.hpp"
#include "renderer/renderer.hpp"

namespace tst
{
	namespace utils
	{
		inline ERenderResourceType GetDefaultResourceType(ERenderInputType inputType)
		{
			switch (inputType)
			{
				case ERenderInputType::eImageSampler: return ERenderResourceType::eSampler;
				case ERenderInputType::eImageSampler2D: return ERenderResourceType::eTexture2D;
				case ERenderInputType::eImageSampler3D: return ERenderResourceType::eTextureCube;
				case ERenderInputType::eStorageImage2D: return ERenderResourceType::eImage2D;
				case ERenderInputType::eStorageImage3D: return ERenderResourceType::eTextureCube;
				case ERenderInputType::eUniformBuffer: return ERenderResourceType::eUniformBuffer;
				case ERenderInputType::eStorageBuffer: return ERenderResourceType::eStorageBuffer;
			}

			TST_ASSERT(false);
			return ERenderResourceType::eNone;
		}

		inline bool IsWriteable(ERenderInputType inputType)
		{
			return inputType == ERenderInputType::eStorageImage1D || inputType == ERenderInputType::eStorageImage2D || inputType == ERenderInputType::eStorageImage3D ||
				   inputType == ERenderInputType::eStorageBuffer;
		}

		inline nvrhi::ResourceType GetBindingLayoutType(nvrhi::BindingLayoutHandle bindingLayout, uint32_t binding)
		{
			const nvrhi::BindingLayoutDesc *desc = bindingLayout->getDesc();
			if (!desc)
				return nvrhi::ResourceType::None;

			for (const auto &item: desc->bindings)
			{
				if (item.slot == binding)
					return item.type;
			}
			return nvrhi::ResourceType::None;
		}
	}

	DescriptorSetManager::DescriptorSetManager(const DescriptorSetManagerSpecification &specification)
		: m_Specification(specification)
	{
		Init();
	}

	DescriptorSetManager::DescriptorSetManager(const DescriptorSetManager &other)
		: m_Specification(other.m_Specification)
	{
		Init();
		InputResources = other.InputResources;
		Bake();
	}

	DescriptorSetManager DescriptorSetManager::Copy(const DescriptorSetManager &other)
	{
		DescriptorSetManager result(other);
		return result;
	}

	void DescriptorSetManager::Init()
	{
		const auto &shaderDescriptorSets = m_Specification.shader->getShaderDescriptorSets();
		uint32_t    framesInFlight       = Renderer::getConfigInfo().maxFramesInFlight;
		m_BindingSetHandles.resize(framesInFlight);

		for (uint32_t set = m_Specification.startSet; set <= m_Specification.EndSet; set++)
		{
			if (set >= shaderDescriptorSets.size())
				break;

			const auto &shaderDescriptor = shaderDescriptorSets[set];
			for (auto &&[bname, inputDecl]: shaderDescriptor.inputDeclarations)
			{
				// NOTE(Emily): This is a hack to fix a bad input decl name
				//				Coming from somewhere.
				const char *broken = strrchr(bname.c_str(), '.');
				std::string name   = broken ? broken + 1 : bname;

				InputDeclarations[name] = inputDecl;

				uint32_t binding = inputDecl.binding;

				// Insert default resources (useful for materials)
				if (m_Specification.DefaultResources || true)
				{
					// Create RenderPassInput
					RenderPassInput &input = InputResources[set][binding];
					input.Input.resize(inputDecl.count);
					input.Type        = utils::GetDefaultResourceType(inputDecl.type);
					input.IsWriteable = utils::IsWriteable(inputDecl.type);

					// Set default textures and samplers
					if (inputDecl.type == ERenderInputType::eImageSampler)
					{
						for (size_t i      = 0; i < input.Input.size(); i++)
							input.Input[i] = Renderer::getDefaultSampler();
					}
					if (inputDecl.type == ERenderInputType::eImageSampler2D)
					{
						for (size_t i      = 0; i < input.Input.size(); i++)
							input.Input[i] = Renderer::getWhiteTexture();
					}
					// else if (inputDecl.Type == ERenderInputType::eImageSampler3D)
					// {
					// 	for (size_t i      = 0; i < input.Input.size(); i++)
					// 		input.Input[i] = Renderer::getBlackCubeTexture();
					// }
				}

				for (uint32_t frameIndex = 0; frameIndex < framesInFlight; frameIndex++)
					m_BindingSetHandles[frameIndex][set][binding].resize(inputDecl.count);
			}
		}
	}

	void DescriptorSetManager::SetInput(std::string_view name, RefPtr<UniformBufferSet> uniformBufferSet)
	{
		const RenderInputDeclaration *decl = GetInputDeclaration(name);
		if (decl)
		{
			InputResources.at(decl->set).at(decl->binding).Set(uniformBufferSet);
			m_State = EState::ePending;
		}
		else
		{
			TST_WARN_TAG("Renderer", "[RenderPass ({})] Input {} not found", m_Specification.debugName, name);
		}
	}

	void DescriptorSetManager::SetInput(std::string_view name, RefPtr<UniformBuffer> uniformBuffer)
	{
		const RenderInputDeclaration *decl = GetInputDeclaration(name);
		if (decl)
		{
			InputResources.at(decl->set).at(decl->binding).Set(uniformBuffer);
			m_State = EState::ePending;
		}
		else
		{
			TST_WARN_TAG("Renderer", "[RenderPass ({})] Input {} not found", m_Specification.debugName, name);
		}
	}

	void DescriptorSetManager::SetInput(std::string_view name, RefPtr<StorageBufferSet> storageBufferSet)
	{
		const RenderInputDeclaration *decl = GetInputDeclaration(name);
		if (decl)
		{
			InputResources.at(decl->set).at(decl->binding).Set(storageBufferSet);
			m_State = EState::ePending;
		}
		else
		{
			TST_WARN_TAG("Renderer", "[RenderPass ({})] Input {} not found", m_Specification.debugName, name);
		}
	}

	void DescriptorSetManager::SetInput(std::string_view name, RefPtr<StorageBuffer> storageBuffer)
	{
		const RenderInputDeclaration *decl = GetInputDeclaration(name);
		if (decl)
		{
			InputResources.at(decl->set).at(decl->binding).Set(storageBuffer);
			m_State = EState::ePending;
		}
		else
		{
			TST_WARN_TAG("Renderer", "[RenderPass ({})] Input {} not found", m_Specification.debugName, name);
		}
	}

	void DescriptorSetManager::SetInput(std::string_view name, RefPtr<Texture2D> texture, uint32_t arrayIndex)
	{
		const RenderInputDeclaration *decl = GetInputDeclaration(name);
		if (decl)
		{
			InputResources.at(decl->set).at(decl->binding).Set(texture, arrayIndex);
			m_State = EState::ePending;
		}
		else
		{
			TST_WARN_TAG("Renderer", "[RenderPass ({})] Input {} not found", m_Specification.debugName, name);
		}
	}

	// void DescriptorSetManager::SetInput(std::string_view name, RefPtr<TextureCube> textureCube)
	// {
	// 	const RenderInputDeclaration *decl = GetInputDeclaration(name);
	// 	if (decl)
	// 	{
	// 		InputResources.at(decl->set).at(decl->binding).Set(textureCube);
	// 		m_State = State::Pending;
	// 	}
	// 	else
	// 	{
	// 		TST_WARN_TAG("Renderer", "[RenderPass ({})] Input {} not found", m_Specification.DebugName, name);
	// 	}
	// }

	void DescriptorSetManager::SetInput(std::string_view name, RefPtr<Image2D> image, uint32_t arrayIndex)
	{
		const RenderInputDeclaration *decl = GetInputDeclaration(name);
		if (decl)
		{
			InputResources.at(decl->set).at(decl->binding).Set(image, arrayIndex);
			m_State = EState::ePending;
		}
		else
		{
			TST_WARN_TAG("Renderer", "[RenderPass ({})] Input {} not found", m_Specification.debugName, name);
		}
	}

	void DescriptorSetManager::SetInput(std::string_view name, RefPtr<ImageView> image, uint32_t arrayIndex)
	{
		const RenderInputDeclaration *decl = GetInputDeclaration(name);
		if (decl)
			InputResources.at(decl->set).at(decl->binding).Set(image, arrayIndex);
		else
			TST_WARN_TAG("Renderer", "[RenderPass ({})] Input {} not found", m_Specification.debugName, name);
	}

	void DescriptorSetManager::SetInput(std::string_view name, RefPtr<Sampler> sampler)
	{
		const RenderInputDeclaration *decl = GetInputDeclaration(name);
		if (decl)
		{
			InputResources.at(decl->set).at(decl->binding).Set(sampler);
			m_State = EState::ePending;
		}
		else
		{
			TST_WARN_TAG("Renderer", "[RenderPass ({})] Input {} not found", m_Specification.debugName, name);
		}
	}

	bool DescriptorSetManager::IsInvalidated(uint32_t set, uint32_t binding) const
	{
		if (InvalidatedInputResources.find(set) != InvalidatedInputResources.end())
		{
			const auto &resources = InvalidatedInputResources.at(set);
			return resources.find(binding) != resources.end();
		}

		return false;
	}

	std::set<uint32_t> DescriptorSetManager::HasBufferSets() const
	{
		// Find all descriptor sets that have either UniformBufferSet or StorageBufferSet descriptors
		std::set<uint32_t> sets;

		for (const auto &[set, resources]: InputResources)
		{
			for (const auto &[binding, input]: resources)
			{
				if (input.Type == ERenderResourceType::eUniformBufferSet || input.Type == ERenderResourceType::eStorageBufferSet)
				{
					sets.insert(set);
					break;
				}
			}
		}
		return sets;
	}

	bool DescriptorSetManager::Validate()
	{
		// Go through pipeline requirements to make sure we have all required resource
		const auto &shaderDescriptorSets = m_Specification.shader->getShaderDescriptorSets();

		// Nothing to validate, pipeline only contains material inputs
		//if (shaderDescriptorSets.size() < 2)
		//	return true;

		for (uint32_t set = m_Specification.startSet; set <= m_Specification.EndSet; set++)
		{
			if (set >= shaderDescriptorSets.size())
				break;

			// No descriptors in this set
			if (!shaderDescriptorSets[set])
				continue;

			if (InputResources.find(set) == InputResources.end())
			{
				TST_ERROR_TAG("Renderer", "[RenderPass ({})] No input resources for Set {}", m_Specification.debugName, set);
				return false;
			}

			const auto &setInputResources = InputResources.at(set);

			const auto &shaderDescriptor = shaderDescriptorSets[set];
			for (auto &&[name, inputDecl]: shaderDescriptor.inputDeclarations)
			{
				uint32_t binding = inputDecl.binding;
				if (setInputResources.find(binding) == setInputResources.end())
				{
					TST_ERROR_TAG("Renderer", "[RenderPass ({})] No input resource for {}.{}", m_Specification.debugName, set, binding);
					TST_ERROR_TAG("Renderer", "[RenderPass ({})] Required resource is {} ({})", m_Specification.debugName, name, (int) inputDecl.type);
					return false;
				}

				const auto &resource = setInputResources.at(binding);
				if (!IsCompatibleInput(resource.Type, inputDecl.type))
				{
					TST_ERROR_TAG("Renderer", "[RenderPass ({})] Required resource is wrong type! {} but needs {}", m_Specification.debugName, (uint16_t) resource.Type,
								  (int) inputDecl.type);
					return false;
				}

				if (resource.Type != ERenderResourceType::eImage2D && resource.Input[0] == nullptr)
				{
					TST_ERROR_TAG("Renderer", "[RenderPass ({})] Resource is null! {} ({}.{})", m_Specification.debugName, name, set, binding);
					return false;
				}
			}
		}

		// All resources present
		return true;
	}

	void DescriptorSetManager::Bake()
	{
		// Make sure all resources are present and we can properly bake
		if (!Validate())
		{
			TST_ERROR_TAG("Renderer", "[RenderPass] Bake - Validate failed! {}", m_Specification.debugName);
			return;
		}

		// If valid, we can create descriptor sets
		nvrhi::DeviceHandle device = Application::getGraphicsDevice();

		auto bufferSets             = HasBufferSets();
		bool perFrameInFlight       = !bufferSets.empty();
		perFrameInFlight            = true; // always
		uint32_t descriptorSetCount = Renderer::getConfigInfo().maxFramesInFlight;
		if (!perFrameInFlight)
			descriptorSetCount = 1;

		m_BindingSets.resize(descriptorSetCount);
		for (auto &set: m_BindingSets)
			set = {};

		// for (auto& set : m_BindingSetHandles)
		// 	set.clear();

		for (const auto &[set, setData]: InputResources)
		{
			uint32_t descriptorCountInSet = bufferSets.find(set) != bufferSets.end() ? descriptorSetCount : 1;
			for (uint32_t frameIndex = 0; frameIndex < descriptorSetCount; frameIndex++)
			{
				nvrhi::BindingLayoutHandle bindingLayout = m_Specification.shader->getDescriptorSetLayout(set);

				nvrhi::BindingSetDesc bindingSetDesc;

				if (m_BindingSets[frameIndex].size() <= set)
					m_BindingSets[frameIndex].resize(set + 1);

				auto &                                          bindingSetHandleMap = m_BindingSetHandles[frameIndex].at(set);
				std::vector<std::vector<nvrhi::TextureHandle> > imageInfoStorage;
				uint32_t                                        imageInfoStorageIndex = 0;

				for (const auto &[binding, input]: setData)
				{
					auto &storedHandles = bindingSetHandleMap.at(binding);
					storedHandles.resize(input.Input.size());

					// VkWriteDescriptorSet& writeDescriptor = storedWriteDescriptor.WriteDescriptorSet;
					// writeDescriptor.dstSet = descriptorSet;

					switch (input.Type)
					{
						case ERenderResourceType::eUniformBuffer:
						{
							RefPtr<UniformBuffer> buffer = input.Input[0].as<UniformBuffer>();
							nvrhi::BufferHandle   handle = buffer->GetHandle();
							bindingSetDesc.bindings.push_back(nvrhi::BindingSetItem::ConstantBuffer(binding, handle));
							storedHandles[0] = handle;

							// Defer if resource doesn't exist
							if (buffer->GetHandle() == nullptr)
								InvalidatedInputResources[set][binding] = input;

							break;
						}
						case ERenderResourceType::eUniformBufferSet:
						{
							RefPtr<UniformBufferSet> buffer = input.Input[0].as<UniformBufferSet>();
							nvrhi::BufferHandle      handle = buffer->Get(frameIndex)->GetHandle();
							bindingSetDesc.bindings.push_back(nvrhi::BindingSetItem::ConstantBuffer(binding, handle));
							storedHandles[0] = handle;

							break;
						}
						case ERenderResourceType::eStorageBuffer:
						{
							RefPtr<StorageBuffer> buffer     = input.Input[0].as<StorageBuffer>();
							nvrhi::BufferHandle   handle     = buffer->getHandle();
							nvrhi::ResourceType   layoutType = utils::GetBindingLayoutType(bindingLayout, binding);
							if (layoutType == nvrhi::ResourceType::RawBuffer_SRV)
								bindingSetDesc.bindings.push_back(nvrhi::BindingSetItem::RawBuffer_SRV(binding, handle));
							else
								bindingSetDesc.bindings.push_back(nvrhi::BindingSetItem::RawBuffer_UAV(binding, handle));
							storedHandles[0] = handle;

							break;
						}
						case ERenderResourceType::eStorageBufferSet:
						{
							RefPtr<StorageBufferSet> buffer     = input.Input[0].as<StorageBufferSet>();
							nvrhi::BufferHandle      handle     = buffer->get(frameIndex)->getHandle();
							nvrhi::ResourceType      layoutType = utils::GetBindingLayoutType(bindingLayout, binding);
							if (layoutType == nvrhi::ResourceType::RawBuffer_SRV)
								bindingSetDesc.bindings.push_back(nvrhi::BindingSetItem::RawBuffer_SRV(binding, handle));
							else
								bindingSetDesc.bindings.push_back(nvrhi::BindingSetItem::RawBuffer_UAV(binding, handle));
							storedHandles[0] = handle;

							break;
						}
						case ERenderResourceType::eTexture2D:
						{
							for (size_t i = 0; i < input.Input.size(); i++)
							{
								RefPtr<Texture2D> texture = input.Input[i].as<Texture2D>();

								nvrhi::TextureHandle  handle         = texture->GetHandle();
								nvrhi::BindingSetItem bindingSetItem = nvrhi::BindingSetItem::Texture_SRV(binding, handle);
								bindingSetItem.arrayElement          = (uint32_t) i;
								bindingSetDesc.bindings.push_back(bindingSetItem);

								storedHandles[i] = handle;
							}

							break;
						}
						// case ERenderResourceType::eTextureCube:
						// {
						// 	RefPtr<TextureCube> texture   = input.Input[0].as<TextureCube>();
						// 	ImageInfo *         imageInfo = (ImageInfo *) texture->getDescriptorInfo();
						//
						// 	nvrhi::TextureHandle handle = imageInfo->ImageHandle;
						//
						// 	nvrhi::BindingSetItem bindingSetItem = input.IsWriteable
						// 											   ? nvrhi::BindingSetItem::Texture_UAV(binding, handle)
						// 											   : nvrhi::BindingSetItem::Texture_SRV(binding, handle);
						//
						// 	bindingSetItem.dimension = imageInfo->Dimension;
						// 	TST_ASSERT(bindingSetItem.dimension == nvrhi::TextureDimension::TextureCube);
						//
						// 	bindingSetDesc.bindings.push_back(bindingSetItem);
						// 	storedHandles[0] = handle;
						//
						// 	break;
						// }
						case ERenderResourceType::eImage2D:
						{
							for (size_t i = 0; i < input.Input.size(); i++)
							{
								RefPtr<RendererResource> image = input.Input[i].as<RendererResource>();
								// Defer if resource doesn't exist
								if (image == nullptr)
								{
									InvalidatedInputResources[set][binding] = input;
									break;
								}

								ImageInfo *          imageInfo = static_cast<ImageInfo *>(image->getDescriptorInfo());
								nvrhi::TextureHandle handle    = imageInfo->ImageHandle;

								nvrhi::BindingSetItem bindingSetItem = input.IsWriteable
																		   ? nvrhi::BindingSetItem::Texture_UAV(binding, handle)
																		   : nvrhi::BindingSetItem::Texture_SRV(binding, handle);

								bindingSetItem.arrayElement = i;
								bindingSetItem.subresources = imageInfo->ImageView;
								bindingSetItem.dimension    = imageInfo->Dimension;

								bindingSetDesc.bindings.push_back(bindingSetItem);
								storedHandles[i] = handle;
							}

							break;
						}
						case ERenderResourceType::eSampler:
						{
							RefPtr<RendererResource> sampler = input.Input[0].as<RendererResource>();
							// Defer if resource doesn't exist
							if (sampler == nullptr)
							{
								InvalidatedInputResources[set][binding] = input;
								break;
							}

							nvrhi::SamplerHandle handle = static_cast<Sampler *>(sampler->getDescriptorInfo())->getHandle();
							bindingSetDesc.bindings.push_back(nvrhi::BindingSetItem::Sampler(binding, handle));
							storedHandles[0] = handle;

							break;
						}
					}
				}

				if (!bindingSetDesc.bindings.empty())
				{
					m_BindingSets[frameIndex][set] = device->createBindingSet(bindingSetDesc, bindingLayout);
				}
			}
		}

		for (uint32_t frameIndex = 0; frameIndex < descriptorSetCount; frameIndex++)
		{
			if (!m_BindingSets[frameIndex].empty() && m_BindingSets[frameIndex][0] == nullptr)
			{
				nvrhi::BindingLayoutHandle bindingLayout = m_Specification.shader->getDescriptorSetLayout(0);

				nvrhi::BindingSetDesc bindingSetDesc;
				m_BindingSets[frameIndex][0] = device->createBindingSet(bindingSetDesc, bindingLayout);
			}
		}
	}

	void DescriptorSetManager::InvalidateAndUpdate()
	{
		if (m_State == EState::eReady)
			return;

		uint32_t currentFrameIndex = Renderer::_getCurrentFrameIndex();

		// Check for invalidated resources
		for (const auto &[set, inputs]: InputResources)
		{
			for (const auto &[binding, input]: inputs)
			{
				const auto &bindingSetHandleArray = m_BindingSetHandles[currentFrameIndex].at(set).at(binding);
				const auto &bindingSetHandle      = bindingSetHandleArray[0];

				switch (input.Type)
				{
					case ERenderResourceType::eUniformBuffer:
					{
						nvrhi::BufferHandle handle = input.Input[0].as<UniformBuffer>()->GetHandle();
						if (handle != bindingSetHandle)
						{
							InvalidatedInputResources[set][binding] = input;
							break;
						}
						break;
					}
					case ERenderResourceType::eUniformBufferSet:
					{
						nvrhi::BufferHandle handle = input.Input[0].as<UniformBufferSet>()->Get(currentFrameIndex)->GetHandle();
						if (handle != bindingSetHandle)
						{
							InvalidatedInputResources[set][binding] = input;
							break;
						}
						break;
					}
					case ERenderResourceType::eStorageBuffer:
					{
						nvrhi::BufferHandle handle = input.Input[0].as<StorageBuffer>()->getHandle();
						if (handle != bindingSetHandle)
						{
							InvalidatedInputResources[set][binding] = input;
							break;
						}
						break;
					}
					case ERenderResourceType::eStorageBufferSet:
					{
						nvrhi::BufferHandle handle = input.Input[0].as<StorageBufferSet>()->get(currentFrameIndex)->getHandle();
						if (handle != bindingSetHandle)
						{
							InvalidatedInputResources[set][binding] = input;
							break;
						}
						break;
					}
					case ERenderResourceType::eTexture2D:
					{
						for (size_t i = 0; i < input.Input.size(); i++)
						{
							RefPtr<Texture2D> texture = input.Input[i].as<Texture2D>();
							if (texture == nullptr)
							{
								texture = Renderer::getWhiteTexture();
								TST_ASSERT(false);
							}

							if (texture->GetHandle() != bindingSetHandleArray[i])
							{
								InvalidatedInputResources[set][binding] = input;
								break;
							}
						}
						break;
					}
					// case ERenderResourceType::eTextureCube:
					// {
					// 	nvrhi::TextureHandle handle = input.Input[0].as<TextureCube>()->GetHandle();
					// 	if (handle != bindingSetHandle)
					// 	{
					// 		InvalidatedInputResources[set][binding] = input;
					// 		break;
					// 	}
					// 	break;
					// }
					case ERenderResourceType::eImage2D:
					{
						for (size_t i = 0; i < input.Input.size(); i++)
						{
							RefPtr<RendererResource> image  = input.Input[i].as<RendererResource>();
							nvrhi::TextureHandle     handle = static_cast<ImageInfo *>(image->getDescriptorInfo())->ImageHandle;
							if (handle != bindingSetHandleArray[i])
							{
								InvalidatedInputResources[set][binding] = input;
								break;
							}
						}
						break;
					}
					case ERenderResourceType::eSampler:
					{
						RefPtr<RendererResource> image  = input.Input[0].as<RendererResource>();
						nvrhi::SamplerHandle     handle = static_cast<Sampler *>(image->getDescriptorInfo())->getHandle();
						if (handle != bindingSetHandle)
						{
							InvalidatedInputResources[set][binding] = input;
							break;
						}
						break;
					}
				}
			}
		}

		if (!InvalidatedInputResources.empty())
		{
			TST_INFO_TAG("Renderer", "DescriptorSetManager::InvalidateAndUpdate ({}) - updating {} descriptors (frameIndex={})", m_Specification.debugName,
						 InvalidatedInputResources.size(), currentFrameIndex);
			Bake();
		}

		if (!m_Specification.IsDynamic)
			m_State = EState::eReady;
	}

	bool DescriptorSetManager::HasDescriptorSets() const
	{
		return !m_BindingSets.empty() && !m_BindingSets[0].empty();
	}

	uint32_t DescriptorSetManager::GetFirstSetIndex() const
	{
		if (InputResources.empty())
			return UINT32_MAX;

		// Return first key (key == descriptor set index)
		return InputResources.begin()->first;
	}

	nvrhi::BindingSetHandle DescriptorSetManager::GetBindingSet(uint32_t frameIndex) const
	{
		if (m_BindingSets.empty())
			return nullptr;

		if (frameIndex > 0 && m_BindingSets.size() == 1)
			frameIndex = 0; // Frame index is irrelevant for this type of render pass

		if (m_BindingSets[frameIndex].empty())
			return nullptr;

		return m_BindingSets[frameIndex][0];
	}

	nvrhi::BindingSetVector DescriptorSetManager::GetBindingSets(uint32_t frameIndex) const
	{
		if (m_BindingSets.empty())
			return {};

		if (frameIndex > 0 && m_BindingSets.size() == 1)
			frameIndex = 0; // Frame index is irrelevant for this type of render pass

		nvrhi::BindingSetVector result(m_BindingSets[frameIndex].size());
		for (size_t i = 0; i < result.size(); i++)
			result[i] = m_BindingSets[frameIndex][i];

		return result;
	}

	bool DescriptorSetManager::IsInputValid(std::string_view name) const
	{
		std::string nameStr(name);
		return InputDeclarations.find(nameStr) != InputDeclarations.end();
	}

	const RenderInputDeclaration *DescriptorSetManager::GetInputDeclaration(std::string_view name) const
	{
		std::string nameStr(name);
		if (InputDeclarations.find(nameStr) == InputDeclarations.end())
			return nullptr;

		const RenderInputDeclaration &decl = InputDeclarations.at(nameStr);
		return &decl;
	}
}
