#pragma once

#include "allocator/AppAllocator.h"
#include "CommonRenderTypes.h"
#include "RenderInstanceHandleTypes.h"

#define SHADER_NAME_SIZE 64
#define MAX_SHADER_MAPS 4
#define MAX_SHADER_RESOURCES 32
#define MAX_SHADER_RESOURCE_SET_TEMPLATES 16

struct ShaderDetails
{
	char glslShaderName[SHADER_NAME_SIZE];
	int glslShaderNameSize;
	char hlslShaderName[SHADER_NAME_SIZE];
	int hlslShaderNameSize;
	ShaderComputeLayout computeLayout;
	int pad;
	EntryHandle shaderHandle;
};

struct ShaderMap
{
	ShaderStageType type;
	int shaderReference;
};

#define MAX_FRAME_GRAPHS_RENDER_PASS_COMBOS 4

struct PipelineInstanceData
{
	AttachmentGraphInstanceIndex frameGraphIndices[MAX_FRAME_GRAPHS_RENDER_PASS_COMBOS];
	int frameGraphRenderPasses[MAX_FRAME_GRAPHS_RENDER_PASS_COMBOS];
	int frameGraphPipelineIndices[MAX_FRAME_GRAPHS_RENDER_PASS_COMBOS];
	int frameGraphCount;
	int pipelineCount;
	RenderDeviceIndex deviceIndex;
	int pad;
};

struct GraphPipelineDescription
{
	EntryHandle pipelineIndices[MAX_SAMPLE_COUNT_LEVEL * MAX_FRAME_GRAPHS_RENDER_PASS_COMBOS];
	PipelineInstanceData instanceData;
};

struct ShaderGraph
{
	int shaderMapCount;
	int resourceSetCount;
	int resourceCount;

	ShaderMap shaderMaps[MAX_SHADER_MAPS];
	ShaderResourceTemplate shaderResources[MAX_SHADER_RESOURCES];
	ShaderResourceSetTemplate shaderResourceSetTemplates[MAX_SHADER_RESOURCE_SET_TEMPLATES];
};

struct ShaderGraphsHolder
{
	PoolAllocator<ShaderGraph> shaderGraphPtrs{};
	PoolAllocator<ShaderDetails> shaderDetails{};

	ShaderGraphsHolder() = default;

	void Create(Allocator* shaderGraphAllocator, uint32_t maxShaderGraphs, uint32_t maxShaderHandles, StringView shadersGraphsAllocatorName, StringView shadersAllocatorName, Logger* loggerForBoth)
	{
		shaderGraphPtrs.Create(shaderGraphAllocator, maxShaderGraphs, shadersGraphsAllocatorName, loggerForBoth);

		shaderDetails.Create(shaderGraphAllocator, maxShaderHandles, shadersAllocatorName, loggerForBoth);
	}
};

