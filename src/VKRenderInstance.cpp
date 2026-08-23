#include "RenderInstance.h"

int DestroyDriverPhysicalDevice(VKInstance* instance, EntryHandle handle)
{
	if (EntryHandle() == handle)
	{
		return -1;
	}
	instance->DestroyPhysicalDevice(handle);
	return 0;
}

int DestroyDriverLogicalDevice(VKInstance* instance, EntryHandle handle)
{
	if (EntryHandle() == handle)
	{
		return -1;
	}
	instance->DestroyLogicalDevice(handle);
	return 0;
}

int DestroyDriverWindowsSurface(VKInstance* device, EntryHandle handle)
{
	if (EntryHandle() == handle)
	{
		return -1;
	}
	device->DestroyRenderSurface(handle);
	return 0;
}

int DestroyDriverSwapChain(RHIDevice* device, EntryHandle handle)
{
	if (EntryHandle() == handle)
	{
		return -1;
	}
	device->device->DestroySwapChain(handle);
	return 0;
}

int DestroyDriverBufferHandle(RHIDevice* device, EntryHandle handle)
{
	if (EntryHandle() == handle)
	{
		return -1;
	}
	device->device->DestroyBuffer(handle);
	return 0;
}

int DestroyDriverImagePool(RHIDevice* device, EntryHandle handle)
{
	if (EntryHandle() == handle)
	{
		return -1;
	}
	device->device->DestroyImagePool(handle);
	return 0;
}

int DestroyDriverPipelineHandle(RHIDevice* device, EntryHandle handle)
{
	if (EntryHandle() == handle)
	{
		return -1;
	}
	device->device->DestroyPipelineCacheObject(handle);
	return 0;
}

int DestroyDriverImage(RHIDevice* device, EntryHandle handle)
{
	if (EntryHandle() == handle)
	{
		return -1;
	}
	device->device->DestroyImage(handle);
	return 0;
}

int DestroyDriverImageView(RHIDevice* device, EntryHandle handle)
{
	if (EntryHandle() == handle)
	{
		return -1;
	}
	device->device->DestroyImageView(handle);
	return 0;
}

int DestroyDriverSamplerResourceHandle(RHIDevice* device, EntryHandle handle)
{
	if (EntryHandle() == handle)
	{
		return -1;
	}
	device->device->DestroySampler(handle);
	return 0;
}

int DestroyOldStyleRenderPass(RHIDevice* device, EntryHandle renderPass)
{
	if (EntryHandle() == renderPass)
	{
		return -1;
	}
	device->device->DestroyRenderPass(renderPass);
	return 0;
}

int DestroyDriverMainRenderTarget(RHIDevice* device, EntryHandle handle)
{
	if (EntryHandle() == handle)
	{
		return -1;
	}
	device->device->DestroyRenderTarget(handle);
	return 0;
}

int DestroyDriverShaderResourceLayout(RHIDevice* device, EntryHandle handle)
{
	if (EntryHandle() == handle)
	{
		return -1;
	}
	device->device->DestroyDescriptorLayout(handle);
	return 0;
}

int DestroyDriverDescriptorHeap(RHIDevice* device, EntryHandle handle)
{
	if (EntryHandle() == handle)
	{
		return -1;
	}
	device->device->DestroyDescriptorPool(handle);
	return 0;
}

int DestroyDriverShader(RHIDevice* device, EntryHandle handle)
{
	if (EntryHandle() == handle)
	{
		return -1;
	}
	device->device->DestroyShader(handle);
	return 0;
}

int DestoryDriverBufferView(RHIDevice* device, EntryHandle handle)
{
	if (EntryHandle() == handle)
	{
		return -1;
	}
	device->device->DestroyBufferView(handle);
	return 0;
}

int DestroyDriverSemaphore(RHIDevice* device, EntryHandle handle)
{
	if (EntryHandle() == handle)
	{
		return -1;
	}
	device->device->DestroySemaphore(handle);
	return 0;
}

int DestroyDescriptorSet(RHIDevice* device, EntryHandle handle)
{
	if (EntryHandle() == handle)
	{
		return -1;
	}
	device->device->DestroyDescriptorSet(handle);
	return 0;
}

void ResetCommandPool(CommandRecorder* recorder)
{
	recorder->rbo.ResetCommandPoolForBuffer();
}

void BeginCommandRecording(CommandRecorder* recorder)
{
	recorder->rbo.BeginRecordingCommand(nullptr, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
}

void EndCommandRecording(CommandRecorder* recorder)
{
	recorder->rbo.EndRecordingCommand();
}

void ResetDeviceQueries(CommandRecorder* recorder, EntryHandle queryPoolIndex, uint32_t firstQuery, uint32_t queryCount)
{
	recorder->rbo.ResetQueries(queryPoolIndex, firstQuery, queryCount);
}

void BindComputePipelineCmd(CommandRecorder* recorder, EntryHandle pipelineHandle)
{
	recorder->rbo.BindComputePipeline(pipelineHandle);
}

void BindComputeDescriptorSetsCmd(CommandRecorder* recorder, EntryHandle handle, uint32_t descriptorSetIndex, uint32_t setCount, uint32_t firstSet, uint32_t dynamicOffsetCount, uint32_t* dynamicOffsets)
{
	recorder->rbo.BindComputeDescriptorSets(handle, descriptorSetIndex, setCount, firstSet, dynamicOffsetCount, dynamicOffsets);
}

void PushConstantsCmd(CommandRecorder* recorder, uint32_t offset, uint32_t size, ShaderStageType stage, void* data)
{
	recorder->rbo.PushConstantsCommand(offset, size, API::ConvertShaderStageToVulkanShaderStage(stage), data);
}

void DispatchIndirectCmd(CommandRecorder* recorder, EntryHandle indirectBufferIndex, size_t offset)
{
	recorder->rbo.IndirectDispatchCommand(indirectBufferIndex, offset);
}

void DispatchCmd(CommandRecorder* recorder, uint32_t x, uint32_t y, uint32_t z)
{
	recorder->rbo.DispatchCommand(x, y, z);
}

void BeginRenderPassCmd(CommandRecorder* recorder, EntryHandle renderTargetInfo, uint32_t imageIndex, AttachmentClear* agnosticClears, uint32_t clearCount, Allocator* clearsAllocators)
{
	VkClearValue* clears = (VkClearValue*)clearsAllocators->Allocate(sizeof(VkClearValue) * clearCount);

	for (uint32_t g = 0; g < clearCount; g++)
	{
		VkClearValue* currClear = &clears[g];
		switch (agnosticClears[g].type)
		{
		case NOCLEAR:
			break;
		case CLEARCOLOR:
			currClear->color.float32[0] = agnosticClears[g].val.cdata[0];
			currClear->color.float32[1] = agnosticClears[g].val.cdata[1];
			currClear->color.float32[2] = agnosticClears[g].val.cdata[2];
			currClear->color.float32[3] = agnosticClears[g].val.cdata[3];
			break;
		case CLEARDEPTH:
			currClear->depthStencil.depth = agnosticClears[g].val.ddata;
			currClear->depthStencil.stencil = agnosticClears[g].val.sdata;
			break;
		}
	}

	RenderTarget* renderTarget = recorder->device->device->GetRenderTarget(renderTargetInfo);

	recorder->rbo.BeginRenderPassCommandForRenderTarget(renderTargetInfo, imageIndex, VK_SUBPASS_CONTENTS_INLINE, clears, clearCount);
}

void EndRenderPassCmd(CommandRecorder* recorder)
{
	recorder->rbo.EndRenderPassCommand();
}

void SetViewportCmd(CommandRecorder* recorder, float x, float y, float width, float height, float minDepth, float maxDepth)
{
	recorder->rbo.SetViewportCommand(x, y, width, height, minDepth, maxDepth);
}

void SetViewportCmd(CommandRecorder* recorder, EntryHandle renderTargetIndex, float minDepth, float maxDepth)
{
	RenderTarget* renderTarget = recorder->device->device->GetRenderTarget(renderTargetIndex);

	float x = static_cast<float>(renderTarget->width), y = static_cast<float>(renderTarget->height);

	float xOff = static_cast<float>(renderTarget->wOffset), yOff = static_cast<float>(renderTarget->hOffset);

	recorder->rbo.SetViewportCommand(xOff, yOff, x, y, minDepth, maxDepth);
}

void SetScissorCmd(CommandRecorder* recorder, int32_t x, int32_t y, uint32_t width, uint32_t height)
{
	recorder->rbo.SetScissorCommand(x, y, width, height);
}

void SetScissorCmd(CommandRecorder* recorder, EntryHandle renderTargetIndex)
{
	RenderTarget* renderTarget = recorder->device->device->GetRenderTarget(renderTargetIndex);

	recorder->rbo.SetScissorCommand(renderTarget->wOffset, renderTarget->hOffset, renderTarget->width, renderTarget->height);
}

void BindGraphicsPipelineCmd(CommandRecorder* recorder, EntryHandle pipelineHandle)
{
	recorder->rbo.BindGraphicsPipeline(pipelineHandle);
}

void BindGraphicsDescriptorSetsCmd(CommandRecorder* recorder, EntryHandle descriptorHandle, uint32_t descriptorSetIndex, uint32_t setCount, uint32_t firstSet, uint32_t dynamicOffsetCount, uint32_t* dynamicOffsets)
{
	recorder->rbo.BindGraphicsDescriptorSets(descriptorHandle, descriptorSetIndex, setCount, firstSet, dynamicOffsetCount, dynamicOffsets);
}

void BindVertexBufferCmd(CommandRecorder* recorder, EntryHandle bufferHandle, uint32_t firstBinding, uint32_t bindingCount, size_t* offsets)
{
	recorder->rbo.BindVertexBuffer(bufferHandle, firstBinding, bindingCount, offsets);
}

void BindIndexBufferCmd(CommandRecorder* recorder, EntryHandle bufferHandle, size_t offset, int indexType)
{
	recorder->rbo.BindIndexBuffer(bufferHandle, offset, indexType == 2 ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32);
}

void DrawIndexedIndirectCountCmd(CommandRecorder* recorder, EntryHandle indirectBufferIndex, EntryHandle indirectCountBufferIndex, size_t indirectOffset, size_t countOffset, uint32_t maxDrawCount)
{
	recorder->rbo.BindingDrawIndexedIndirectCount(indirectBufferIndex, indirectCountBufferIndex, indirectOffset, countOffset, maxDrawCount);
}

void DrawIndexedIndirectCmd(CommandRecorder* recorder, EntryHandle indirectBufferIndex, uint32_t drawCount, size_t indirectOffset)
{
	recorder->rbo.BindingIndexedIndirectDrawCmd(indirectBufferIndex, drawCount, indirectOffset);
}

void DrawIndirectCountCmd(CommandRecorder* recorder, EntryHandle indirectBufferIndex, EntryHandle indirectCountBufferIndex, size_t indirectOffset, size_t countOffset, uint32_t maxDrawCount)
{
	recorder->rbo.BindingDrawIndirectCount(indirectBufferIndex, indirectCountBufferIndex, indirectOffset, countOffset, maxDrawCount);
}

void DrawIndirectCmd(CommandRecorder* recorder, EntryHandle indirectBufferIndex, uint32_t drawCount, size_t indirectOffset)
{
	recorder->rbo.BindingIndirectDrawCmd(indirectBufferIndex, drawCount, indirectOffset);
}

void DrawIndexedCmd(CommandRecorder* recorder, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance)
{
	recorder->rbo.BindingDrawIndexedCmd(indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

void DrawCmd(CommandRecorder* recorder, uint32_t firstVertex, uint32_t vertexCount, uint32_t firstInstance, uint32_t instanceCount)
{
	recorder->rbo.BindingDrawCmd(firstVertex, vertexCount, firstInstance, instanceCount);
}

void WriteTimeStamp(CommandRecorder* recorder, EntryHandle queryPoolIndex, uint32_t queryOffset, PipelineStage stage)
{
	recorder->rbo.WriteTimestamp(queryPoolIndex, queryOffset, (VkPipelineStageFlagBits)API::ConvertBarrierStageToVulkanPipelineStage(stage));
}

int WriteHostBufferBatch(RHIDevice* device, EntryHandle bufferHandle, void** cpuDataLocations, size_t* sizesOfDataLocations, size_t* offsetsIntoHostMemory, size_t numberOfLocations, size_t mappableSize, size_t mappableStart)
{
	device->device->WriteToHostBufferBatch(bufferHandle, cpuDataLocations, sizesOfDataLocations, offsetsIntoHostMemory, mappableSize, mappableStart, numberOfLocations);
	
	return 0;
}

int WriteDeviceBufferBatch(CommandRecorder* recorder, EntryHandle bufferHandle, EntryHandle stagingBufferHandle,
	void** cpuDataLocations, size_t* sizesOfDataLocations, size_t* offsetsIntoStagingMemory, size_t* offsetsIntoDeviceMemory, size_t numberOfCopies, size_t mappableSize)
{
	recorder->device->device->WriteToDeviceBufferBatch(bufferHandle, stagingBufferHandle, 
		cpuDataLocations, sizesOfDataLocations, offsetsIntoDeviceMemory, 
		mappableSize, offsetsIntoStagingMemory, numberOfCopies, &recorder->rbo);

	return 0;
}

void FillBuffer(CommandRecorder* recorder, EntryHandle bufferHandle, size_t regionSize, size_t regionOffset, uint32_t fillVal)
{
	recorder->rbo.FillBuffer(bufferHandle, regionSize, regionOffset, fillVal);
}

void MakeAndBindDriverAccumulatedBarriers(CommandRecorder* recorder)
{
	RBOPipelineBarrierArgs args{};

	BarrierAccumulator* accumulator = recorder->accumulator;

	VKDevice* device = recorder->device->device;

	if (accumulator->accumulators[IMAGE_BARRIER_ACCUMULATOR].barrierCount)
	{
		args.srcStageMask |= API::ConvertBarrierStageToVulkanPipelineStage(accumulator->accumulators[IMAGE_BARRIER_ACCUMULATOR].srcStage);
		args.dstStageMask |= API::ConvertBarrierStageToVulkanPipelineStage(accumulator->accumulators[IMAGE_BARRIER_ACCUMULATOR].dstStage);

		args.imageMemoryBarrierCount = accumulator->accumulators[IMAGE_BARRIER_ACCUMULATOR].barrierCount;

		AgnosticImageMemoryBarrier* imageBarriers = (AgnosticImageMemoryBarrier*)accumulator->accumulators[IMAGE_BARRIER_ACCUMULATOR].allocator->dataHead;

		for (uint32_t i = 0; i < args.imageMemoryBarrierCount; i++)
		{
			VkImageMemoryBarrier* barrier = (VkImageMemoryBarrier*)accumulator->accumulators[IMAGE_BARRIER_ACCUMULATOR].driverAllocator->CAllocate(sizeof(VkImageMemoryBarrier));

			AgnosticImageMemoryBarrier* currImageBarriers = &imageBarriers[i];

			barrier->sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			
			barrier->subresourceRange.aspectMask = API::ConvertImageViewAspectMaskToVulkanImageAspectFlags(currImageBarriers->aspectMask);
			barrier->subresourceRange.baseArrayLayer = currImageBarriers->startLevel;
			barrier->subresourceRange.baseMipLevel = currImageBarriers->startMip;
			barrier->subresourceRange.layerCount = currImageBarriers->levelCount;
			barrier->subresourceRange.levelCount = currImageBarriers->mipCount;

			barrier->dstAccessMask = API::ConvertBarrierActionToVulkanAccessFlags(currImageBarriers->dstAccess);
			barrier->srcAccessMask = API::ConvertBarrierActionToVulkanAccessFlags(currImageBarriers->srcAccess);
			barrier->newLayout = API::ConvertImageLayoutToVulkanImageLayout(currImageBarriers->newLayout);
			barrier->oldLayout = API::ConvertImageLayoutToVulkanImageLayout(currImageBarriers->oldLayout);
			barrier->image = device->GetImageByHandle(currImageBarriers->textureIndex);

			barrier->srcQueueFamilyIndex = currImageBarriers->srcQueueFamily;
			barrier->dstQueueFamilyIndex = currImageBarriers->dstQueueFamily;
		}

		args.pImageMemoryBarriers = (VkImageMemoryBarrier*)accumulator->accumulators[IMAGE_BARRIER_ACCUMULATOR].driverAllocator->dataHead;
	}

	if (accumulator->accumulators[BUFFER_BARRIER_ACCUMULATOR].barrierCount)
	{
		args.srcStageMask |= API::ConvertBarrierStageToVulkanPipelineStage(accumulator->accumulators[BUFFER_BARRIER_ACCUMULATOR].srcStage);
		args.dstStageMask |= API::ConvertBarrierStageToVulkanPipelineStage(accumulator->accumulators[BUFFER_BARRIER_ACCUMULATOR].dstStage);

		args.bufferMemoryBarrierCount = accumulator->accumulators[BUFFER_BARRIER_ACCUMULATOR].barrierCount;

		AgnosticBufferMemoryBarrier* bufferBarriers = (AgnosticBufferMemoryBarrier*)accumulator->accumulators[BUFFER_BARRIER_ACCUMULATOR].allocator->dataHead;

		for (uint32_t i = 0; i < args.bufferMemoryBarrierCount; i++)
		{
			VkBufferMemoryBarrier* barrier = (VkBufferMemoryBarrier*)accumulator->accumulators[BUFFER_BARRIER_ACCUMULATOR].driverAllocator->CAllocate(sizeof(VkBufferMemoryBarrier));

			AgnosticBufferMemoryBarrier* currImageBarriers = &bufferBarriers[i];

			barrier->sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;

			barrier->dstAccessMask = API::ConvertBarrierActionToVulkanAccessFlags(currImageBarriers->dstAccess);
			barrier->srcAccessMask = API::ConvertBarrierActionToVulkanAccessFlags(currImageBarriers->srcAccess);

			barrier->buffer = device->GetBufferHandle(currImageBarriers->bufferHandle);
			barrier->size = currImageBarriers->size;
			barrier->offset = currImageBarriers->offset;

			barrier->srcQueueFamilyIndex = currImageBarriers->srcQueueFamily;
			barrier->dstQueueFamilyIndex = currImageBarriers->dstQueueFamily;
		}

		args.pBufferMemoryBarriers = (VkBufferMemoryBarrier*)accumulator->accumulators[BUFFER_BARRIER_ACCUMULATOR].driverAllocator->dataHead;
	}

	if (args.imageMemoryBarrierCount || args.bufferMemoryBarrierCount)
	{
		recorder->rbo.BindPipelineBarrierCommand(&args);

		accumulator->accumulators[BUFFER_BARRIER_ACCUMULATOR].barrierCount = accumulator->accumulators[BUFFER_BARRIER_ACCUMULATOR].dstStage = accumulator->accumulators[BUFFER_BARRIER_ACCUMULATOR].srcStage = 0;

		accumulator->accumulators[BUFFER_BARRIER_ACCUMULATOR].allocator->Reset();
		accumulator->accumulators[BUFFER_BARRIER_ACCUMULATOR].driverAllocator->Reset();

		accumulator->accumulators[IMAGE_BARRIER_ACCUMULATOR].barrierCount = accumulator->accumulators[IMAGE_BARRIER_ACCUMULATOR].srcStage = accumulator->accumulators[IMAGE_BARRIER_ACCUMULATOR].dstStage = 0;

		accumulator->accumulators[IMAGE_BARRIER_ACCUMULATOR].allocator->Reset();
		accumulator->accumulators[IMAGE_BARRIER_ACCUMULATOR].driverAllocator->Reset();
	}
}

void MakeAndBindDriverIntraPassBarriers(CommandRecorder* recorder, PipelineHandleIndex& pipelineIndex)
{
	VKDevice* device = recorder->device->device;

	if (recorder->accumulator->intraPassTop == recorder->accumulator->intraPassCount)
		return;

	IntraPassBarrier* ipb = &recorder->accumulator->intraPassBarriers[recorder->accumulator->intraPassTop];

	VkImageMemoryBarrier imageMemoryBarriers[32]{};
	VkBufferMemoryBarrier bufferMemoryBarriers[32]{};

	uint32_t imageCount = 0;
	uint32_t bufferCount = 0;

	RBOPipelineBarrierArgs args{};

	args.pBufferMemoryBarriers = bufferMemoryBarriers;
	args.pImageMemoryBarriers = imageMemoryBarriers;

	while (recorder->accumulator->intraPassTop < recorder->accumulator->intraPassCount && ipb->pipelineInst == pipelineIndex)
	{
		args.srcStageMask |= API::ConvertBarrierStageToVulkanPipelineStage(ipb->srcStage);
		args.dstStageMask |= API::ConvertBarrierStageToVulkanPipelineStage(ipb->destStage);

		if (ipb->barrierType == BarrierType::IMAGE_BARRIER)
		{
			AgnosticImageMemoryBarrier* barriers = (AgnosticImageMemoryBarrier*)ipb->driverSpecificBarriers;

			for (uint32_t i = 0; i < ipb->barrierCount; i++)
			{
				VkImageMemoryBarrier* barrier = (VkImageMemoryBarrier*)&args.pImageMemoryBarriers[imageCount++];

				AgnosticImageMemoryBarrier* currImageBarriers = &barriers[i];

				barrier->sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;

				barrier->subresourceRange.aspectMask = API::ConvertImageViewAspectMaskToVulkanImageAspectFlags(currImageBarriers->aspectMask);
				barrier->subresourceRange.baseArrayLayer = currImageBarriers->startLevel;
				barrier->subresourceRange.baseMipLevel = currImageBarriers->startMip;
				barrier->subresourceRange.layerCount = currImageBarriers->levelCount;
				barrier->subresourceRange.levelCount = currImageBarriers->mipCount;

				barrier->dstAccessMask = API::ConvertBarrierActionToVulkanAccessFlags(currImageBarriers->dstAccess);
				barrier->srcAccessMask = API::ConvertBarrierActionToVulkanAccessFlags(currImageBarriers->srcAccess);
				barrier->newLayout = API::ConvertImageLayoutToVulkanImageLayout(currImageBarriers->newLayout);
				barrier->oldLayout = API::ConvertImageLayoutToVulkanImageLayout(currImageBarriers->oldLayout);
				barrier->image = device->GetImageByHandle(currImageBarriers->textureIndex);

				barrier->srcQueueFamilyIndex = currImageBarriers->srcQueueFamily;
				barrier->dstQueueFamilyIndex = currImageBarriers->dstQueueFamily;
			}
		}
		else if (ipb->barrierType == BarrierType::BUFFER_BARRIER)
		{
			AgnosticBufferMemoryBarrier* barriers = (AgnosticBufferMemoryBarrier*)ipb->driverSpecificBarriers;

			for (uint32_t i = 0; i < ipb->barrierCount; i++)
			{
				VkBufferMemoryBarrier* barrier = &args.pBufferMemoryBarriers[bufferCount++];

				AgnosticBufferMemoryBarrier* currImageBarriers = &barriers[i];

				barrier->sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;

				barrier->dstAccessMask = API::ConvertBarrierActionToVulkanAccessFlags(currImageBarriers->dstAccess);
				barrier->srcAccessMask = API::ConvertBarrierActionToVulkanAccessFlags(currImageBarriers->srcAccess);

				barrier->buffer = device->GetBufferHandle(currImageBarriers->bufferHandle);
				barrier->size = currImageBarriers->size;
				barrier->offset = currImageBarriers->offset;

				barrier->srcQueueFamilyIndex = currImageBarriers->srcQueueFamily;
				barrier->dstQueueFamilyIndex = currImageBarriers->dstQueueFamily;
			}
		}

		recorder->accumulator->intraPassTop++;

		ipb = &recorder->accumulator->intraPassBarriers[recorder->accumulator->intraPassTop];
	}

	if (imageCount || bufferCount)
	{
		args.imageMemoryBarrierCount = imageCount;
		args.bufferMemoryBarrierCount = bufferCount;
		recorder->rbo.BindPipelineBarrierCommand(&args);
	}
}

int UploadImageDataToDeviceMemory(
	CommandRecorder* recorder, 
	EntryHandle textureHandle, EntryHandle stagingBufferHandle,
	void* cpuImageData, size_t totalUploadSize, size_t imageDataOffsetInStaging,
	uint32_t writeWidth, uint32_t writeHeight,
	uint32_t mipLevels, uint32_t layersCount,
	ImageFormat imageFormat, ImageViewAspectMask aspectMask
)
{
	recorder->device->device->UploadImageData(
		textureHandle,
		(char*)cpuImageData,
		totalUploadSize,
		stagingBufferHandle,
		writeWidth,
		writeHeight,
		mipLevels,
		layersCount,
		API::ConvertImageFormatToVulkanFormat(imageFormat),
		API::ConvertImageViewAspectMaskToVulkanImageAspectFlags(aspectMask),
		imageDataOffsetInStaging,
		&recorder->rbo
	);

	return 0;
}

size_t GetDriverImageMemoryBarrierSize()
{
	return sizeof(VkImageMemoryBarrier);
}

size_t GetDriverBufferMemoryBarrierSize()
{
	return sizeof(VkBufferMemoryBarrier);
}

size_t GetDriverImageMemoryBarrierAlign()
{
	return alignof(VkImageMemoryBarrier);
}

size_t GetDriverBufferMemoryBarrierAlign()
{
	return alignof(VkBufferMemoryBarrier);
}

size_t GetDriverIndexedIndirectDrawCommandSize()
{
	return sizeof(VkDrawIndexedIndirectCommand);
}

size_t GetDriverIndirectDrawCommandSize()
{
	return sizeof(VkDrawIndirectCommand);
}

size_t GetDriverIndirectDispatchCommandSize()
{
	return sizeof(VkDispatchIndirectCommand);
}

size_t GetDriverIndexedIndirectDrawCommandAlign()
{
	return alignof(VkDrawIndexedIndirectCommand);
}

size_t GetDriverIndirectDrawCommandAlign()
{
	return alignof(VkDrawIndirectCommand);
}

size_t GetDriverIndirectDispatchCommandAlign()
{
	return alignof(VkDispatchIndirectCommand);
}

void GetDriverCommandBufferObject(CommandRecorder* recorder, EntryHandle commandBufferIndex)
{
	recorder->rbo = recorder->device->device->GetRecordingBufferObject(commandBufferIndex);
}