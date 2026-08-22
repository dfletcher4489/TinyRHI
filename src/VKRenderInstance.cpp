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

void ResetCommandPool(CommandRecorder* recorder)
{
	recorder->rbo->ResetCommandPoolForBuffer();
}

void BeginCommandRecording(CommandRecorder* recorder)
{
	recorder->rbo->BeginRecordingCommand(nullptr, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
}

void EndCommandRecording(CommandRecorder* recorder)
{
	recorder->rbo->EndRecordingCommand();
}

void ResetDeviceQueries(CommandRecorder* recorder, EntryHandle queryPoolIndex, uint32_t firstQuery, uint32_t queryCount)
{
	recorder->rbo->ResetQueries(queryPoolIndex, firstQuery, queryCount);
}

void BindComputePipelineCmd(CommandRecorder* recorder, EntryHandle pipelineHandle)
{
	recorder->rbo->BindComputePipeline(pipelineHandle);
}

void BindComputeDescriptorSetsCmd(CommandRecorder* recorder, EntryHandle handle, uint32_t descriptorSetIndex, uint32_t setCount, uint32_t firstSet, uint32_t dynamicOffsetCount, uint32_t* dynamicOffsets)
{
	recorder->rbo->BindComputeDescriptorSets(handle, descriptorSetIndex, setCount, firstSet, dynamicOffsetCount, dynamicOffsets);
}

void PushConstantsCmd(CommandRecorder* recorder, uint32_t offset, uint32_t size, ShaderStageType stage, void* data)
{
	recorder->rbo->PushConstantsCommand(offset, size, API::ConvertShaderStageToVulkanShaderStage(stage), data);
}

void DispatchIndirectCmd(CommandRecorder* recorder, EntryHandle indirectBufferIndex, size_t offset)
{
	recorder->rbo->IndirectDispatchCommand(indirectBufferIndex, offset);
}

void DispatchCmd(CommandRecorder* recorder, uint32_t x, uint32_t y, uint32_t z)
{
	recorder->rbo->DispatchCommand(x, y, z);
}

void BeginRenderPassCmd(CommandRecorder* recorder, EntryHandle renderTargetInfo, int subTargetSelection, VkSubpassContents contents, VkRect2D renderArea, VkClearValue* clears, uint32_t clearCount)
{
	recorder->rbo->BeginRenderPassCommand(renderTargetInfo, subTargetSelection, contents, renderArea, clears, clearCount);
}

void EndRenderPassCmd(CommandRecorder* recorder)
{
	recorder->rbo->EndRenderPassCommand();
}

void SetViewportCmd(CommandRecorder* recorder, float x, float y, float width, float height, float minDepth, float maxDepth)
{
	recorder->rbo->SetViewportCommand(x, y, width, height, minDepth, maxDepth);
}

void SetScissorCmd(CommandRecorder* recorder, int32_t x, int32_t y, uint32_t width, uint32_t height)
{
	recorder->rbo->SetScissorCommand(x, y, width, height);
}

void BindGraphicsPipelineCmd(CommandRecorder* recorder, EntryHandle pipelineHandle)
{
	recorder->rbo->BindGraphicsPipeline(pipelineHandle);
}

void BindGraphicsDescriptorSetsCmd(CommandRecorder* recorder, EntryHandle descriptorHandle, uint32_t descriptorSetIndex, uint32_t setCount, uint32_t firstSet, uint32_t dynamicOffsetCount, uint32_t* dynamicOffsets)
{
	recorder->rbo->BindGraphicsDescriptorSets(descriptorHandle, descriptorSetIndex, setCount, firstSet, dynamicOffsetCount, dynamicOffsets);
}

void BindVertexBufferCmd(CommandRecorder* recorder, EntryHandle bufferHandle, uint32_t firstBinding, uint32_t bindingCount, size_t* offsets)
{
	recorder->rbo->BindVertexBuffer(bufferHandle, firstBinding, bindingCount, offsets);
}

void BindIndexBufferCmd(CommandRecorder* recorder, EntryHandle bufferHandle, size_t offset, int indexType)
{
	recorder->rbo->BindIndexBuffer(bufferHandle, offset, indexType == 2 ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32);
}

void DrawIndexedIndirectCountCmd(CommandRecorder* recorder, EntryHandle indirectBufferIndex, EntryHandle indirectCountBufferIndex, size_t indirectOffset, size_t countOffset, uint32_t maxDrawCount)
{
	recorder->rbo->BindingDrawIndexedIndirectCount(indirectBufferIndex, indirectCountBufferIndex, indirectOffset, countOffset, maxDrawCount);
}

void DrawIndexedIndirectCmd(CommandRecorder* recorder, EntryHandle indirectBufferIndex, uint32_t drawCount, size_t indirectOffset)
{
	recorder->rbo->BindingIndexedIndirectDrawCmd(indirectBufferIndex, drawCount, indirectOffset);
}

void DrawIndirectCountCmd(CommandRecorder* recorder, EntryHandle indirectBufferIndex, EntryHandle indirectCountBufferIndex, size_t indirectOffset, size_t countOffset, uint32_t maxDrawCount)
{
	recorder->rbo->BindingDrawIndirectCount(indirectBufferIndex, indirectCountBufferIndex, indirectOffset, countOffset, maxDrawCount);
}

void DrawIndirectCmd(CommandRecorder* recorder, EntryHandle indirectBufferIndex, uint32_t drawCount, size_t indirectOffset)
{
	recorder->rbo->BindingIndirectDrawCmd(indirectBufferIndex, drawCount, indirectOffset);
}

void DrawIndexedCmd(CommandRecorder* recorder, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance)
{
	recorder->rbo->BindingDrawIndexedCmd(indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

void DrawCmd(CommandRecorder* recorder, uint32_t firstVertex, uint32_t vertexCount, uint32_t firstInstance, uint32_t instanceCount)
{
	recorder->rbo->BindingDrawCmd(firstVertex, vertexCount, firstInstance, instanceCount);
}

void WriteTimeStamp(CommandRecorder* recorder, EntryHandle queryPoolIndex, uint32_t queryOffset, PipelineStage stage)
{
	recorder->rbo->WriteTimestamp(queryPoolIndex, queryOffset, (VkPipelineStageFlagBits)API::ConvertBarrierStageToVulkanPipelineStage(stage));
}