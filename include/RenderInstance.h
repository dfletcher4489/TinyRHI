#pragma once

#include <vulkan/vulkan.h>

#include "allocator/AppAllocator.h"
#include "IndexTypes.h"
#include "VKTypes.h"
#include "math/VertexTypes.h"
#include "WindowManager.h"

#include "VKRenderInstance.h"

namespace API 
{
	VkFormat ConvertComponentFormatTypeToVulkanFormat(ComponentFormatType type);

	VkCompareOp ConvertCompareOpToVulkanCompareOp(CompareOp testApp);

	VkFormat ConvertImageFormatToVulkanFormat(ImageFormat format);

	VkPrimitiveTopology ConvertTopology(PrimitiveType type);

	VkAccessFlags ConvertBarrierActionToVulkanAccessFlags(BarrierAction action);

	VkPipelineStageFlags ConvertBarrierStageToVulkanPipelineStage(PipelineStage sourceStage);

	VkShaderStageFlags ConvertShaderStageToVulkanShaderStage(ShaderStageType type);

	VkImageLayout ConvertImageLayoutToVulkanImageLayout(ImageLayout layout);

	void ConvertVertexInputToVKVertexAttrDescription(VertexInputDescription* inputDescs, int numInputDescs, int vertexBufferLocation, VkVertexInputAttributeDescription* attrs);

	VkCullModeFlags ConvertCullMode(CullMode mode);

	VkFrontFace ConvertTriangleWinding(TriangleWinding winding);

	void ConvertGPUFeatureRequestToVkPhysicalDeviceProperties(const GPUFeatureRequest* request,
		VkPhysicalDeviceFeatures2* features2,
		VkPhysicalDeviceVulkan12Features* features12);

	VkImageType ConvertImageTypeToVulkanImageType(ImageType imageType);

	VkImageAspectFlags ConvertImageViewAspectMaskToVulkanImageAspectFlags(ImageViewAspectMask aspectMask);

	VkImageViewType ConvertImageTypeToVulkanImageViewType(ImageType imageType);

	VkImageUsageFlags ConvertImageUsageFlagsToVulkanImageUsageFlags(ImageUsageFlags flags);

	VkMemoryPropertyFlags ConvertMemoryTypeToVkMemoryPropertyFlags(MemoryType memType);

	VkBlendFactor ConvertBlendFactorToVulkanBlendFactor(BlendFactor factor);

	VkBlendOp ConvertBlendOpToVulkanBlendOp(BlendOp op);

	VkLogicOp ConvertBlendLogicOpToVulkanLogicOp(BlendLogicOp op);

	VkFilter ConvertSamplerFilterModeToVulkanFilter(SamplerFilterMode filterMode);

	VkSamplerAddressMode ConvertSamplerAddressModeToVulkanSamplerAddressMode(SamplerAddressMode addressMode);

	VkSamplerMipmapMode ConvertSamplerMipmapModeToVulkanSamplerMipmapMode(SamplerMipmapMode mipmapMode);
}

struct RenderInstanceCreateInfo
{
	uint32_t maxGPUS;
	uint32_t maxLogicalDevices;
	uint32_t maxWindows;
	uint32_t maxSwapChains;
	uint32_t maxAttachmentGraphTemplates;
	uint32_t maxAttachmentGraphInstances;
	uint32_t maxImagePoolsCount;
	uint32_t maxBufferPoolsCount;
	uint32_t maxRenderTargets;
	uint32_t maxShaderGraphs;
	uint32_t maxShaderHandles;
	uint32_t maxShaderResourceTemplates;
	uint32_t maxDescriptorManagers;
	uint32_t maxComputeQueues;
	uint32_t maxRenderQueues;
	uint32_t maxPipelineTemplates;
	uint32_t maxPipelineInstances;
	uint32_t maxPipelineHandles;
	uint32_t maxAllocations;
	uint32_t maxSubAllocations;
	uint32_t maxGPUCommandsStreams;
	uint32_t maxTextureHandles;
	uint32_t maxSamplerHandles;
	uint32_t maxResourceStatuses;
	uint32_t commandBuffersSize;
	uint32_t commandsCacheSize;
	uint32_t internalLoggerRingSize;
	uint32_t numberOfDriverHostAllocations;
	uint32_t numberOfTransferCommandAllocations;
	uint32_t numberOfResourceUpdateAllocations;
	uint32_t numberOfDriverDeviceAllocations;
	uint32_t numberOfImageMemoryAllocations;
	uint32_t maxConcurrentRecordings;
	OSFileHandle internalRendererHandle;
};

struct LogicalDeviceCreateInfo
{
	GPUFeatureRequest* requestedPhysicalFeatures; 
	LogicalDeviceFeatures* requestedDeviceFeatures;
	size_t driverPermanentSize;
	size_t driverCacheSize;
	size_t deviceInstPermanentSize;
	size_t deviceInstHandleSize;
	size_t deviceInstCacheSize;
	uint32_t maxQueries;
	WindowIndex surfaceIndexForPresent;
	RenderPhysicalDeviceIndex physicalDeviceIndex;
};

struct RenderInstance
{
	RenderInstance() = default;

	~RenderInstance();

	void CreateRenderInstance(RenderInstanceCreateInfo *info, Allocator* instanceStorageAllocator, RingAllocator* instanceCacheAllocator);

	void DestroySwapChainAttachments(RenderDeviceIndex deviceSelection, EntryHandle swapChainIndex);

	int RecreateSwapChain(RenderDeviceIndex deviceSelection, SwapChainIndex swapChainIndex, uint32_t width, uint32_t height);

	int CreateAttachmentResources(RenderDeviceIndex deviceSelection, AttachmentGraphInstanceIndex& graphIndex, int renderPassIndex, int imageCount, TextureIndex* backBufferTextureIds, uint32_t width, uint32_t height,
		RenderPassType rpType, AttachmentClear* clears, DeviceSlabAllocator* rsvAllocator, DeviceSlabAllocator* dsvAllocator, ImageMemoryIndex rsvPoolIndex, ImageMemoryIndex dsvPoolIndex);

	int CreateSwapChainAttachment(RenderDeviceIndex deviceSelection, SwapChainIndex swapChainIndex, AttachmentGraphInstanceIndex graphIndex, int renderPassIndex, AttachmentClear* clears, DeviceSlabAllocator* rsvAllocator, DeviceSlabAllocator* dsvAllocator, ImageMemoryIndex rsvPoolIndex, ImageMemoryIndex dsvPoolIndex);

	int CreatePerFrameAttachment(RenderDeviceIndex deviceSelection, AttachmentGraphInstanceIndex graphIndex, int renderPassIndex, int imageCount, uint32_t width, uint32_t height, AttachmentClear* clears, DeviceSlabAllocator* rsvAllocator, DeviceSlabAllocator* dsvAllocator, ImageMemoryIndex rsvPoolIndex, ImageMemoryIndex dsvPoolIndex);

	AttachmentGraphInstanceIndex CreateAttachmentGraphInstance(RenderDeviceIndex deviceSelection, AttachmentGraph* graph);

	int CreateRenderPass(RenderDeviceIndex deviceSelection, AttachmentGraphInstance* graph);

	EntryHandle CreateVulkanComputePipelineTemplate(RenderDeviceIndex deviceSelection, ShaderGraph* graph);

	uint32_t BeginFrame(RenderDeviceIndex deviceSelection, SwapChainIndex swapChainIndex);

	int SubmitFrame(RenderDeviceIndex deviceSelection, SwapChainIndex swapChainIndex, uint32_t imageIndex);

	void WaitOnRender(RenderDeviceIndex deviceSelection);

	void CreatePipelines(StringView* pipelineDescriptions, int pipelineDescriptionsCount);

	int CreateDriverSwapChainData(RHIDevice* rhiDevice, EntryHandle swapChainIndex, uint32_t width, uint32_t height, bool recreate);

	void UploadHostTransfers(RHIDevice* rhiDevice);

	void UploadDescriptorsUpdates(RHIDevice* rhiDevice);

	void InvokeTransferCommands(RHIDevice* rhiDevice, RecordingBufferObject* rbo, BarrierAccumulator* accum);

	void UploadImageMemoryTransfers(RHIDevice* rhiDevice, RecordingBufferObject* rbo, BarrierAccumulator* accum);

	void UploadDeviceLocalTransfers(RHIDevice* rhiDevice, RecordingBufferObject* rbo, BarrierAccumulator* accum);

	int GetAllocFromBuffer(RenderDeviceIndex deviceSelection, BufferMemoryIndex bufferHandle, size_t structureSize, size_t copiesOfStructure, size_t alignment, AllocationType allocType, ComponentFormatType formatType, BufferAlignmentType bufferAlignmentType, int parentIndex, DeviceSlabAllocator* allocator);

	TextureIndex CreateImageHandle(
		RenderDeviceIndex deviceSelection,
		size_t gpuMemAddress,
		uint32_t width, uint32_t height,
		uint32_t mipLevels, uint32_t arrayLayers, ImageFormat format, ImageType imageType, ImageUsageFlags usageFlags, ImageMemoryIndex poolIndex);

	int CreateImageView(RenderDeviceIndex deviceSelection, TextureIndex& imageHandle, int firstMip, int mipCount, int firstLayer, int layerCount, ImageViewAspectMask imageAspect, ImageLayout desiredImageLayoutUsage);

	ImageMemoryIndex CreateImagePool(RenderDeviceIndex deviceSelection, size_t size, ImageFormat format, int maxWidth, int maxHeight, ImageUsageFlags usageFlags, MemoryType memType);

	RenderDeviceIndex CreateLogicalDevice(LogicalDeviceCreateInfo* createInfo);

	uint32_t GetSwapChainHeight(SwapChainIndex swapChainIndex);

	uint32_t GetSwapChainWidth(SwapChainIndex swapChainIndex);

	int CreateGraphicsPipelineObject(RenderDeviceIndex deviceSelection, GraphicsIntermediaryPipelineInfo *info);

	int CreateComputePipelineObject(RenderDeviceIndex deviceSelection, ComputeIntermediaryPipelineInfo* info);

	void DrawScene(RenderDeviceIndex deviceSelection, int commandStreamIndex, uint32_t imageIndex);

	void IncreaseMSAA(AttachmentGraphInstanceIndex& frameGraph, int renderPassIndex);

	void DecreaseMSAA(AttachmentGraphInstanceIndex& frameGraph, int renderPassIndex);

	int CreateShaderResourceMap(RHIDevice* device, ShaderGraph *graph);

	ShaderResourceSetBuilder AllocateShaderResourceSet(ShaderResourceManagerIndex descriptorManagerIndex, int shaderGraphIndex, int targetSet, int setCount);

	int CreateShaderResourceSet(ShaderResourceManager* descriptorManager, RenderDeviceIndex deviceSelection, int descriptorSet);

	void GeneratePipelineDescriptorBarriers(RenderDeviceIndex deviceSelection, ShaderResourceSetHandle* descriptorid, int descriptorcount, BarrierAccumulator* accumulator, int pipelineIndex);

	void GenerateDrawBindingsBarriers(RenderDeviceIndex deviceSelection, PipelineHandle* pipelineHandle, BarrierAccumulator* accumulator);

	void GenerateComputeDispatchBindingsBarriers(RenderDeviceIndex deviceSelection, PipelineHandle* handle, int pipelineIndex, BarrierAccumulator* accumulator);

	ShaderComputeLayout* GetComputeLayout(int shaderGraphIndex);

	void EndFrame(RenderDeviceIndex deviceSelection, int commandStreamIndex);

	int AddPipelineToRPGraphicsQueue(int psoIndex, AttachmentGraphInstanceIndex& frameGraphIndex, int renderPass);

	int AddPipelineToComputeQueue(int queueIndex, int psoIndex);

	int ReadData(RenderDeviceIndex deviceSelection, int handle, void* dest, int size, int offset);

	int CreatePipelineFromGraphAndSpec(RenderDeviceIndex deviceSelection, GenericPipelineStateInfo* stateInfo, ShaderGraph* graph, EntryHandle* outHandles, uint32_t outHandlePointer, AttachmentGraphInstance* graphInstance, uint32_t graphRenderPassIndex);

	int UpdateDriverMemory(void* data, int allocationIndex, int size, int allocOffset, TransferType transferType);

	int UpdateImageMemory(void* data, TextureIndex& textureIndex, size_t totalSize, int width, int height, int mipLevels, int mipStart, int layerCount, int layerStart, ImageViewAspectMask mask);

	int InsertTransferCommand(int allocationIndex, int size, int allocOffset, uint32_t fillValue);

	int UpdateShaderResourceArray(ShaderResourceSetHandle handle, int bindingindex, ShaderResourceType type, DeviceHandleArrayUpdate* resourceArrayData);

	int UpdateBufferResourceArray(ShaderResourceSetHandle handle, int bindingindex, ShaderResourceType type, BufferArrayUpdate* resourceArrayData);

	void SwapUpdateCommands();

	SamplerIndex CreateSampler(
		RenderDeviceIndex deviceSelection, uint32_t baseLod, uint32_t maxLod, 
		SamplerFilterMode minFilter, SamplerFilterMode magFilter, 
		SamplerAddressMode addressMode, SamplerMipmapMode mipmapMode, 
		CompareOp compareOp
	);

	ImageFormat FindSupportedBackBufferColorFormat(RenderPhysicalDeviceIndex physicalDeviceIndex, WindowIndex surfaceLevel, ImageFormat* requestedFormats, uint32_t requestSize);
	ImageFormat FindSupportedDepthFormat(RenderDeviceIndex deviceSelection, ImageFormat* requestedFormats, uint32_t requestSize);

	AttachmentGraphInstanceIndex CreateAttachmentGraph(RenderDeviceIndex deviceSelection, StringView* attachmentLayout);

	SwapChainIndex CreateSwapChainHandle(RenderDeviceIndex deviceSelection, WindowIndex surfaceIndex, ImageFormat mainBackBufferColorFormat, uint32_t width, uint32_t height);

	int CreateShaderGraphs(RenderDeviceIndex deviceSelection, StringView* shaderGraphLayouts, int shaderGraphLayoutsCount);

	int CreateGraphicRenderStateObject(RenderDeviceIndex deviceSelection, int shaderGraphIndex, int pipelineDescriptionIndex, AttachmentGraphInstanceIndex* frameGraphAttachments, int* perFrameRenderPassSelection, int frameGraphCount);
	int CreateComputePipelineStateObject(RenderDeviceIndex deviceSelection, int shaderGraphIndex);

	void ResetCommandList(int commandStreamIndex);

	void CreateGraphicsQueueForAttachments(AttachmentGraphInstanceIndex& frameGraphIndex, int renderPassIndex, uint32_t pipelineCount);

	int CreateComputeQueue();

	void AddComputeCommandQueue(int commandStreamIndex, int handleIndex);

	void AddAttachmentCommandQueue(int commandStreamIndex, AttachmentGraphInstanceIndex& handleIndex);

	int UploadFrameAttachmentResource(AttachmentGraphInstanceIndex& frameGraph, int resourceIndex, int perTextureViewIndex, ShaderResourceSetHandle handle, int bindingIndex, int textureStart);

	void PipelineUpdateIndirectCommandBuffer(int pipelineIndex, int allocationIndex);
	void PipelineUpdateVertexBuffer(int pipelineIndex, int allocationIndex, uint32_t vertexCount);
	void PipelineUpdateIndexBuffer(int pipelineIndex, int allocationIndex, uint32_t indexCount, uint32_t indexStride);
	void PipelineUpdateIndirectCountBuffer(int pipelineIndex, int allocationIndex);
	void PipelineUpdateDispatchCommands(int pipelineIndex, uint32_t x, uint32_t y, uint32_t z);

	BufferMemoryIndex CreateUniversalBuffer(RenderDeviceIndex deviceSelection, size_t size, MemoryType bufferMemoryType);

	int GetGPURequestedImageSizeAndAlignment(RenderDeviceIndex deviceSelection, uint32_t width, uint32_t height, uint32_t mipLevels, uint32_t layers, ImageFormat type, ImageUsageFlags usageFlags, size_t* actualImageSize, size_t* actualAlignment);

	ShaderResourceManagerIndex CreateDescriptorHeap(RenderDeviceIndex deviceSelection, DescriptorTypes* types, uint32_t* descriptorCountPerFrame, uint32_t numDescriptorTypesCount, uint32_t maxDescriptorSets, uint32_t maxShaderResourceSets);

	WindowIndex CreateWindowedSurface(OSWindowInternalData* windowData);

	int CreateHighLevelInstance(uint32_t vkDriverSpecificMemory, uint32_t vkDriverCacheSize, uint32_t instancePermanentSpecificMemory, uint32_t instanceCacheMemory);

	RenderPhysicalDeviceIndex CreatePhysicalDeviceAdapter(GPUFeatureRequest* requestedPhysicalFeatures, LogicalDeviceFeatures* requestedDeviceFeatures);

	int OpenPhysicalDevicePicker();

	RenderPhysicalDeviceIndex CreatePhysicalDeviceAdapterWithQuerying(GPUFeatureRequest* requestedPhysicalFeatures, LogicalDeviceFeatures* requestedDeviceFeatures);
	
	void ClosePhysicalDevicePicker();

	int CreatePerFrameStagingBuffers(RenderDeviceIndex deviceSelection, uint32_t bufferSize);

	int CreateResourceStatusActions(ResourceStatus* status, int numberOfCurrentActions, int numberOfCurrentStages, int numberOfCurrentLayouts);

	void InitializeResourceStatus(ResourceStatus* status, int numberOfCurrentActions, int numberOfCurrentStages, int numberOfCurrentLayouts, BarrierAction action, PipelineStage stage, ImageLayout imageLayout);

	void TransitionImageLayout(VKDevice* dev, TextureIndex& imageIndex, int perImageViewIndex, PipelineStage destBarrierStage, BarrierAction destBarrierAction, BarrierAccumulator* accumulator, int pipelineIndex);

	void TransitionImageLayout(VKDevice* dev, EntryHandle imageIndex, int mipStart, int mipCount, int totalMipCount, int layerStart, int layerCount,
		ImageViewAspectMask mask, ImageLayout requestedLayout, ResourceStatus* status,
		PipelineStage destBarrierStage, BarrierAction destBarrierAction, BarrierAccumulator* accumulator, int pipelineIndex);

	void InsertBufferBarrier(VKDevice* dev, int allocationIndex, PipelineStage destBarrierStage, ShaderResourceHeader* header, int pipelineIndex, BarrierAccumulator* accumulator);

	void InsertBufferBarrier(VKDevice* dev, int allocationIndex, PipelineStage destBarrierStage, BarrierAction destBarrierAction, BarrierAccumulator* accumulator);

	TextureIndex CreateAttachmentImage(
		uint32_t width, uint32_t height,
		uint32_t arrayLayers, uint32_t mipCount,
		ImageType imageType, int sampleCount,
		ImageFormat format, ImageUsageFlags usageFlags,
		DeviceSlabAllocator* attachmentAllocator, ImageLayout initialLayout,
		RenderDeviceIndex devSelection, ImageMemoryIndex imageMemoryPoolIndex, ResourceStatusType resourceType
	);

	int CreateAttachmentImageView(TextureIndex& textureIndex, uint32_t firstMip, uint32_t mipCount, uint32_t firstArrayLayer, uint32_t arrayLayerCount, ImageViewAspectMask mask, ImageLayout desiredLayout, RHIDevice* dev);

	int CreateAttachmentImageView(RenderDeviceIndex deviceSelection, AttachmentGraphInstanceIndex& attachmentGraphInstance, int attachmentResourceIndex, uint32_t firstMip, uint32_t mipCount, uint32_t firstArrayLayer, uint32_t arrayLayerCount, ImageViewAspectMask mask, ImageLayout desiredLayout);

	int CreateGPUCommandStream(int maxGPUCommandCount);

	void CreateDriverSpecificBarrierArenas(BarrierAccumulator* barrierAccumulator, int maxTextures, int maxAllocations);

	void InsertAccumulatedBarriers(RecordingBufferObject* rcb, BarrierAccumulator* accumulator);

	uint32_t PopBarrierAccumulator();

	void ReturnBarrierAccumulator(uint32_t returnIndex);

	IntraPassBarrier* GetIntraPassBarrier(BarrierAccumulator* accum, BarrierType type, int pipelineIndex, void* driverBarrierData);

	void InsertIntraPassBarrier(RecordingBufferObject* rbo, BarrierAccumulator* accum, int pipelineIndex);

	void ResetIntraBarrierAccumulator(BarrierAccumulator* accumulator);

	void* AllocateFromStorageAllocator(size_t size, size_t alignment);

	void* AllocateFromStorageAllocator(size_t size);

	void FreeFromStorageAllocator(void* address);

	RHIDevice* GetDeviceHandle(RenderDeviceIndex deviceSelection);

	void GetLastDeviceDriverError(RHIDevice* device, StringView messageHeader);

	void GetLastInstanceDriverError(StringView messageHeader);

	size_t GetNecessaryMemoryUsage(RenderInstanceCreateInfo* info);

	void WriteDeviceQuery(RHIDevice* device, RecordingBufferObject* rcbo, PipelineStage stage);

	void ToggleDeviceQueries(RenderDeviceIndex mainDeviceSelection);
	
	void DestroyPhysicalDeviceIndices(RenderPhysicalDeviceIndex handle);
	void DestroyLogicalDeviceIndices(RenderDeviceIndex handle);
	void DestroyWindowsSurfaces(WindowIndex& handle);
	void DestroySwapChain(SwapChainIndex& handle);
	void DestroyBufferHandle(BufferMemoryIndex& handle);
	void DestroyImagePool(ImageMemoryIndex& handle);
	void DestroyPipelineHandle(int handle);
	void DestroyAttachmentGraph(int handle);
	void DestroyAttachmentGraphInstance(RenderDeviceIndex mainLogicalDevice, AttachmentGraphInstanceIndex& handle);
	void DestroyRenderTargetQueue(int handle);
	void DestroyComputeQueue(int handle);
	void DestroyTextureResourceHandle(TextureIndex& handle);
	void DestroyTextureViewsResourceHandle(RenderDeviceIndex mainLogicalDevice, int handle);
	void DestroySamplerResourceHandle(RenderDeviceIndex mainLogicalDevice, SamplerIndex handle);
	void DestroyResourceStatus(ResourceIndex& handle);
	void DestroyPipelineInfo(int handle);
	void DestroyRenderPass(RenderDeviceIndex mainLogicalDevice, int handle);
	void DestroyRenderTarget(RenderDeviceIndex mainLogicalDevice, int handle);
	void DestroyShaderResourceTemplate(RenderDeviceIndex mainLogicalDevice, int handle);
	void DestroyAllocation(int handle);
	void DestroyDescriptorManager(ShaderResourceManagerIndex& handle);
	void DestroyGpuCommandStream(int handle);
	void DestroyShaderGraph(RenderDeviceIndex mainLogicalDevice, int handle);
	void DestroyGraphPipelineDescription(int handle);

	void CleanInitializePhysicalDeviceIndices(RenderPhysicalDeviceContainer* physicalDevice);
	void CleanInitializeRHIDevice(RHIDevice* logicalDevice);
	void CleanInitializeWindowsSurface(RenderWindowSpecificData* windowsSurface);
	void CleanInitializeSwapChain(RenderSwapchainData* swapChain);
	void CleanInitializeBufferHandle(RenderBufferDescription* bufferHandle);
	void CleanInitializeImagePool(ImagePoolDescription* imagePool);
	void CleanInitializePipelineHandle(PipelineHandle* pipelineHandle);
	void CleanInitializeAttachmentGraph(AttachmentGraph* attachmentGraph);
	void CleanInitializeAttachmentGraphsInstance(AttachmentGraphInstance* attachmentGraphsInstance);
	void CleanInitializeRenderTargetQueue(RenderQueue* renderTargetQueue);
	void CleanInitializeComputeQueue(ComputeQueue* computeQueue);
	void CleanInitializeTextureResourceHandle(RenderTextureDescription* textureResourceHandle);
	void CleanInitializeTextureViewsResourceHandle(RenderImageViewDescription* textureViewsResourceHandle);
	void CleanInitializeResourceStatus(ResourceStatus* resourceStatus);
	void CleanInitializePipelineInfo(GenericPipelineStateInfo* pipelineInfo);
	void CleanInitializeAllocation(RenderAllocation* allocation);
	void CleanInitializeDescriptorManager(ShaderResourceManager* descriptorManager);
	void CleanInitializeGpuCommandStream(GPUCommandStreamAllocator* gpuCommandStream);
	void CleanInitializeShaderGraph(ShaderGraph* shaderGraph);
	void CleanInitializeGraphPipeline(GraphPipelineDescription* desc);

	static constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 3;

	VKInstance *vkInstance = nullptr;

	RenderPhysicalDeviceContainer* physicalDeviceIndices{};

	RHIDevice* logicalDeviceIndices{};

	TypedPoolAllocator<RenderWindowSpecificData, WindowIndex> windowsSurfaces{};

	TypedPoolAllocator<RenderSwapchainData, SwapChainIndex> swapChains{};

	TypedPoolAllocator<RenderBufferDescription, BufferMemoryIndex> bufferHandles{};

	TypedPoolAllocator<ImagePoolDescription, ImageMemoryIndex> imagePools{};

	PoolAllocator<PipelineHandle> pipelineHandles{};
	
	PoolAllocator<AttachmentGraph> attachmentGraphs{};

	TypedPoolAllocator<AttachmentGraphInstance, AttachmentGraphInstanceIndex> attachmentGraphsInstances{};

	PoolAllocator<RenderQueue> renderTargetQueues{};

	PoolAllocator<ComputeQueue> computeQueues{};

	TypedPoolAllocator<RenderTextureDescription, TextureIndex> textureResourceHandles{};

	PoolAllocator<RenderImageViewDescription> textureViewsResourceHandles{};

	TypedPoolAllocator<EntryHandle, SamplerIndex> samplerResourceHandles{};

	TypedPoolAllocator<ResourceStatus, ResourceIndex> resourceStatuses{};

	PoolAllocator<GenericPipelineStateInfo> pipelineInfos{};

	PoolAllocator<EntryHandle> renderPasses{};

	PoolAllocator<EntryHandle> mainRenderTargets{};

	PoolAllocator<EntryHandle> shaderResourceTemplates{};

	PoolAllocator<RenderAllocation> allocations{};

	TypedPoolAllocator<ShaderResourceManager, ShaderResourceManagerIndex> descriptorManagers{};

	PoolAllocator<GPUCommandStreamAllocator> gpuCommandStreams{};

	PoolAllocator<GraphPipelineDescription> graphPipelineDescriptions{};
	
	ShaderGraphsHolder shaderGraphs;

	MemoryDriverTransferPool driverHostMemoryUpdater;

	TransferCommandsPool transferCommandPool;

	ShaderResourceUpdatePool descriptorUpdatePool;

	MemoryDriverTransferPool driverDeviceMemoryUpdater;

	ImageMemoryUpdateManager imageMemoryUpdateManager;

	RingAllocator* cacheAllocator;

	Allocator* storageAllocator;

	RingAllocator* updateCommandsCache;

	SlabAllocator* updateCommandBuffers[2];

	Logger* internalRendererLogger;

	uint32_t* barriersQueue = nullptr;

	BarrierAccumulator* barrierAccumulators = nullptr;

	uint32_t maxBarrierAccumulationCount = 0;
	uint32_t currentBarrierAccumulationTop = 0;

	int currentUpdateCommandBuffer = 0;
	uint32_t currentFrame = 0;
	uint32_t previousFrame = ~0U;
	uint32_t physicalDeviceCounter = 0;
	uint32_t logicalDeviceCounter = 0;
	uint32_t maxLogicalDevices = 0;
	uint32_t maxPhysicalDevices = 0;
	uint32_t physicalDevicesOnComputerPerDriver = 0;
};

namespace GlobalRenderer 
{
	extern RenderInstance gRenderInstance;
}

int DestroyDriverPhysicalDevice(VKInstance* instance, EntryHandle handle);
int DestroyDriverLogicalDevice(VKInstance* instance, EntryHandle handle);
int DestroyDriverWindowsSurface(VKInstance* device, EntryHandle handle);
int DestroyDriverSwapChain(RHIDevice* device, EntryHandle handle);
int DestroyDriverBufferHandle(RHIDevice* device, EntryHandle handle);
int DestroyDriverImagePool(RHIDevice* device, EntryHandle handle);
int DestroyDriverPipelineHandle(RHIDevice* device, EntryHandle handle);
int DestroyDriverImage(RHIDevice* device, EntryHandle handle);
int DestroyDriverImageView(RHIDevice* device, EntryHandle handle);
int DestroyDriverSamplerResourceHandle(RHIDevice* device, EntryHandle handle);
int DestroyOldStyleRenderPass(RHIDevice* device, EntryHandle handle);
int DestroyDriverMainRenderTarget(RHIDevice* device, EntryHandle handle);
int DestroyDriverShaderResourceLayout(RHIDevice* device, EntryHandle handle);
int DestroyDriverDescriptorHeap(RHIDevice* device, EntryHandle handle);
int DestroyDriverShader(RHIDevice* device, EntryHandle handle);
int DestoryDriverBufferView(RHIDevice* device, EntryHandle handle);
int DestroyDriverSemaphore(RHIDevice* device, EntryHandle handle);
