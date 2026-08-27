#pragma once

#include "allocator/AppAllocator.h"
#include "IndexTypes.h"
#include "math/VertexTypes.h"
#include "WindowManager.h"

#include "VKRenderInstance.h"

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

	void DestroySwapChainAttachments();

	int RecreateSwapChain(SwapChainIndex& swapChainIndex, uint32_t width, uint32_t height);

	int CreateAttachmentResources(AttachmentGraphInstanceIndex& graphIndex, int renderPassIndex, int imageCount, TextureIndex* backBufferTextureIds, uint32_t width, uint32_t height,
		RenderPassType rpType, AttachmentClear* clears, DeviceSlabAllocator* rsvAllocator, DeviceSlabAllocator* dsvAllocator, ImageMemoryIndex rsvPoolIndex, ImageMemoryIndex dsvPoolIndex);

	int CreateSwapChainAttachment(AttachmentGraphInstanceIndex& graphIndex, int renderPassIndex, SwapChainIndex swapChainIndex, AttachmentClear* clears, DeviceSlabAllocator* rsvAllocator, DeviceSlabAllocator* dsvAllocator, ImageMemoryIndex rsvPoolIndex, ImageMemoryIndex dsvPoolIndex);

	int CreatePerFrameAttachment(AttachmentGraphInstanceIndex& graphIndex, int renderPassIndex, int imageCount, uint32_t width, uint32_t height, AttachmentClear* clears, DeviceSlabAllocator* rsvAllocator, DeviceSlabAllocator* dsvAllocator, ImageMemoryIndex rsvPoolIndex, ImageMemoryIndex dsvPoolIndex);

	AttachmentGraphInstanceIndex CreateAttachmentGraphInstance(RenderDeviceIndex deviceSelection, AttachmentGraph* graph);

	int CreateRenderPass(AttachmentGraphInstance* graph);

	uint32_t BeginFrame(SwapChainIndex swapChainIndex);

	int SubmitFrame(SwapChainIndex swapChainIndex, uint32_t imageIndex);

	void WaitOnRender(RenderDeviceIndex deviceSelection);

	GenericRenderPipelineInfoIndex CreateGenericRenderPipelineDescription(StringView pipelineDescriptionFileName);

	void UploadHostTransfers(CommandRecorder* recorder);

	void UploadDescriptorsUpdates(CommandRecorder* recorder);

	void InvokeTransferCommands(CommandRecorder* recorder);

	void UploadImageMemoryTransfers(CommandRecorder* recorder);

	void UploadDeviceLocalTransfers(CommandRecorder* recorder);

	AllocationInstanceIndex GetAllocFromBuffer(BufferMemoryIndex bufferHandle, size_t structureSize, size_t copiesOfStructure, size_t alignment, AllocationType allocType, ComponentFormatType formatType, BufferAlignmentType bufferAlignmentType, AllocationInstanceIndex parentIndex, DeviceSlabAllocator* allocator);

	TextureIndex CreateImageHandle(
		RenderDeviceIndex deviceSelection,
		size_t gpuMemAddress,
		uint32_t width, uint32_t height,
		uint32_t mipLevels, uint32_t arrayLayers, ImageFormat format, ImageType imageType, ImageUsageFlags usageFlags, ImageMemoryIndex poolIndex);

	int CreateImageView(TextureIndex& imageHandle, int firstMip, int mipCount, int firstLayer, int layerCount, ImageViewAspectMask imageAspect, ImageLayout desiredImageLayoutUsage);

	ImageMemoryIndex CreateImagePool(RenderDeviceIndex deviceSelection, size_t size, ImageFormat format, int maxWidth, int maxHeight, ImageUsageFlags usageFlags, MemoryType memType);

	RenderDeviceIndex CreateLogicalDevice(LogicalDeviceCreateInfo* createInfo);

	uint32_t GetSwapChainHeight(SwapChainIndex& swapChainIndex);

	uint32_t GetSwapChainWidth(SwapChainIndex& swapChainIndex);

	PipelineHandleIndex CreateGraphicsPipelineObject(GraphicsIntermediaryPipelineInfo *info);

	PipelineHandleIndex CreateComputePipelineObject(ComputeIntermediaryPipelineInfo* info);

	void DrawScene(RenderDeviceIndex deviceSelection, GPUCommandStreamIndex& commandStreamIndex, uint32_t imageIndex);

	void IncreaseMSAA(AttachmentGraphInstanceIndex& frameGraph, int renderPassIndex);

	void DecreaseMSAA(AttachmentGraphInstanceIndex& frameGraph, int renderPassIndex);

	int CreateShaderResourceMap(RHIDevice* device, ShaderGraph *graph);

	ShaderResourceSetBuilder AllocateShaderResourceSet(ShaderResourceManagerIndex descriptorManagerIndex, RenderShaderGraphIndex& shaderGraphIndex, int targetSet, int setCount);

	int CreateShaderResourceSet(ShaderResourceManager* descriptorManager, DescriptorSetInstanceIndex& descriptorSet);

	void GeneratePipelineDescriptorBarriers(CommandRecorder* recorder, ShaderResourceSetHandle* descriptorid, int descriptorcount, PipelineHandleIndex& pipelineIndex);

	void GenerateDrawBindingsBarriers(CommandRecorder* recorder, PipelineHandle* pipelineHandle);

	void GenerateComputeDispatchBindingsBarriers(CommandRecorder* recorder, PipelineHandle* handle, PipelineHandleIndex& pipelineIndex);

	ShaderComputeLayout* GetComputeLayout(RenderShaderGraphIndex& shaderGraphIndex);

	void EndFrame(RenderDeviceIndex deviceSelection, GPUCommandStreamIndex& commandStreamIndex);

	int AddPipelineToRPGraphicsQueue(PipelineHandleIndex& psoIndex, AttachmentGraphInstanceIndex& frameGraphIndex, int renderPass);

	int AddPipelineToComputeQueue(PipelineQueueIndex& queueIndex, PipelineHandleIndex& psoIndex);

	int ReadData(AllocationInstanceIndex& handle, void* dest, int size, int offset);

	int UpdateDriverMemory(void* data, AllocationInstanceIndex& allocationIndex, int size, int allocOffset, TransferType transferType);

	int UpdateImageMemory(void* data, TextureIndex& textureIndex, size_t totalSize, int width, int height, int mipLevels, int mipStart, int layerCount, int layerStart, ImageViewAspectMask mask);

	int InsertTransferCommand(AllocationInstanceIndex& allocationIndex, int size, int allocOffset, uint32_t fillValue);

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

	AttachmentGraphInstanceIndex CreateAttachmentGraph(RenderDeviceIndex deviceSelection, StringView attachmentLayout);

	SwapChainIndex CreateSwapChainHandle(RenderDeviceIndex deviceSelection, WindowIndex surfaceIndex, ImageFormat mainBackBufferColorFormat, uint32_t width, uint32_t height);

	RenderShaderGraphIndex CreateShaderGraphInstance(RenderDeviceIndex deviceSelection, StringView shaderGraphLayouts);

	GeneratedPipelineInstanceIndex CreateGraphicRenderStateObject(RenderShaderGraphIndex& shaderGraphIndex, GenericRenderPipelineInfoIndex& pipelineDescriptionIndex, AttachmentGraphInstanceIndex* frameGraphAttachments, int* perFrameRenderPassSelection, int frameGraphCount);
	GeneratedPipelineInstanceIndex CreateComputePipelineStateObject(RenderShaderGraphIndex& shaderGraphIndex);

	void ResetCommandList(GPUCommandStreamIndex& commandStreamIndex);

	void CreateGraphicsQueueForAttachments(AttachmentGraphInstanceIndex& frameGraphIndex, int renderPassIndex);

	PipelineQueueIndex CreateComputeQueue();

	void AddComputeCommandQueue(GPUCommandStreamIndex& commandStreamIndex, PipelineQueueIndex& handleIndex);

	void AddAttachmentCommandQueue(GPUCommandStreamIndex& commandStreamIndex, AttachmentGraphInstanceIndex& handleIndex);

	int UploadFrameAttachmentResource(AttachmentGraphInstanceIndex& frameGraph, int resourceIndex, int perTextureViewIndex, ShaderResourceSetHandle handle, int bindingIndex, int textureStart);

	void PipelineUpdateIndirectCommandBuffer(PipelineHandleIndex& pipelineIndex, AllocationInstanceIndex& allocationIndex);
	void PipelineUpdateVertexBuffer(PipelineHandleIndex& pipelineIndex, AllocationInstanceIndex& allocationIndex, uint32_t vertexCount);
	void PipelineUpdateIndexBuffer(PipelineHandleIndex& pipelineIndex, AllocationInstanceIndex& allocationIndex, uint32_t indexCount, uint32_t indexStride);
	void PipelineUpdateIndirectCountBuffer(PipelineHandleIndex& pipelineIndex, AllocationInstanceIndex& allocationIndex);
	void PipelineUpdateDispatchCommands(PipelineHandleIndex& pipelineIndex, uint32_t x, uint32_t y, uint32_t z);

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

	void TransitionImageLayout(TextureIndex& imageIndex, int perImageViewIndex, PipelineStage destBarrierStage, BarrierAction destBarrierAction, BarrierAccumulator* accumulator, PipelineHandleIndex& pipelineIndex);

	void TransitionImageLayout(EntryHandle imageIndex, int mipStart, int mipCount, int totalMipCount, int layerStart, int layerCount,
		ImageViewAspectMask mask, ImageLayout requestedLayout, ResourceStatus* status,
		PipelineStage destBarrierStage, BarrierAction destBarrierAction, BarrierAccumulator* accumulator, PipelineHandleIndex& pipelineIndex);

	void InsertBufferBarrier(AllocationInstanceIndex& allocationIndex, PipelineStage destBarrierStage, ShaderResourceHeader* header, PipelineHandleIndex& pipelineIndex, BarrierAccumulator* accumulator);

	void InsertDrawCommandBufferBarrier(AllocationInstanceIndex& allocationIndex, PipelineStage destBarrierStage, BarrierAction destBarrierAction, BarrierAccumulator* accumulator);

	TextureIndex CreateAttachmentImage(
		uint32_t width, uint32_t height,
		uint32_t arrayLayers, uint32_t mipCount,
		ImageType imageType, int sampleCount,
		ImageFormat format, ImageUsageFlags usageFlags,
		DeviceSlabAllocator* attachmentAllocator, ImageLayout initialLayout,
		RenderDeviceIndex devSelection, ImageMemoryIndex imageMemoryPoolIndex, ResourceStatusType resourceType
	);

	int CreateAttachmentImageView(TextureIndex& textureIndex, uint32_t firstMip, uint32_t mipCount, uint32_t firstArrayLayer, uint32_t arrayLayerCount, ImageViewAspectMask mask, ImageLayout desiredLayout);

	int CreateAttachmentImageView(AttachmentGraphInstanceIndex& attachmentGraphInstance, int attachmentResourceIndex, uint32_t firstMip, uint32_t mipCount, uint32_t firstArrayLayer, uint32_t arrayLayerCount, ImageViewAspectMask mask, ImageLayout desiredLayout);

	GPUCommandStreamIndex CreateGPUCommandStream(int maxGPUCommandCount);

	void CreateDriverSpecificBarrierArenas(BarrierAccumulator* barrierAccumulator, int maxTextures, int maxAllocations);

	uint32_t PopBarrierAccumulator();

	void ReturnBarrierAccumulator(uint32_t returnIndex);

	IntraPassBarrier* GetIntraPassBarrier(BarrierAccumulator* accum, BarrierType type, PipelineHandleIndex& pipelineIndex, void* driverBarrierData);

	void ResetIntraBarrierAccumulator(BarrierAccumulator* accumulator);

	void* AllocateFromStorageAllocator(size_t size, size_t alignment);

	void* AllocateFromStorageAllocator(size_t size);

	void FreeFromStorageAllocator(void* address);

	RHIDevice* GetDeviceHandle(RenderDeviceIndex deviceSelection);

	void GetLastDeviceDriverError(RHIDevice* device, StringView messageHeader);

	void GetLastInstanceDriverError(StringView messageHeader);

	size_t GetNecessaryMemoryUsage(RenderInstanceCreateInfo* info);

	void WriteDeviceQuery(CommandRecorder* recorder, PipelineStage stage);

	void ToggleDeviceQueries(RenderDeviceIndex mainDeviceSelection);
	
	void DestroyPhysicalDeviceIndices(RenderPhysicalDeviceIndex handle);
	void DestroyLogicalDeviceIndices(RenderDeviceIndex handle);
	void DestroyWindowsSurfaces(WindowIndex& handle);
	void DestroySwapChain(SwapChainIndex& handle);
	void DestroyBufferHandle(BufferMemoryIndex& handle);
	void DestroyImagePool(ImageMemoryIndex& handle);
	void DestroyPipelineHandle(PipelineHandleIndex& handle);
	void DestroyAttachmentGraph(int handle);
	void DestroyAttachmentGraphInstance(AttachmentGraphInstanceIndex& handle);
	void DestroyRenderTargetQueue(PipelineQueueIndex& handle);
	void DestroyComputeQueue(PipelineQueueIndex& handle);
	void DestroyTextureResourceHandle(TextureIndex& handle);
	void DestroyTextureViewsResourceHandle(RenderDeviceIndex mainLogicalDevice, TextureViewIndex& handle);
	void DestroySamplerResourceHandle(RenderDeviceIndex mainLogicalDevice, SamplerIndex handle);
	void DestroyResourceStatus(ResourceIndex& handle);
	void DestroyPipelineInfo(GenericRenderPipelineInfoIndex& handle);
	void DestroyRenderPass(OldStyleRenderPassIndex& handle);
	void DestroyRenderTarget(DriverRenderTargetIndex& handle);
	void DestroyShaderResourceTemplate(ShaderResourceTemplateInstanceIndex& handle);
	void DestroyAllocation(AllocationInstanceIndex& handle);
	void DestroyDescriptorManager(ShaderResourceManagerIndex& handle);
	void DestroyGpuCommandStream(GPUCommandStreamIndex& handle);
	void DestroyShaderGraph(RenderShaderGraphIndex& handle);
	void DestroyGraphPipelineDescription(GeneratedPipelineInstanceIndex& handle);
	void DestroyShaderResourceSet(ShaderResourceSetHandle& handle);

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

	RHIInstance vkInstance;

	RenderPhysicalDeviceContainer* physicalDeviceIndices{};

	RHIDevice* logicalDeviceIndices{};

	TypedPoolAllocator<RenderWindowSpecificData, WindowIndex> windowsSurfaces{};

	TypedPoolAllocator<RenderSwapchainData, SwapChainIndex> swapChains{};

	TypedPoolAllocator<RenderBufferDescription, BufferMemoryIndex> bufferHandles{};

	TypedPoolAllocator<ImagePoolDescription, ImageMemoryIndex> imagePools{};

	TypedPoolAllocator<PipelineHandle, PipelineHandleIndex> pipelineHandles{};
	
	PoolAllocator<AttachmentGraph> attachmentGraphs{};

	TypedPoolAllocator<AttachmentGraphInstance, AttachmentGraphInstanceIndex> attachmentGraphsInstances{};

	TypedPoolAllocator<RenderQueue, PipelineQueueIndex> renderTargetQueues{};

	TypedPoolAllocator<ComputeQueue, PipelineQueueIndex> computeQueues{};

	TypedPoolAllocator<RenderTextureDescription, TextureIndex> textureResourceHandles{};

	TypedPoolAllocator<RenderImageViewDescription, TextureViewIndex> textureViewsResourceHandles{};

	TypedPoolAllocator<EntryHandle, SamplerIndex> samplerResourceHandles{};

	TypedPoolAllocator<ResourceStatus, ResourceIndex> resourceStatuses{};

	TypedPoolAllocator<GenericPipelineStateInfo, GenericRenderPipelineInfoIndex> pipelineInfos{};

	TypedPoolAllocator<RenderOldStyleVulkanRenderPassInfo, OldStyleRenderPassIndex> renderPasses{};

	TypedPoolAllocator<RenderTargetInfo, DriverRenderTargetIndex> mainRenderTargets{};

	TypedPoolAllocator<RenderShaderResourceTemplateInfo, ShaderResourceTemplateInstanceIndex> shaderResourceTemplates{};

	TypedPoolAllocator<RenderAllocation, AllocationInstanceIndex> allocations{};

	TypedPoolAllocator<ShaderResourceManager, ShaderResourceManagerIndex> descriptorManagers{};

	TypedPoolAllocator<GPUCommandStreamAllocator, GPUCommandStreamIndex> gpuCommandStreams{};

	TypedPoolAllocator<GraphPipelineDescription, GeneratedPipelineInstanceIndex> graphPipelineDescriptions{};
	
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

int DestroyDriverPhysicalDevice(RHIInstance* instance, EntryHandle handle);
int DestroyDriverLogicalDevice(RHIInstance* instance, EntryHandle handle);
int DestroyDriverWindowsSurface(RHIInstance* instance, EntryHandle handle);
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
int DestroyDescriptorSet(RHIDevice* device, EntryHandle handle);


void ResetCommandPool(CommandRecorder* recorder);

void BeginCommandRecording(CommandRecorder* recorder);

void EndCommandRecording(CommandRecorder* recorder);

void ResetDeviceQueries(CommandRecorder* recorder, EntryHandle queryPoolIndex, uint32_t firstQuery, uint32_t queryCount);

void BindComputePipelineCmd(CommandRecorder* recorder, EntryHandle pipelineHandle);

void BindComputeDescriptorSetsCmd(CommandRecorder* recorder, EntryHandle handle, uint32_t descriptorSetIndex, uint32_t setCount, uint32_t firstSet, uint32_t dynamicOffsetCount, uint32_t* dynamicOffsets);

void PushConstantsCmd(CommandRecorder* recorder, uint32_t offset, uint32_t size, ShaderStageType stage, void* data);

void DispatchIndirectCmd(CommandRecorder* recorder, EntryHandle indirectBufferIndex, size_t offset);

void DispatchCmd(CommandRecorder* recorder, uint32_t x, uint32_t y, uint32_t z);

void BeginRenderPassCmd(CommandRecorder* recorder, EntryHandle renderTargetInfo, uint32_t imageIndex, AttachmentClear* clears, uint32_t clearCount, Allocator* clearsAllocators);

void EndRenderPassCmd(CommandRecorder* recorder);

void SetViewportCmd(CommandRecorder* recorder, float x, float y, float width, float height, float minDepth, float maxDepth);

void SetViewportCmd(CommandRecorder* recorder, EntryHandle renderTargetIndex, float minDepth, float maxDepth);

void SetScissorCmd(CommandRecorder* recorder, int32_t x, int32_t y, uint32_t width, uint32_t height);

void SetScissorCmd(CommandRecorder* recorder, EntryHandle renderTargetIndex);

void BindGraphicsPipelineCmd(CommandRecorder* recorder, EntryHandle pipelineHandle);

void BindGraphicsDescriptorSetsCmd(CommandRecorder* recorder, EntryHandle handle, uint32_t descriptorSetIndex, uint32_t setCount, uint32_t firstSet, uint32_t dynamicOffsetCount, uint32_t* dynamicOffsets);

void BindVertexBufferCmd(CommandRecorder* recorder, EntryHandle bufferHandle, uint32_t firstBinding, uint32_t bindingCount, size_t* offsets);

void BindIndexBufferCmd(CommandRecorder* recorder, EntryHandle bufferHandle, size_t offset, int indexType);

void DrawIndexedIndirectCountCmd(CommandRecorder* recorder, EntryHandle indirectBufferIndex, EntryHandle indirectCountBufferIndex, size_t indirectOffset, size_t countOffset, uint32_t maxDrawCount);

void DrawIndexedIndirectCmd(CommandRecorder* recorder, EntryHandle indirectBufferIndex, uint32_t drawCount, size_t indirectOffset);

void DrawIndirectCountCmd(CommandRecorder* recorder, EntryHandle indirectBufferIndex, EntryHandle indirectCountBufferIndex, size_t indirectOffset, size_t countOffset, uint32_t maxDrawCount);

void DrawIndirectCmd(CommandRecorder* recorder, EntryHandle indirectBufferIndex, uint32_t drawCount, size_t indirectOffset);

void DrawIndexedCmd(CommandRecorder* recorder, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance);

void DrawCmd(CommandRecorder* recorder, uint32_t firstVertex, uint32_t vertexCount, uint32_t firstInstance, uint32_t instanceCount);

void WriteTimeStamp(CommandRecorder* recorder, EntryHandle queryPoolIndex, uint32_t queryOffset, PipelineStage stage);

int WriteHostBufferBatch(RHIDevice* device, EntryHandle bufferHandle, void** cpuDataLocations, size_t* sizesOfDataLocations, size_t* offsetsIntoHostMemory, size_t numberOfLocations, size_t mappableSize, size_t mappableStart);

int WriteDeviceBufferBatch(CommandRecorder* recorder, EntryHandle bufferHandle, EntryHandle stagingBufferHandle,
	void** cpuDataLocations, size_t* sizesOfDataLocations, size_t* offsetsIntoStagingMemory, size_t* offsetsIntoDeviceMemory, size_t numberOfCopies, size_t mappableSize);

void FillBuffer(CommandRecorder* recorder, EntryHandle bufferHandle, size_t regionSize, size_t regionOffset, uint32_t fillVal);

void MakeAndBindDriverAccumulatedBarriers(CommandRecorder* recorder);

void MakeAndBindDriverIntraPassBarriers(CommandRecorder* recorder, PipelineHandleIndex& pipelineIndex);

int UploadImageDataToDeviceMemory(
	CommandRecorder* recorder, 
	EntryHandle textureHandle, EntryHandle stagingBufferHandle,
	void* cpuImageData, size_t totalUploadSize, size_t imageDataOffsetInStaging,
	uint32_t writeWidth, uint32_t writeHeight,
	uint32_t mipLevels, uint32_t layersCount,
	ImageFormat imageFormat, ImageViewAspectMask aspectMask
);

void GetDriverCommandBufferObject(CommandRecorder* recorder, EntryHandle commandBufferIndex);

int WaitOnSwapChain(RHIDevice* device, EntryHandle swapChainIndex);

EntryHandle GetSwapChainViewHandles(RHIDevice* device, EntryHandle swapChainIndex, uint32_t imageIndex);

uint32_t GetSwapChainImageCount(RHIDevice* device, EntryHandle swapChainIndex);

int WaitOnDevice(RHIDevice* device, uint64_t timeout);

int GetImageMemorySizeAndAlignment(RHIDevice* device, DriverImageCreationInfo* info);

EntryHandle CreateDriverImageHandle(RHIDevice* device, DriverImageCreationInfo* info);

EntryHandle CreateDriverImageViewHandle(RHIDevice* device, DriverImageViewCreationInfo* info);

EntryHandle CreateDriverBufferView(RHIDevice* device, EntryHandle bufferHandle, ComponentFormatType format, size_t viewSize, size_t offset, uint32_t copiesOfRangeSize);

EntryHandle CreateDriverImageMemoryPool(RHIDevice* device, DriverImageMemoryPoolCreationInfo* info);

EntryHandle CreateDriverSampler(RHIDevice* device, DriverSamplerCreationInfo* info);

EntryHandle CreateShaderCode(RHIDevice* device, char* shaderData, size_t length, ShaderStageType type);

int CreateDriverGraphicsPipeline(RHIDevice* device,
	GraphicsPipelineCreationInfo* info,
	EntryHandle* outHandles, uint32_t outHandlePointer
);

EntryHandle CreateDriverComputePipeline(RHIDevice* device, ComputePipelineCreationInfo* info);

EntryHandle CreateDriverShaderResourceLayout(RHIDevice* device, ShaderResourceSetTemplateCreator* creator);

int CreateDriverSwapChainData(RHIDevice* rhiDevice, EntryHandle swapChainIndex, uint32_t width, uint32_t height, bool recreate);

EntryHandle CreateDriverDescriptorHeap(RHIDevice* device, DescriptorTypes* types, uint32_t* descriptorCountPerFrame, uint32_t numDescriptorTypesCount, uint32_t maxDescriptorSets, uint32_t frameInFlight);

EntryHandle CreateDriverBufferMemoryPool(RHIDevice* device, size_t poolSize, BufferUsage usage, MemoryType memoryType);

int ReadDriverHostData(RHIDevice* device, EntryHandle bufferHandle, void* dataOut, size_t size, size_t offset);

EntryHandle CreateDriverSwapChain(RHIDevice* device, uint32_t requestedImageCount, uint32_t maxFramesInFlight, ImageFormat requestedFormat, EntryHandle renderSurfaceIndex);

EntryHandle* CreateDriverSemaphores(RHIDevice* device, uint32_t semaphoreCount);

int ReadBackQueryResults(RHIDevice* device, EntryHandle queryPoolIndex, uint32_t queryOffset, uint32_t queryCount, void* queryResults, size_t queryResultsSizeBytes, size_t individualQueryResultSize, int queryFlags);

int FindDriverSupportedDepthFormat(RHIInstance* instance, EntryHandle gpuIndex, ImageFormat format);

int FindDriverSupportBackbufferFormat(RHIInstance* instance, EntryHandle gpuIndex, EntryHandle surfaceIndex, ImageFormat requestedFormat);

EntryHandle CreateDriverShaderResourceSet(RHIDevice* device, ShaderResourceSetInstanceCreator* creator, EntryHandle descriptorHeapIndex, EntryHandle descriptorLayoutHandle, uint32_t numberOfSubResourceHandle, uint32_t variableSizeRequestForLastDescriptor);

int UpdateDriverShaderResourceSet(RHIDevice* device, ShaderResourceSetInstanceCreator* creator, EntryHandle descriptorSetHandle);

void CreateDriverInstanceMemory(RHIInstance* instance, Allocator* allocator);

void DestroyDriverInstance(RHIInstance* instance);

int CreateDriverInstance(RHIInstance* instance, WindowManagementType windowType, uint32_t driverSpecificMemory, uint32_t driverCacheSize, uint32_t instancePermanentSpecificMemory, uint32_t instanceCacheMemory, Logger* logger, Allocator* allocator);

EntryHandle CreateDriverWindowSurface(RHIInstance* instance, OSWindowInternalData* windowData);

EntryHandle CreateDriverRenderTarget(RHIDevice* device, EntryHandle renderPassIndex, uint32_t framebufferCount, uint32_t width, uint32_t height, uint32_t wOffset, uint32_t hOffset);

int CreateDriverRenderTargetFrameBuffer(RHIDevice* device, EntryHandle renderTargetHandle, EntryHandle* imageViews,  uint32_t attachmentsCount, uint32_t frameBufferIndex, uint32_t width, uint32_t height);

size_t GetDriverImageMemoryBarrierSize();
size_t GetDriverBufferMemoryBarrierSize();
size_t GetDriverImageMemoryBarrierAlign();
size_t GetDriverBufferMemoryBarrierAlign();
size_t GetDriverIndexedIndirectDrawCommandSize();
size_t GetDriverIndirectDrawCommandSize();
size_t GetDriverIndirectDispatchCommandSize();
size_t GetDriverIndexedIndirectDrawCommandAlign();
size_t GetDriverIndirectDrawCommandAlign();
size_t GetDriverIndirectDispatchCommandAlign();