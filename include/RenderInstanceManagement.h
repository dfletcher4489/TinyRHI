#pragma once

#include <atomic>
#include <stdint.h>
#include "allocator/AppAllocator.h"
#include "CommonRenderTypes.h"
#include "RenderInstanceHandleTypes.h"
#include "ShaderResourceSet.h"

enum class AllocationType
{
	STATIC = 0,
	PERFRAME = 1,
	PERDRAW = 2
};

enum class TransferType
{
	CACHED = 0,
	MEMORY = 1,
};

struct ShaderResourceUpdate
{
	void* data;
	ShaderResourceType type;
	ShaderResourceManagerIndex descriptorManagerIndex;
	DescriptorSetInstanceIndex descriptorSet;
	int bindingIndex;
	int copyCount;
	int dataSize;
};

enum class DeviceHandleArrayUpdateType
{
	TEXTURE_VIEW_UPDATE = 1,
	TEXTURE_VIEW_SAMPLER_UPDATE = 2,
	SAMPLER_UPDATE = 3,
};

struct DeviceHandleArrayUpdateTextureView
{
	TextureIndex imageHandle;
	int viewIndex;
};

struct DeviceHandleArrayUpdateTextureViewSampler
{
	TextureIndex imageHandle;
	int viewIndex;
	SamplerIndex samplerHandle;
};

struct DeviceHandleArrayUpdate
{
	DeviceHandleArrayUpdateType updateType;
	int resourceDstBegin;
	int resourceCount;
	void* resourceHandles;
};

struct BufferArrayUpdate
{
	int resourceDstBegin;
	int allocationCount;
	AllocationInstanceIndex* allocationIndices;
};

struct GraphicsIntermediaryPipelineInfo
{
	AllocationInstanceIndex vertexBufferHandle;
	uint32_t vertexCount;
	GeneratedPipelineInstanceIndex pipelinename;
	uint32_t descCount;
	ShaderResourceSetHandle* descriptorsetid;
	AllocationInstanceIndex indexBufferHandle;
	uint32_t indexCount;
	uint32_t instanceCount;
	uint32_t indexSize;
	AllocationInstanceIndex indirectAllocation;
	int indirectDrawCount;
	AllocationInstanceIndex indirectCountAllocation;
};

struct ComputeIntermediaryPipelineInfo
{
	uint32_t x;
	uint32_t y;
	uint32_t z;
	GeneratedPipelineInstanceIndex pipelinename;
	uint32_t descCount;
	AllocationInstanceIndex indirectDispatchAllocation;
	ShaderResourceSetHandle* descriptorsetid;
};

struct BufferMemoryTransferRegion
{
	void* data;
	int size;
	int copyCount;
	AllocationInstanceIndex allocationIndex;
	int allocoffset;
};

struct TextureMemoryRegion
{
	void* data;
	size_t totalSize;
	size_t currentPointerUpdate;
	TextureIndex textureIndex;
	int width;
	int height;
	int mipLevels;
	int layerCount;
	int layerStart;
	int mipStart;
	ImageViewAspectMask transferMask;
};

struct TransferCommand
{
	int fillVal;
	int size;
	int offset;
	AllocationInstanceIndex allocationIndex;
	int copycount;
};

struct RenderAllocation
{
	size_t offset;
	size_t deviceAllocSize;
	size_t requestedSize;
	size_t alignment;
	EntryHandle viewIndex;
	AllocationType allocType;
	ComponentFormatType formatType;
	int structureCopies;
	BufferMemoryIndex memIndex;
	ResourceIndex resourceStatus;
	AllocationInstanceIndex parentAllocation;
	RenderDeviceIndex deviceIndex;
	int pad;
};

enum AppPipelineHandleType
{
	COMPUTESO,
	GRAPHICSO,
};

struct PipelineHandle
{
	int group;
	int numHandles;
	GeneratedPipelineInstanceIndex pipelineIdentifierGroup;
	ShaderResourceSetHandle resourceSets[16];
	int resourceSetCount;
	AllocationInstanceIndex vertexBufferHandle;
	uint32_t vertexCount;
	AllocationInstanceIndex indexBufferHandle;
	uint32_t indexCount;
	uint32_t pushRangeCount;
	uint32_t instanceCount;
	uint32_t indexSize;
	uint32_t indirectDrawCount;
	AllocationInstanceIndex indirectBufferHandle;
	AllocationInstanceIndex indirectCountBufferHandle;
	uint32_t x;
	uint32_t y;
	uint32_t z;
	AllocationInstanceIndex indirectDispatchCommandHandle;
};

enum GPUCommandStreamType
{
	ATTACHMENT_COMMANDS = 1,
	COMPUTE_QUEUE_COMMANDS = 2,
};

struct GPUCommand
{
	GPUCommandStreamType streamType;
	union {
		PipelineQueueIndex indexForComputeQueue;
		AttachmentGraphInstanceIndex attachmentGraphIndex;
	} commandIndex;
};

enum class DriverUpdateType
{
	MEMORYUPDATE = 1,
	RESOURCEUPDATE = 2,
	IMAGEMEMORYUPDATE = 3,
	TRANSFERCOMMAND = 4
};

struct RenderDriverUpdateCommandHeader
{
	DriverUpdateType updateType;
};

struct RenderDriverUpdateCommandMemory : public RenderDriverUpdateCommandHeader
{
	AllocationInstanceIndex allocationIndex;
	int size;
	int allocOffset;
	int copiesWithin;
	int pad1;
	void* data;

	RenderDriverUpdateCommandHeader* GetNext()
	{
		return (RenderDriverUpdateCommandHeader*)((uintptr_t)this + sizeof(RenderDriverUpdateCommandMemory));
	}
};

struct RenderDriverUpdateCommandFill : public RenderDriverUpdateCommandHeader
{
	AllocationInstanceIndex allocationIndex;
	int size;
	int allocOffset;
	int copiesWithin;
	uint32_t fillValue; 
	int pad[2];

	RenderDriverUpdateCommandHeader* GetNext()
	{
		return (RenderDriverUpdateCommandHeader*)((uintptr_t)this + sizeof(RenderDriverUpdateCommandFill));
	}
};

struct RenderDriverUpdateCommandImage : public RenderDriverUpdateCommandHeader
{
	int width; 
	int height; 
	int mipLevels; 
	int layersCount;
	TextureIndex textureIndex;
	int mipStart;
	int layerStart;
	ImageViewAspectMask mask;
	int pad1[2];
	void* data;
	size_t totalSize;

	RenderDriverUpdateCommandHeader* GetNext()
	{
		return (RenderDriverUpdateCommandHeader*)((uintptr_t)this + sizeof(RenderDriverUpdateCommandImage));
	}

};

#define PACK_DESCRIPTOR_MANAGER_INDEX(x) ((x) << 20)
#define UNPACK_DESCRIPTOR_MANAGER_INDEX(x) ((x&0xFFF00000) >> 20)
#define PACK_DESCRIPTOR_SET_INDEX(x) ((x&0x000FFFFF))
#define UNPACK_DESCRIPTOR_SET_INDEX(x) ((x&0x000FFFFF))

struct RenderDriverUpdateCommandResource: public RenderDriverUpdateCommandHeader
{
	int descriptorIdManagerIndex; 
	int bindingindex; 
	ShaderResourceType type;
	int cachedDataSize;
	int copies;
	void* data;

	RenderDriverUpdateCommandHeader* GetNext()
	{
		return (RenderDriverUpdateCommandHeader*)((uintptr_t)this + sizeof(RenderDriverUpdateCommandResource));
	}

};

static_assert(sizeof(RenderDriverUpdateCommandMemory) % 32 == 0);
static_assert(sizeof(RenderDriverUpdateCommandResource) % 32 == 0);
static_assert(sizeof(RenderDriverUpdateCommandImage) % 32 == 0);
static_assert(sizeof(RenderDriverUpdateCommandFill) % 32 == 0);

struct MemoryDriverTransferPool
{
	BufferMemoryTransferRegion* transferRegions;
	int* regionLinks;
	int linkHead = -1;
	int linkCount = 0;
	int ddsRegionAlloc = 0;
	int ddsRegionSize = 0;

	int GetSize(int numberOfRegions)
	{
		return (sizeof(BufferMemoryTransferRegion) + sizeof(int)) * numberOfRegions;
	}

	void AllocateList(void* poolData, int poolSize)
	{
		ddsRegionSize = (int)((float)poolSize/(float)(sizeof(BufferMemoryTransferRegion) + sizeof(int)));

		transferRegions = (BufferMemoryTransferRegion*)poolData;

		regionLinks = (int*)(transferRegions + ddsRegionSize);
	}

	int Create(void* data, int size, AllocationInstanceIndex& allocationIndex, int allocOffset, int copies)
	{
		int link = Find(allocationIndex, allocOffset);
		
		BufferMemoryTransferRegion* region = nullptr;
		
		if (link < 0)
		{
			int regionAlloc = ddsRegionAlloc;

			ddsRegionAlloc = (ddsRegionAlloc + 1) % ddsRegionSize;

			region = &transferRegions[regionAlloc];

			regionLinks[regionAlloc] = -1;
		
			region->allocationIndex = allocationIndex;
			region->allocoffset = allocOffset;
			region->size = size;
			region->data = data;

			Insert(regionAlloc);
		}
		else
		{
			region = &transferRegions[link];
			region->data = data;
			region->allocoffset = allocOffset;
			region->size = size;
		}

		region->copyCount = copies;
		return 0;
	}

	void Insert(int newlink)
	{
		int* test = &linkHead;
		while ((*test >= 0) && (transferRegions[(*test)].allocationIndex <= transferRegions[newlink].allocationIndex))
		{
			test = &(regionLinks[(*test)]);
		}
		regionLinks[newlink] = *test;
		*test = newlink;
		linkCount++;
	}
	
	int Find(AllocationInstanceIndex& allocationIndex, int offset)
	{
		int link = linkHead;
		while (link >= 0 && (transferRegions[link].allocationIndex != allocationIndex || transferRegions[link].allocoffset != offset))
		{
			link = regionLinks[link];
		}
		return (link);
	}

	int PopLink(BufferMemoryTransferRegion* outputRegion, int link, int** popprev)
	{
		if (link < 0 || link >= ddsRegionSize) return -1;
		outputRegion->allocationIndex = transferRegions[link].allocationIndex;
		outputRegion->size = transferRegions[link].size;
		outputRegion->copyCount = transferRegions[link].copyCount;
		outputRegion->data = transferRegions[link].data;
		outputRegion->allocoffset = transferRegions[link].allocoffset;
		int linkRet = regionLinks[link];
		if (transferRegions[link].copyCount > 1)
		{
			transferRegions[link].copyCount--;
			*popprev = &regionLinks[link];
		}
		else
		{
			*(*popprev) = linkRet;
			linkCount--;
			transferRegions[link].allocationIndex = -1;
			transferRegions[link].size = 0;
			transferRegions[link].copyCount = -1;
			transferRegions[link].allocoffset = -1;
			transferRegions[link].data = nullptr;
			regionLinks[link] = -1;
		}
		return linkRet;
	}
};

struct TransferCommandsPool
{
	TransferCommand* transferRegions;
	int* regionLinks;
	int linkHead = -1;
	int linkCount = 0;
	int ddsRegionAlloc = 0;
	int ddsRegionSize = 0;

	int GetSize(int numberOfRegions)
	{
		return (sizeof(TransferCommand) + sizeof(int)) * numberOfRegions;
	}

	void AllocateList(void* poolData, int poolSize)
	{
		ddsRegionSize = (int)((float)poolSize / (float)(sizeof(TransferCommand) + sizeof(int)));

		transferRegions = (TransferCommand*)poolData;

		regionLinks = (int*)(transferRegions + ddsRegionSize);
	}

	int Create(AllocationInstanceIndex& allocationIndex, int size, int allocOffset, uint32_t fillValue, int copies)
	{
		int link = Find(allocationIndex, allocOffset);
		TransferCommand* region = nullptr;

		if (link < 0)
		{
			int regionAlloc = ddsRegionAlloc;

			ddsRegionAlloc = (ddsRegionAlloc + 1) % ddsRegionSize;

			region = &transferRegions[regionAlloc];

			regionLinks[regionAlloc] = -1;

			region->allocationIndex = allocationIndex;
			region->offset = allocOffset;
			region->size = size;
			region->fillVal = fillValue;

			Insert(regionAlloc);
		}
		else
		{
			region = &transferRegions[link];
			region->fillVal = fillValue;
			region->offset = allocOffset;
			region->size = size;
		}

		region->copycount = copies;

		return 0;
	}

	void Insert(int newlink)
	{
		int* test = &linkHead;
		while ((*test >= 0) && (transferRegions[*test].allocationIndex <= transferRegions[newlink].allocationIndex))
		{
			test = &(regionLinks[*test]);
		}
		regionLinks[newlink] = *test;
		*test = newlink;
		linkCount++;
	}

	int Find(AllocationInstanceIndex& allocationIndex, int offset)
	{
		int link = linkHead;
		while ((link >= 0) && (transferRegions[link].allocationIndex != allocationIndex || transferRegions[link].offset != offset))
		{
			link = regionLinks[link];
		}
		return link;
	}

	int PopLink(TransferCommand* outputRegion, int link, int** popprev)
	{
		if (link < 0 || link >= ddsRegionSize) return -1;
		outputRegion->allocationIndex = transferRegions[link].allocationIndex;
		outputRegion->size = transferRegions[link].size;
		outputRegion->copycount = transferRegions[link].copycount;
		outputRegion->fillVal = transferRegions[link].fillVal;
		outputRegion->offset = transferRegions[link].offset;
		int linkRet = regionLinks[link];
		if (transferRegions[link].copycount > 1)
		{
			transferRegions[link].copycount--;
			*popprev = &regionLinks[link];
		}
		else
		{
			*(*popprev) = linkRet;
			linkCount--;
			TransferCommand* region = &transferRegions[link];
			region->allocationIndex = -1;
			region->size = 0;
			region->copycount = -1;
			region->offset = -1;
			region->fillVal = 0;
		}
		return linkRet;
	}
};

struct ImageMemoryUpdateManager
{
	TextureMemoryRegion* transferRegions;
	int* regionLinks;
	int linkHead = -1;
	int linkCount = 0;
	int ddsRegionAlloc = 0;
	int ddsRegionSize = 0;

	int GetSize(int numberOfRegions)
	{
		return (sizeof(TextureMemoryRegion) + sizeof(int)) * numberOfRegions;
	}

	void AllocateList(void* poolData, int poolSize)
	{
		ddsRegionSize = (int)((float)poolSize / (float)(sizeof(TextureMemoryRegion) + sizeof(int)));

		transferRegions = (TextureMemoryRegion*)poolData;

		regionLinks = (int*)(transferRegions + ddsRegionSize);
	}

	int Create(void* data, TextureIndex textureIndex, size_t totalSize, int width, int height, int mipLevels, int layerCount, int mipStart, int layerStart, ImageViewAspectMask mask)
	{
		int link = Find(textureIndex);
		TextureMemoryRegion* region = nullptr;

		if (link >= 0) return -1;
		
		int regionAlloc = ddsRegionAlloc;

		ddsRegionAlloc = (ddsRegionAlloc + 1) % ddsRegionSize;

		region = &transferRegions[regionAlloc];

		regionLinks[regionAlloc] = -1;

		region->data = data;
		region->height = height;
		region->width = width;
		region->totalSize = totalSize;
		region->mipLevels = mipLevels;
		region->textureIndex = textureIndex;
		region->currentPointerUpdate = 0;
		region->layerCount = layerCount;
		region->mipStart = mipStart;
		region->layerStart = layerStart;
		region->transferMask = mask;

		Insert(regionAlloc);
		
		return 0; 
	}

	void Insert(int newlink)
	{
		regionLinks[newlink] = linkHead;
		linkHead = newlink;
		linkCount++;
	}

	int Find(TextureIndex textureIndex)
	{
		return (-1);
	}

	int PopLink(TextureMemoryRegion* outputRegion, int link)
	{
		if (link < 0 || link >= ddsRegionSize)
			return -1;

		int regionIndex = link;
		TextureMemoryRegion* src = &transferRegions[regionIndex];

		outputRegion->data = src->data;
		outputRegion->totalSize = src->totalSize;
		outputRegion->textureIndex = src->textureIndex;
		outputRegion->width = src->width;
		outputRegion->height = src->height;
		outputRegion->mipLevels = src->mipLevels;
		outputRegion->currentPointerUpdate = src->currentPointerUpdate;
		outputRegion->layerCount = src->layerCount;
		outputRegion->transferMask = src->transferMask;
		outputRegion->mipStart = src->mipStart;
		outputRegion->layerStart = src->layerStart;

		int linkRet = regionLinks[link];

		linkCount--;

		src->data = nullptr;
		src->totalSize = 0;
		src->textureIndex = 0;
		src->width = 0;
		src->height = 0;
		src->mipLevels = 0;
		src->currentPointerUpdate = 0;
		src->layerCount = -1;
		src->layerStart = -1;
		src->mipStart = -1;
		src->transferMask = 0;

		regionLinks[link] = -1;

		return linkRet;
	}
};

struct ShaderResourceUpdatePool
{
	ShaderResourceUpdate* transferRegions;
	int* regionLinks;
	int linkHead = -1;
	int linkCount = 0;
	int ddsRegionAlloc = 0;
	int ddsRegionSize = 0;

	int GetSize(int numberOfRegions)
	{
		return (sizeof(ShaderResourceUpdate) + sizeof(int)) * numberOfRegions;
	}

	void AllocateList(void* poolData, int poolSize)
	{
		ddsRegionSize = (int)((float)poolSize / (float)(sizeof(ShaderResourceUpdate) + sizeof(int)));

		transferRegions = (ShaderResourceUpdate*)poolData;

		regionLinks = (int*)(transferRegions + ddsRegionSize);
	}

	int Create(ShaderResourceManagerIndex descriptorManagerIndex, DescriptorSetInstanceIndex descriptorid, int bindingindex, ShaderResourceType type, void* data, int cachedDataSize, int copies)
	{
		ShaderResourceUpdate* region = nullptr;

		int regionAlloc = ddsRegionAlloc;

		ddsRegionAlloc = (ddsRegionAlloc + 1) % (int)ddsRegionSize;

		region = &transferRegions[regionAlloc];

		region->descriptorManagerIndex = descriptorManagerIndex;
		region->data = data;
		region->dataSize = cachedDataSize;
		region->descriptorSet = descriptorid;
		region->bindingIndex = bindingindex;
		region->type = type;
		region->copyCount = copies;

		regionLinks[regionAlloc] = -1;

		Insert(regionAlloc);

		return 0;
	}

	void Insert(int index)
	{
		DescriptorSetInstanceIndex newid = transferRegions[index].descriptorSet;
		int newbindingindex = transferRegions[index].bindingIndex;
		int* test = &linkHead;
		while ((*test >= 0) && (transferRegions[(*test)].descriptorSet <= newid))
		{
			if (transferRegions[(*test)].bindingIndex < newbindingindex)
				break;
			test = &(regionLinks[(*test)]);
		}
		regionLinks[index] = *test;
		*test = index;
		linkCount++;
	}

	int Find(DescriptorSetInstanceIndex descriptor, int bindingindex)
	{
		int link = linkHead;
		while ((link >= 0) && ((transferRegions[link].descriptorSet != descriptor) || (transferRegions[link].bindingIndex != bindingindex)))
		{
			link = regionLinks[link];
		}
		return (link);
	}

	int PopLink(ShaderResourceUpdate* outputRegion, int link, int** popprev)
	{
		if (link < 0 || link >= ddsRegionSize) return -1;

		ShaderResourceUpdate* region = &transferRegions[link];

		outputRegion->type = region->type;
		outputRegion->descriptorSet = region->descriptorSet;
		outputRegion->bindingIndex = region->bindingIndex;
		outputRegion->copyCount = region->copyCount;
		outputRegion->dataSize = region->dataSize;
		outputRegion->data = region->data;
		outputRegion->descriptorManagerIndex = region->descriptorManagerIndex;

		int linkRet = regionLinks[link];

		if (region->copyCount > 1)
		{
			region->copyCount--;
			*popprev = &regionLinks[link];
		}
		else
		{
			*(*popprev) = linkRet;
			linkCount--;
			region->bindingIndex = -1;
			region->copyCount = -1;
			region->data = nullptr;
			region->descriptorSet = -1;
			region->type = ShaderResourceType::INVALID_SHADER_RESOURCE;
			region->dataSize = -1;
			region->descriptorManagerIndex = -1;
			regionLinks[link] = -1;
		}

		return linkRet;
	}
};

enum ResourceStatusType
{
	BUFFER_RESOURCE = 1,
	IMAGE_RESOURCE = 2,
	MANAGED_IMAGE_RESOURCE = 3,
};

struct ResourceStatus
{
	ResourceStatusType resourceType;
	PipelineStage* currStage;
	BarrierAction* currAction;
	ImageLayout* currentLayout;
};

#define MAX_QUEUE_ENTRIES 63

struct ComputeQueue
{
	PipelineHandleIndex pipelines[63];
	uint32_t queueCount;
};

struct RenderQueue
{
	PipelineHandleIndex pipelines[63];
	uint32_t queueCount;
};

struct RenderTimelineSync
{
	EntryHandle driverTimelineObject;
	uint64_t currentValue;
};

#define MAX_INSTANCE_FRAME_IN_FLIGHT 4

#define MAX_SWC_IMAGE_COUNT 4

struct RenderSwapchainData
{
	EntryHandle swapChainIdx;
	uint32_t width;
	uint32_t height;
	EntryHandle rendererWaitSemaphores[MAX_INSTANCE_FRAME_IN_FLIGHT];
	EntryHandle rendererFinishedSemaphores[MAX_SWC_IMAGE_COUNT];
	TextureIndex textureIds[MAX_SWC_IMAGE_COUNT];
	uint32_t imageCount;
	RenderDeviceIndex deviceIndex;
};

struct RenderWindowSpecificData
{
	EntryHandle vkRenderSurface;

	EntryHandle operator()() 
	{
		return vkRenderSurface;
	}
};

struct RenderPhysicalDeviceInformation
{
	int minUniformAlignment;
	int minStorageAlignment;
	int optimalImageCopyOffsetAlignment;
	int optimalImageCopyRowPitch;
	int maxMSAALevels;
	double deviceTimeStampPeriodNS;
};

struct RenderPhysicalDeviceContainer
{
	EntryHandle physicalDeviceIndex;
	RenderPhysicalDeviceInformation information;
	int internalDriverDeviceListIdentifier;
};

#define MAX_QUERY_RESULTS 32

struct RenderLogicalDeviceContainer
{
	EntryHandle logicalDeviceIndex;
	EntryHandle graphicsComputeTransfer;
	EntryHandle presentQueue;
	EntryHandle queryPoolIndex; 
	RenderTimelineSync deviceTimelineSyncObject;
	EntryHandle currentCommandBufferIndex[MAX_INSTANCE_FRAME_IN_FLIGHT];
	EntryHandle stagingBuffers[MAX_INSTANCE_FRAME_IN_FLIGHT];
	RenderPhysicalDeviceInformation* relatedPhysDeviceInfo;
	DeviceSlabAllocator stagingBufferAllocators[MAX_INSTANCE_FRAME_IN_FLIGHT];
	uint32_t queryResults[MAX_QUERY_RESULTS];
	uint32_t queryCounts[MAX_INSTANCE_FRAME_IN_FLIGHT];
	int maxQueryResults = 0;
	int queriesAreActive = 0;
};

struct RenderBufferDescription
{
	EntryHandle bufferHandle;
	MemoryType type;
	RenderDeviceIndex deviceIndex;
};

#define MAX_VIEWS_ATTACHED_TO_TEXTURE 5
#define ATTACHMENT_VIEW_INDEX 0

struct RenderTextureDescription
{
	EntryHandle textureIndex;
	TextureViewIndex viewIndex[MAX_VIEWS_ATTACHED_TO_TEXTURE];
	ResourceIndex resourceStatusIndex;
	ImageFormat format; 
	ImageType imageType;
	uint32_t imageHeight;
	uint32_t imageWidth;
	uint32_t mipLayers;
	uint32_t arrayLayers;
	uint32_t viewCount;
	RenderDeviceIndex deviceIndex;
};

#define IMAGE_VIEW_ALL_MIPS ~0U
#define IMAGE_VIEW_ALL_LAYERS ~0U

struct RenderImageViewDescription
{
	ImageViewAspectMask mask;
	uint32_t firstMipLevel;
	uint32_t mipLevelCount;
	uint32_t firstLayer;
	uint32_t layerCount;
	ImageLayout desiredLayoutForView;
	EntryHandle viewIndex;
};

struct GPUCommandStreamAllocator
{
	int commandCount;
	int maxCommandCount;
	GPUCommand* commands;
};

struct DriverSpecificBarrierAllocator
{
	PipelineStage srcStage;
	PipelineStage dstStage;
	int barrierCount;
	SlabAllocator* allocator;
	SlabAllocator* driverAllocator;
};

struct IntraPassBarrier
{
	PipelineHandleIndex pipelineInst;
	BarrierType barrierType;
	PipelineStage srcStage;
	PipelineStage destStage;
	uint32_t barrierCount;
	uint32_t pad1;
	void* driverSpecificBarriers;
};

#define MAX_INTRA_PASS_BARRIERS 64

#define BUFFER_BARRIER_ACCUMULATOR 0
#define IMAGE_BARRIER_ACCUMULATOR 1
#define TOTAL_BARRIER_ACCUMULATORS 2
#define QUEUE_FAMILY_IGNORED ~0UL

struct AgnosticImageMemoryBarrier
{
	BarrierAction srcAccess;
	BarrierAction dstAccess;
	uint32_t srcQueueFamily;
	uint32_t dstQueueFamily;
	ImageLayout oldLayout;
	ImageLayout newLayout;
	uint32_t startMip;
	uint32_t mipCount;
	uint32_t startLevel;
	uint32_t levelCount;
	ImageViewAspectMask aspectMask;
	int pad;
	EntryHandle textureIndex;
};

struct AgnosticBufferMemoryBarrier
{
	BarrierAction srcAccess;
	BarrierAction dstAccess;
	uint32_t srcQueueFamily;
	uint32_t dstQueueFamily;
	size_t offset;
	size_t size;
	EntryHandle bufferHandle;
};

struct BarrierAccumulator
{
	DriverSpecificBarrierAllocator accumulators[TOTAL_BARRIER_ACCUMULATORS];
	SlabAllocator intraPassBarrierAllocator;
	IntraPassBarrier intraPassBarriers[MAX_INTRA_PASS_BARRIERS];
	int intraPassCount;
	int intraPassTop;
};

struct ImagePoolDescription
{
	RenderDeviceIndex deviceIndex;
	MemoryType imagePoolType;
	EntryHandle imagePoolHandle;
	size_t imagePoolSize;
};

#define MAX_RESOURCE_IMAGES 4

struct AttachmentResourceInstance
{
	TextureIndex textureIds[MAX_SAMPLE_COUNT_LEVEL][MAX_RESOURCE_IMAGES];
	ImageUsageFlags usage;
	int sampLo;
	int sampHi;
	int imageCount;
};

struct AttachmentRenderPassInstance
{
	AttachmentInstance attachInst[MAX_RENDER_PASS_DESCRIPTIONS];
	int attachInstCount;
	int maxSampleCount;
	DriverRenderTargetIndex baseRenderTarget[MAX_SAMPLE_COUNT_LEVEL];
	OldStyleRenderPassIndex baseRenderPass[MAX_SAMPLE_COUNT_LEVEL];
	int currentSampleCount;
	PipelineQueueIndex graphicsOTQIndex;
	RenderPassType rpType;
};

struct AttachmentGraphInstance
{
	AttachmentGraph* graphLayout;
	AttachmentResourceInstance resources[MAX_GRAPH_RESOURCES];
	AttachmentRenderPassInstance passes[MAX_GRAPH_RENDER_PASSES];
	RenderDeviceIndex deviceIndex;
};

struct RenderShaderResourceTemplateInfo
{
	EntryHandle resourceTemplateInstanceHandle;
	RenderDeviceIndex deviceIndex;
};

struct RenderOldStyleVulkanRenderPassInfo
{
	EntryHandle renderPassHandle;
	RenderDeviceIndex deviceIndex;
};

struct RenderTargetInfo
{
	EntryHandle driverRenderTargetInfo;
	RenderDeviceIndex deviceIndex;
};