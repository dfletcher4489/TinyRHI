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