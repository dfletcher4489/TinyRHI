#pragma once
#include "RenderInstanceManagement.h"

#include "VKTypes.h"

struct RHIInstance
{
	VKInstance* mainInstance;
};

struct RHIDevice
{
	RenderLogicalDeviceContainer container;
	VKDevice* device;
};

struct CommandRecorder
{
	RHIDevice* device;
	BarrierAccumulator* accumulator;
	RecordingBufferObject* rbo;
	uint64_t errorCodes[64];
	uint32_t errorCount;
};