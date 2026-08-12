#include "RenderInstance.h"

int DestroyDriverImageView(RHIDevice* device, EntryHandle viewIndex)
{
	device->device->DestroyImageView(viewIndex);
	return 0;
}

int DestroyDriverImage(RHIDevice* device, EntryHandle imageIndex)
{
	device->device->DestroyImage(imageIndex);
	return 0;
}

int DestroyOldStyleRenderPass(RHIDevice* device, EntryHandle renderPass)
{
	device->device->DestroyRenderPass(renderPass);
	return 0;
}