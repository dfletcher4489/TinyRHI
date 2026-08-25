#pragma once
#include "IndexTypes.h"

#define UNBOUNDED_DESCRIPTOR_ARRAY (1u << 31)
#define DESCRIPTOR_COUNT_MASK (0x7FFFFFFF)

enum CompareOp
{
	NEVER = 0,
	LESS = 1,
	EQUAL = 2,
	LESSEQUAL = 3,
	GREATER = 4,
	NOTEQUAL = 5,
	GREATEREQUAL = 6,
	ALLPASS = 7
};

enum ImageFormat
{
	X8L8U8V8 = 0,
	DXT1 = 1,
	DXT3 = 2,
	R8G8B8A8 = 3,
	B8G8R8A8 = 4,
	D24UNORMS8STENCIL = 5,
	D32FLOATS8STENCIL = 6,
	D32FLOAT = 7,
	R8G8B8A8_UNORM = 8,
	R8G8B8 = 9,
	B8G8R8A8_UNORM = 10,
	R32_UINT = 11,
	IMAGE_UNKNOWN = 0x7fffffff
};

enum PrimitiveType
{
	TRIANGLES = 0,
	TRISTRIPS = 6,
	TRIFAN = 7,
	POINTSLIST = 8,
	LINELIST = 9,
	LINESTRIPS = 10
};

enum class ImageLayout
{
	UNDEFINED = 0,
	WRITEABLE = 1,
	SHADERREADABLE = 2,
	COLORATTACHMENT = 3,
	DEPTHSTENCILATTACHMENT = 4,
	PRESENT = 5,
	DEPTHATTACHMENT = 6,
	STENCILATTACHMENT = 7,
	GENERAL = 8,
	TRANSFER_DEST_OPTIMAL = 9,
	TRANSFER_SRC_OPTIMAL = 10
};

enum class ComponentFormatType
{
	NO_BUFFER_FORMAT = 0,
	RAW_8BIT_BUFFER = 1,
	R32_UINT = 2,
	R32_SINT = 3,
	R32G32B32A32_SFLOAT = 4,
	R32G32B32_SFLOAT = 5,
	R32G32_SFLOAT = 6,
	R32_SFLOAT = 7,
	R32G32_SINT = 8,
	R8G8_UINT = 9,
	R16G16_SINT = 10,
	R16G16B16_SINT = 11
};

enum BarrierActionBits
{
	WRITE_SHADER_RESOURCE = 1,
	READ_SHADER_RESOURCE = 2,
	READ_UNIFORM_BUFFER = 4,
	READ_VERTEX_INPUT = 8,
	READ_INDIRECT_COMMAND = 16,
	TRANSFER_WRITE_DATA_RESOURCE = 32,
	READ_INDEX_INPUT = 64,
};

enum StageBits
{
	VERTEX_SHADER_BARRIER = 1,
	VERTEX_INPUT_BARRIER = 2,
	COMPUTE_BARRIER = 4,
	FRAGMENT_BARRIER = 8,
	BEGINNING_OF_PIPE = 16,
	INDIRECT_DRAW_BARRIER = 32,
	TRANSFER_BARRIER = 64,
	INDEX_INPUT_BARRIER = 128,
	END_OF_PIPE = 256
};

typedef int BarrierAction;

typedef int PipelineStage;

enum class ShaderResourceType
{
	SAMPLER2D = 1,
	STORAGE_BUFFER = 2,
	UNIFORM_BUFFER = 4,
	CONSTANT_BUFFER = 8,
	IMAGESTORE2D = 16,
	IMAGESTORE3D = 32,
	IMAGE2D = 33,
	IMAGE3D = 34,
	BUFFER_VIEW = 128,
	SAMPLER3D = 129,
	SAMPLERCUBE = 130,
	SAMPLERSTATE = 131,
	INVALID_SHADER_RESOURCE = 0x7FFFFFFF
};

enum class ShaderResourceAction
{
	SHADERREAD = 1,
	SHADERWRITE = 2,
	SHADERREADWRITE = 3
};

enum ImageViewAspectMaskBits
{
	COLOR_IMAGE_ASPECT = 1,
	DEPTH_IMAGE_ASPECT = 2,
	STENCIL_IMAGE_ASPECT = 4,
};

typedef int ImageViewAspectMask;

enum class ImageType
{
	IMAGE_1D = 1,
	IMAGE_2D = 2,
	IMAGE_3D = 3,
	IMAGE_CUBE = 4,
};

typedef int ImageUsageFlags;

enum ImageUsageFlagBits
{
	TRANSFER_SRC = 1,
	TRANSFER_DEST = 2,
	SAMPLED = 4,
	STORAGE = 8,
	DEPTH_ATTACHMENT = 16,
	STENCIL_ATTACHMENT = 32,
	COLOR_ATTACHMENT = 64,
	TRANSIENT_ATTACHMENT = 128,
	RESOLVE_ATTACHMENT = 256,
};

enum ShaderStageTypeBits
{
	VERTEXSHADERSTAGE = 1,
	FRAGMENTSHADERSTAGE = 2,
	COMPUTESHADERSTAGE = 4,
};

typedef int ShaderStageType;


enum class VertexUsage : size_t
{
	POSITION = 0,
	TEX0 = 1,
	TEX1 = 2,
	TEX2 = 3,
	TEX3 = 4,
	NORMAL = 5,
	BONES = 6,
	WEIGHTS = 7,
	COLOR0 = 8,
	TANGENTS = 9,
	NUM_VERTEX_FORMAT
};

enum class VertexBufferRate
{
	PERVERTEX = 0,
	PERINSTANCE = 1,
};

struct VertexInputDescription
{
	ComponentFormatType format;
	int byteoffset;
	VertexUsage vertexusage;
};

struct VertexBufferDescription
{
	VertexBufferRate rate;
	int descCount;
	int perInputSize;
	VertexInputDescription descriptions[10];
};

enum TriangleWinding
{
	CW = 0,
	CCW = 1
};

enum class CullMode
{
	CULL_NONE = 0,
	CULL_BACK = 1,
	CULL_FRONT = 2,
};

enum class StencilOp
{
	REPLACE = 0,
	KEEP = 1,
	ZERO = 2,
};

struct FaceStencilData
{
	StencilOp failOp;
	StencilOp passOp;
	StencilOp depthFailOp;
	CompareOp stencilCompare;
	int writeMask;
	int compareMask;
	int reference;
};

enum class BlendFactor
{
	FACTOR_ZERO = 0,
	FACTOR_ONE = 1,
	FACTOR_SRC_COLOR = 2,
	FACTOR_DST_COLOR = 3,
	FACTOR_SRC_ALPHA = 4,
	FACTOR_DST_ALPHA = 5,
	FACTOR_ONE_MINUS_SRC_ALPHA = 6,
	FACTOR_ONE_MINUS_DST_ALPHA = 7,
};

enum class BlendOp
{
	BLEND_ADD = 1,
	BLEND_SUB = 2,
	BLEND_REVERSE_SUB = 3,
	BLEND_MIN = 4,
	BLEND_MAX = 5
};

enum class BlendLogicOp
{
	LOGIC_CLEAR = 0,
	LOGIC_AND = 1,
	LOGIC_COPY = 2
};

struct BlendAttachments
{
	int blendingEnabled;
	BlendFactor srcColorFactor;
	BlendFactor dstColorFactor;
	BlendOp colorOp;
	BlendFactor srcAlphaFactor;
	BlendFactor dstAlphaFactor;
	BlendOp alphaOp;
};

struct GenericPipelineStateInfo
{
	bool depthEnable;
	bool depthWrite;
	bool StencilEnable;
	bool blendEnable;
	PrimitiveType primType;
	float lineWidth;
	TriangleWinding windingOrder;
	CompareOp depthTest;
	FaceStencilData frontFace;
	FaceStencilData backFace;
	int sampleCountLow;
	int sampleCountHigh;
	ImageFormat colorFormat;
	ImageFormat depthFormat;
	BlendLogicOp blendOp;
	CullMode cullMode;
	int vertexBufferDescCount;
	VertexBufferDescription vertexBufferDesc[4];
	int blendAttachmentCount;
	BlendAttachments blendAttachments[4];
};

enum class AttachmentViewType
{
	SWAPCHAIN = 1,
	STATIC = 2,
};

enum class AttachmentLoadUsage
{
	ATTACHNOCARE = 1,
	ATTACHCLEAR = 2,
};

enum class AttachmentStoreUsage
{
	ATTACHDISCARD = 1,
	ATTACHSTORE = 2
};

struct AttachmentResource
{
	AttachmentViewType viewType;
	ImageFormat format;
	int msaa;
};

struct AttachmentDescription
{
	ImageUsageFlags attachType;
	AttachmentLoadUsage loadOp;
	AttachmentStoreUsage storeOp;
	ImageLayout srcLayout;
	ImageLayout dstLayout;
	int resourceIndex;
};

enum class RenderPassType
{
	SWAPCHAIN_IMAGE_COUNT = 1,
	PER_FRAME_IMAGE_COUNT = 2
};

#define MAX_RENDER_PASS_DESCRIPTIONS 8
#define MAX_GRAPH_RENDER_PASSES 4
#define MAX_GRAPH_RESOURCES 12
#define MAX_SAMPLE_COUNT_LEVEL 4
#define MAX_SAMPLE_COUNT (1 << MAX_SAMPLE_COUNT_LEVEL)

struct AttachmentRenderPass
{
	int attachmentCount;
	int resolveCount;
	int depthStencilCount;
	int colorCount;
	AttachmentDescription descs[MAX_RENDER_PASS_DESCRIPTIONS];
};

struct AttachmentGraph
{
	int passesCount;
	int resourceCount;
	AttachmentRenderPass holders[MAX_GRAPH_RENDER_PASSES];
	AttachmentResource resources[MAX_GRAPH_RESOURCES];
};

enum RPClearType
{
	NOCLEAR = 0,
	CLEARCOLOR = 1,
	CLEARDEPTH = 2
};

union ClearVal {
	struct {
		float cdata[4];
	};
	struct {
		float ddata;
		uint32_t sdata;
	};
};

struct AttachmentClear
{
	RPClearType type;
	ClearVal val;
};

struct AttachmentInstance
{
	AttachmentDescription* descLayout;
	int attachmentResource;
	AttachmentClear clear;
};

enum VertexComponents
{
	POSITION = 1,
	TEXTURE0 = 2,
	TEXTURE1 = 4,
	TEXTURE2 = 8,
	NORMAL = 16,
	BONES2 = 32,
	COLOR = 64,
	TANGENT = 128,
	COMPRESSED = 0x80000000,
};

constexpr float dx = 3.051851e-05f;
constexpr float ax = 0.0009770395f;
constexpr float bx = 0.0019550342f;

enum MemoryTypeBits
{
	HOST_MEMORY_TYPE = 1,
	DEVICE_MEMORY_TYPE = 2,
	HOST_MEMORY_COHERENT_TYPE = 4,
};

typedef int MemoryType;

enum BufferAlignmentType
{
	NO_BUFFER_ALIGNMENT = 0,
	UNIFORM_BUFFER_ALIGNMENT = 1,
	STORAGE_BUFFER_ALIGNMENT = 2,
};

enum GPUDeviceType
{
	INTEGRATED = 1,
	DISCRETE = 2,
	VIRTUAL = 4,
	CPU = 8
};

struct GPUFeatureRequest
{
	uint32_t desiredMaxImageWidth;
	uint32_t desiredMaxImageHeight;
	uint32_t deviceType;

	// Vulkan 1.2 features
	bool requireDescriptorBindingPartiallyBound;
	bool requireDescriptorBindingSampledImageUpdateAfterBind;
	bool requireDescriptorBindingUpdateUnusedWhilePending;
	bool requireDescriptorBindingVariableDescriptorCount;
	bool requireShaderSampledImageArrayNonUniformIndexing;
	bool requireStorageBuffer8BitAccess;
	bool requireDrawIndirectCount;
	bool requireRuntimeDescriptorArray;
	bool requireTimelineSemaphores;

	// base features
	bool requireGeometryShader;
	bool requireTextureCompressionBC;
	bool requireTessellationShader;
	bool requireSamplerAnisotropy;
	bool requireMultiDrawIndirect;
	bool requireWideLines;
	bool requireLogicOp;
};

enum class WindowManagementType
{
	WINDOWS32 = 1,
	XLIB = 2,
	WAYLAND = 3,
};

struct RenderingInstanceFeatures
{
	WindowManagementType windowManagementType;
	bool useValidation;
	bool useValidationExtension; 
	bool useSurface;
	bool useSwapChainMaintenance;
	bool useDebugExt;
};

struct LogicalDeviceFeatures
{
	bool useSwapChain;
	bool useSwapChainMaintenance;
	bool useSPVDrawParameters;
	bool useSPVDebugInfo;
};

enum class DescriptorTypes
{
	UNIFORM_DESCRIPTOR = 1,
	UNORDERED_ACCESS_DESCRIPTOR = 2,
	SAMPLED_IMAGE_DESCRIPTOR = 3,
	STORAGE_IMAGE_DESCRIPTOR = 4,
	SAMPLER_DESCRIPTOR = 5,
	COMBINED_IMAGE_SAMPLER_DESCRIPTOR = 6,
};

enum class BarrierType
{
	NULL_BARRIER = 0,
	BUFFER_BARRIER = 1,
	IMAGE_BARRIER = 2,
};

enum class SamplerFilterMode
{
	FILTER_NEAREST = 1,
	FILTER_LINEAR = 2,
};

enum class SamplerAddressMode
{
	ADDRESS_REPEAT = 1,
	ADDRESS_MIRRORED_REPEAT = 2,
	ADDRESS_CLAMP_TO_EDGE = 3,
	ADDRESS_CLAMP_TO_BORDER = 4,
	ADDRESS_MIRROR_CLAMP_TO_EDGE = 5
};

enum class SamplerMipmapMode
{
	MIPMAP_MODE_NEAREST = 1,
	MIPMAP_MODE_LINEAR = 2,
};

typedef int BufferUsage;

enum BufferUsageBits
{
	BUFFER_USAGE_TRANSFER_SRC_BIT = 0x00000001,
	BUFFER_USAGE_TRANSFER_DST_BIT = 0x00000002,
	BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT = 0x00000004,
	BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT = 0x00000008,
	BUFFER_USAGE_UNIFORM_BUFFER_BIT = 0x00000010,
	BUFFER_USAGE_STORAGE_BUFFER_BIT = 0x00000020,
	BUFFER_USAGE_INDEX_BUFFER_BIT = 0x00000040,
	BUFFER_USAGE_VERTEX_BUFFER_BIT = 0x00000080,
	BUFFER_USAGE_INDIRECT_BUFFER_BIT = 0x00000100
};