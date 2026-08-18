#pragma once

#include "allocator/AppAllocator.h"
#include "CommonRenderTypes.h"
#include "logger/Logger.h"
#include "RenderInstanceHandleTypes.h"

#include <string.h>

#define MAX_SHADER_RESOURCE_SET_CONSTANT_BUFFER_COUNT 8
#define MAX_SHADER_RESOURCE_SET_RESOURCE_COUNT 32
#define SHADER_NAME_SIZE 64
#define MAX_SHADER_MAPS 4
#define MAX_SHADER_RESOURCES 32
#define MAX_SHADER_RESOURCE_SET_TEMPLATES 16
#define MAX_FRAME_GRAPHS_RENDER_PASS_COMBOS 4

struct ShaderResourceSetTemplate
{
	int vulkanDescLayout;
	int dx12DescriptorTable;
	int resourceStart;
	int totalResourceCount;
	int constantsCount;
	int bindingCount;
	int totalSamplersCount;
	int totalViewsCount;
	int constantStageCount;
};

struct ShaderResourceTemplate
{
	ShaderStageType stages;
	ShaderResourceAction action;
	ShaderResourceType type;
	int set;
	int binding;
	int arrayCount;
	int size;
	int offset;
	int rangeIndex;
};

struct ShaderResourceHeader
{
	ShaderResourceType type;
	ShaderResourceAction action;
	ShaderStageType stage;
	int binding;
	int arrayCount;
};

struct ShaderResourceSampler
{
	int samplerCount;
	SamplerIndex* samplerHandles;
};

struct ShaderResourceImageContainer
{
	TextureIndex textureHandle;
	int viewIndex;
};

struct ShaderResourceCombinedImageContainer
{
	TextureIndex textureHandle;
	int viewIndex;
	SamplerIndex samplerHandle;
};

struct ShaderResourceImage
{
	int textureCount;
	ShaderResourceImageContainer* textureDetails;
};

struct ShaderResourceCombinedImage
{
	int textureCount;
	ShaderResourceCombinedImageContainer* textureDetails;
};

struct ShaderResourceBuffer
{
	int bufferCount;
	int* allocationIndex;
};

struct ShaderResourceArray : public ShaderResourceHeader
{
	union
	{
		ShaderResourceSampler samplers;

		ShaderResourceImage images;

		ShaderResourceBuffer buffers;

		ShaderResourceBuffer views;

		ShaderResourceCombinedImage combinedImages;
	} resourceArray;
};

struct ShaderResourceConstantBuffer : public ShaderResourceHeader
{
	ShaderStageType stage;
	int size;
	int offset;
	int rangeindex;
	void* data;
};

struct ShaderResourceSet
{
	int setCount;
	ShaderResourceConstantBuffer constantBuffers[MAX_SHADER_RESOURCE_SET_CONSTANT_BUFFER_COUNT];
	ShaderResourceArray resourceBindings[MAX_SHADER_RESOURCE_SET_RESOURCE_COUNT];
	ShaderResourceSetTemplate* templateMetaData;
};

struct ShaderComputeLayout
{
	unsigned long x;
	unsigned long y;
	unsigned long z;
};

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

PipelineStage ConvertShaderStageToBarrierStage(ShaderStageType type);

struct ShaderResourceSetContext
{
	Logger* contextLogger;
	bool contextFailed;
};

struct ShaderResourceManager
{
	PoolAllocator<EntryHandle> descriptorSetHandles{};
	ShaderResourceSet** descriptorSets{};
	EntryHandle deviceResourceHeap = EntryHandle();
	RenderDeviceIndex deviceIndex;

	ShaderResourceManager() = default;

	void Create(Allocator* shaderResourceMemoryAllocator, uint32_t maxDescriptorSets, StringView descriptorPoolName, Logger* logger);

	int AddShaderToSets(ShaderResourceSet* location);

	int GetConstantBufferCount(int descriptorSet);

	ShaderResourceHeader* GetConstantBuffer(int descriptorSet, int constantBuffer);
};

struct ShaderResourceSetHandle
{
	ShaderResourceSetHandle() = default;

	ShaderResourceSetHandle(ShaderResourceManagerIndex _descriptorManagerIndex, int _descriptorSetIndex)
		:
		descriptorManagerIndex(_descriptorManagerIndex), descriptorSetIndex(_descriptorSetIndex)
	{

	}

	ShaderResourceManagerIndex descriptorManagerIndex;
	int descriptorSetIndex;
};

struct ShaderResourceSetBuilder
{
	ShaderResourceSet* set;
	ShaderResourceSetHandle handle{};

	ShaderResourceSetBuilder(ShaderResourceManagerIndex _descriptorManagerIndex, int _descriptorSetIndex, ShaderResourceSet* _setPtr);

	ShaderResourceSetHandle operator()();

	void SetVariableArrayCount(ShaderResourceSetContext* context, int bindingIndex, int varArrayCount);

	void BindBufferToShaderResource(ShaderResourceSetContext* context, int* allocationIndex, int firstBuffer, int bufferCount, int bindingIndex);

	void BindImageResourceToShaderResource(ShaderResourceSetContext* context, int* index, int* views, int textureCount, int firstTexture, int bindingIndex);

	void BindSamplerResourceToShaderResource(ShaderResourceSetContext* context, SamplerIndex* indices, int samplerCount, int firstSampler, int bindingIndex);

	void BindSampledImageToShaderResource(ShaderResourceSetContext* context, TextureIndex* index, int* views, SamplerIndex* samplers, int textureCount, int firstTexture, int bindingIndex);

	void BindBufferView(ShaderResourceSetContext* context, int* allocationIndex, int firstView, int viewCount, int bindingIndex);

	ShaderResourceConstantBuffer* GetConstantBuffer(int constantBuffer);
	
	int GetConstantBufferCount();

	void UploadConstant(ShaderResourceSetContext* context, void* data, int bufferLocation);
};

int CreateShaderGraph(StringView filename, RingAllocator* readerMemory, ShaderGraph* graph, ShaderDetails* details, int* shaderDetailCount, Logger* outputLogger);
int CreatePipelineDescription(StringView filename, GenericPipelineStateInfo* stateInfo, Allocator* tempAllocator, Logger* outputLogger);
int CreateAttachmentGraphFromFile(StringView filename, AttachmentGraph* graph, Allocator* inputScratchAllocator, Logger* outputLogger);