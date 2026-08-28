#pragma once

struct RenderPhysicalDevice;
struct RenderDevice;
struct Window;
struct ImageMemory;
struct AttachmentGraphInstance;
struct SwapChain;
struct BufferMemory;
struct ShaderResourceManager;
struct Sampler;
struct Texture;
struct TextureView;
struct Resource;
struct PipelineHandle;
struct PipelineQueue;
struct GenericRenderPipelineInfo;
struct GeneratedPipelineInstance;
struct GPUCommandStream;
struct RenderShaderGraph;
struct ShaderResourceTemplateInstance;
struct OldStyleRenderPass;
struct DriverRenderTarget;
struct AllocationInstance;
struct DescriptorSetInstance;
struct AttachmentGraphLayout;

template<typename N>
struct RenderIndex
{
	int index = -1;

	RenderIndex() = default;
	RenderIndex(int val)
		: index(val)
	{

	}

	RenderIndex operator=(const int val)
	{
		this->index = val;
		return *this;
	}

	constexpr bool operator==(const RenderIndex& val) const
	{
		return val.index == this->index;
	}

	constexpr bool operator<=(const RenderIndex& val) const
	{
		return val.index <= this->index;
	}
};

using RenderPhysicalDeviceIndex = RenderIndex<RenderPhysicalDevice>;
using RenderDeviceIndex = RenderIndex<RenderDevice>;
using WindowIndex = RenderIndex<Window>;
using ImageMemoryIndex = RenderIndex<ImageMemory>;
using AttachmentGraphInstanceIndex = RenderIndex<AttachmentGraphInstance>;
using SwapChainIndex = RenderIndex<SwapChain>;
using BufferMemoryIndex = RenderIndex<BufferMemory>;
using ShaderResourceManagerIndex = RenderIndex<ShaderResourceManager>;
using SamplerIndex = RenderIndex<Sampler>;
using TextureIndex = RenderIndex<Texture>;
using TextureViewIndex = RenderIndex<TextureView>;
using ResourceIndex = RenderIndex<Resource>;
using PipelineHandleIndex = RenderIndex<PipelineHandle>;
using PipelineQueueIndex = RenderIndex<PipelineQueue>;
using GenericRenderPipelineInfoIndex = RenderIndex<GenericRenderPipelineInfo>;
using GeneratedPipelineInstanceIndex = RenderIndex<GeneratedPipelineInstance>;
using GPUCommandStreamIndex = RenderIndex<GPUCommandStream>;
using RenderShaderGraphIndex = RenderIndex<RenderShaderGraph>;
using ShaderResourceTemplateInstanceIndex = RenderIndex<ShaderResourceTemplateInstance>;
using DriverRenderTargetIndex = RenderIndex<DriverRenderTarget>;
using OldStyleRenderPassIndex = RenderIndex<OldStyleRenderPass>;
using AllocationInstanceIndex = RenderIndex<AllocationInstance>;
using DescriptorSetInstanceIndex = RenderIndex<DescriptorSetInstance>;
using AttachmentGraphLayoutIndex = RenderIndex<AttachmentGraphLayout>;