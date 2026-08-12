#pragma once

#include "allocator/AppAllocator.h"
#include "CommonRenderTypes.h"

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

struct PipelineInstanceData
{
	int frameGraphIndices[4];
	int frameGraphRenderPasses[4];
	int frameGraphPipelineIndices[4];
	int frameGraphCount;
	int pipelineCount;
};

struct GraphPipelineDescription
{
	PipelineInstanceData instanceData;
	EntryHandle pipelineIndices[MAX_SAMPLE_COUNT_LEVEL];
};

struct ShaderGraph
{
	int shaderMapCount;
	int resourceSetCount;
	int resourceCount;

	ShaderMap shaderMaps[MAX_SHADER_MAPS];
	ShaderResourceTemplate shaderResources[MAX_SHADER_RESOURCES];
	ShaderResourceSetTemplate shaderResourceSetTemplates[MAX_SHADER_RESOURCE_SET_TEMPLATES];

	GraphPipelineDescription pipelineDescription;
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

