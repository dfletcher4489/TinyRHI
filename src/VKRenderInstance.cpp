#include "RenderInstance.h"

int DestroyDriverPhysicalDevice(VKInstance* instance, EntryHandle handle)
{
	instance->DestroyPhysicalDevice(handle);
	return 0;
}

int DestroyDriverLogicalDevice(VKInstance* instance, EntryHandle handle)
{
	instance->DestroyLogicalDevice(handle);
	return 0;
}

int DestroyDriverWindowsSurface(VKInstance* device, EntryHandle handle)
{
	device->DestroyRenderSurface(handle);
	return 0;
}

int DestroyDriverSwapChain(RHIDevice* device, EntryHandle handle)
{
	device->device->DestroySwapChain(handle);
	return 0;
}

int DestroyDriverBufferHandle(RHIDevice* device, EntryHandle handle)
{
	device->device->DestroyBuffer(handle);
	return 0;
}

int DestroyDriverImagePool(RHIDevice* device, EntryHandle handle)
{
	device->device->DestroyImagePool(handle);
	return 0;
}

int DestroyDriverPipelineHandle(RHIDevice* device, EntryHandle handle)
{
	device->device->DestroyPipelineCacheObject(handle);
	return 0;
}

int DestroyDriverImage(RHIDevice* device, EntryHandle handle)
{
	device->device->DestroyImage(handle);
	return 0;
}

int DestroyDriverImageView(RHIDevice* device, EntryHandle handle)
{
	device->device->DestroyImageView(handle);
	return 0;
}

int DestroyDriverSamplerResourceHandle(RHIDevice* device, EntryHandle handle)
{
	device->device->DestroySampler(handle);
	return 0;
}

int DestroyOldStyleRenderPass(RHIDevice* device, EntryHandle renderPass)
{
	device->device->DestroyRenderPass(renderPass);
	return 0;
}

int DestroyDriverMainRenderTarget(RHIDevice* device, EntryHandle handle)
{
	device->device->DestroyRenderTarget(handle);
	return 0;
}

int DestroyDriverShaderResourceLayout(RHIDevice* device, EntryHandle handle)
{
	device->device->DestroyDescriptorLayout(handle);
	return 0;
}

int DestroyDriverDescriptorHeap(RHIDevice* device, EntryHandle handle)
{
	device->device->DestroyDescriptorPool(handle);
	return 0;
}

int DestroyDriverShader(RHIDevice* device, EntryHandle handle)
{
	device->device->DestroyShader(handle);
	return 0;
}

int DestoryDriverBufferView(RHIDevice* device, EntryHandle handle)
{
	device->device->DestroyBufferView(handle);
	return 0;
}

int DestroyDriverSemaphore(RHIDevice* device, EntryHandle handle)
{
	device->device->DestroySemaphore(handle);
	return 0;
}