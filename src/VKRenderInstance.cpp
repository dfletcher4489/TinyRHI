#include "RenderInstance.h"

#include "VKInstance.h"
#include "VKDescriptorLayoutBuilder.h"
#include "VKDescriptorSetBuilder.h"
#include "VKRenderPassBuilder.h"
#include "VKPipelineBuilder.h"
#include "VKSwapChain.h"

#define RENDER_MIN(a, b) ((a) > (b) ? (b) : (a))
#define RENDER_MAX(a, b) ((a) < (b) ? (b) : (a))
#define RENDER_PWR2UP(size, align) (((size) + ((align)-1)) & ~((align)-1))

namespace API
{
	VkAttachmentLoadOp ConvertAttachLoadOpToVulkanLoadOp(AttachmentLoadUsage loadOp)
	{
		VkAttachmentLoadOp ret = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		switch (loadOp)
		{
		case AttachmentLoadUsage::ATTACHNOCARE:
			break;
		case AttachmentLoadUsage::ATTACHCLEAR:
			ret = VK_ATTACHMENT_LOAD_OP_CLEAR;
			break;
		}
		return ret;
	}

	VkAttachmentStoreOp ConvertAttachStoreOpToVulkanStoreOp(AttachmentStoreUsage storeOp)
	{
		VkAttachmentStoreOp ret = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		switch (storeOp)
		{
		case AttachmentStoreUsage::ATTACHDISCARD:
			break;
		case AttachmentStoreUsage::ATTACHSTORE:
			ret = VK_ATTACHMENT_STORE_OP_STORE;
			break;
		}
		return ret;
	}

	VkFormat ConvertComponentFormatTypeToVulkanFormat(ComponentFormatType type)
	{
		VkFormat format = VK_FORMAT_UNDEFINED;
		switch (type)
		{
		case ComponentFormatType::RAW_8BIT_BUFFER:
			format = VK_FORMAT_R8_UINT;
			break;
		case ComponentFormatType::R32_UINT:
			format = VK_FORMAT_R32_UINT;
			break;
		case ComponentFormatType::R32_SINT:
			format = VK_FORMAT_R32_SINT;
			break;
		case ComponentFormatType::R32G32B32A32_SFLOAT:
			format = VK_FORMAT_R32G32B32A32_SFLOAT;
			break;
		case ComponentFormatType::R32G32B32_SFLOAT:
			format = VK_FORMAT_R32G32B32_SFLOAT;
			break;
		case ComponentFormatType::R32G32_SFLOAT:
			format = VK_FORMAT_R32G32_SFLOAT;
			break;
		case ComponentFormatType::R32_SFLOAT:
			format = VK_FORMAT_R32_SFLOAT;
			break;
		case ComponentFormatType::R32G32_SINT:
			format = VK_FORMAT_R32G32_SINT;
			break;
		case ComponentFormatType::R8G8_UINT:
			format = VK_FORMAT_R8G8_UINT;
			break;
		case ComponentFormatType::R16G16_SINT:
			format = VK_FORMAT_R16G16_SINT;
			break;
		case ComponentFormatType::R16G16B16_SINT:
			format = VK_FORMAT_R16G16B16_SINT;
			break;
		default:
			break;
		}

		return format;
	}

	VkCompareOp ConvertCompareOpToVulkanCompareOp(CompareOp testApp)
	{
		VkCompareOp ret = VK_COMPARE_OP_ALWAYS;

		switch (testApp)
		{
		case CompareOp::NEVER:
			ret = VK_COMPARE_OP_NEVER;
			break;

		case CompareOp::LESS:
			ret = VK_COMPARE_OP_LESS;
			break;

		case CompareOp::EQUAL:
			ret = VK_COMPARE_OP_EQUAL;
			break;

		case CompareOp::LESSEQUAL:
			ret = VK_COMPARE_OP_LESS_OR_EQUAL;
			break;

		case CompareOp::GREATER:
			ret = VK_COMPARE_OP_GREATER;
			break;

		case CompareOp::NOTEQUAL:
			ret = VK_COMPARE_OP_NOT_EQUAL;
			break;

		case CompareOp::GREATEREQUAL:
			ret = VK_COMPARE_OP_GREATER_OR_EQUAL;
			break;

		case CompareOp::ALLPASS:
			ret = VK_COMPARE_OP_ALWAYS;
			break;

		default:
			break;
		}

		return ret;
	}

	VkFormat ConvertImageFormatToVulkanFormat(ImageFormat format)
	{
		VkFormat vkFormat = VK_FORMAT_MAX_ENUM;
		switch (format)
		{
		case ImageFormat::DXT1:
			vkFormat = VK_FORMAT_BC1_RGB_SRGB_BLOCK;
			break;
		case ImageFormat::DXT3:
			vkFormat = VK_FORMAT_BC3_SRGB_BLOCK;
			break;
		case ImageFormat::R8G8B8A8:
			vkFormat = VK_FORMAT_R8G8B8A8_SRGB;
			break;
		case ImageFormat::R8G8B8A8_UNORM:
			vkFormat = VK_FORMAT_R8G8B8A8_UNORM;
			break;
		case ImageFormat::B8G8R8A8_UNORM:
			vkFormat = VK_FORMAT_B8G8R8A8_UNORM;
			break;
		case ImageFormat::B8G8R8A8:
			vkFormat = VK_FORMAT_B8G8R8A8_SRGB;
			break;
		case ImageFormat::D24UNORMS8STENCIL:
			vkFormat = VK_FORMAT_D24_UNORM_S8_UINT;
			break;
		case ImageFormat::D32FLOATS8STENCIL:
			vkFormat = VK_FORMAT_D32_SFLOAT_S8_UINT;
			break;

		case ImageFormat::D32FLOAT:
			vkFormat = VK_FORMAT_D32_SFLOAT;
			break;
		case ImageFormat::R32_UINT:
			vkFormat = VK_FORMAT_R32_UINT;
			break;
		}
		return vkFormat;
	}


	ImageFormat ConvertVkFormatToAppFormat(VkFormat vkFormat)
	{
		ImageFormat format = ImageFormat::IMAGE_UNKNOWN;
		switch (vkFormat)
		{
		case VK_FORMAT_BC1_RGB_SRGB_BLOCK:
			format = ImageFormat::DXT1;
			break;
		case VK_FORMAT_BC3_SRGB_BLOCK:
			format = ImageFormat::DXT3;
			break;
		case VK_FORMAT_R8G8B8A8_SRGB:
			format = ImageFormat::R8G8B8A8;
			break;
		case VK_FORMAT_R8G8B8A8_UNORM:
			format = ImageFormat::R8G8B8A8_UNORM;
			break;
		case VK_FORMAT_D24_UNORM_S8_UINT:
			format = ImageFormat::D24UNORMS8STENCIL;
			break;

		case VK_FORMAT_D32_SFLOAT_S8_UINT:
			format = ImageFormat::D32FLOATS8STENCIL;
			break;

		case VK_FORMAT_D32_SFLOAT:
			format = ImageFormat::D32FLOAT;
			break;
		case VK_FORMAT_B8G8R8A8_SRGB:
			format = ImageFormat::B8G8R8A8;
			break;

		}
		return format;
	}

	VkPrimitiveTopology ConvertTopology(PrimitiveType type)
	{
		VkPrimitiveTopology top = VK_PRIMITIVE_TOPOLOGY_MAX_ENUM;

		switch (type)
		{
		case TRIANGLES:
			top = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
			break;

		case TRISTRIPS:
			top = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
			break;

		case TRIFAN:
			top = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
			break;

		case POINTSLIST:
			top = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
			break;

		case LINELIST:
			top = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
			break;

		case LINESTRIPS:
			top = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
			break;

		default:
			break;
		}

		return top;
	}

	VkAccessFlags ConvertBarrierActionToVulkanAccessFlags(BarrierAction action)
	{
		VkAccessFlags flags = 0;
		flags |= (VK_ACCESS_SHADER_WRITE_BIT) * ((action & WRITE_SHADER_RESOURCE) != 0);
		flags |= (VK_ACCESS_SHADER_READ_BIT) * ((action & READ_SHADER_RESOURCE) != 0);
		flags |= (VK_ACCESS_UNIFORM_READ_BIT) * ((action & READ_UNIFORM_BUFFER) != 0);
		flags |= (VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT) * ((action & READ_VERTEX_INPUT) != 0);
		flags |= (VK_ACCESS_INDIRECT_COMMAND_READ_BIT) * ((action & READ_INDIRECT_COMMAND) != 0);
		flags |= (VK_ACCESS_TRANSFER_WRITE_BIT) * ((action & TRANSFER_WRITE_DATA_RESOURCE) != 0);
		flags |= (VK_ACCESS_INDEX_READ_BIT) * ((action & READ_INDEX_INPUT) != 0);
		return flags;
	}

	VkPipelineStageFlags ConvertBarrierStageToVulkanPipelineStage(PipelineStage sourceStage)
	{
		VkPipelineStageFlags flags = 0;
		flags |= (VK_PIPELINE_STAGE_VERTEX_SHADER_BIT) * ((sourceStage & VERTEX_SHADER_BARRIER) != 0);
		flags |= (VK_PIPELINE_STAGE_VERTEX_INPUT_BIT) * ((sourceStage & VERTEX_INPUT_BARRIER) != 0);
		flags |= (VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT) * ((sourceStage & COMPUTE_BARRIER) != 0);
		flags |= (VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT) * ((sourceStage & BEGINNING_OF_PIPE) != 0);
		flags |= (VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT) * ((sourceStage & FRAGMENT_BARRIER) != 0);
		flags |= (VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT) * ((sourceStage & INDIRECT_DRAW_BARRIER) != 0);
		flags |= (VK_PIPELINE_STAGE_TRANSFER_BIT) * ((sourceStage & TRANSFER_BARRIER) != 0);
		flags |= (VK_PIPELINE_STAGE_VERTEX_INPUT_BIT) * ((sourceStage & INDEX_INPUT_BARRIER) != 0);
		flags |= (VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT) * ((sourceStage & END_OF_PIPE) != 0);
		return flags;
	}

	VkShaderStageFlags ConvertShaderStageToVulkanShaderStage(ShaderStageType type)
	{
		VkShaderStageFlags flags = 0;
		flags |= (VK_SHADER_STAGE_VERTEX_BIT) * ((type & VERTEXSHADERSTAGE) != 0);
		flags |= (VK_SHADER_STAGE_FRAGMENT_BIT) * ((type & FRAGMENTSHADERSTAGE) != 0);
		flags |= (VK_SHADER_STAGE_COMPUTE_BIT) * ((type & COMPUTESHADERSTAGE) != 0);
		return flags;
	}

	VkImageLayout ConvertImageLayoutToVulkanImageLayout(ImageLayout layout)
	{
		VkImageLayout outLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		switch (layout)
		{
		case ImageLayout::UNDEFINED:
			break;
		case ImageLayout::WRITEABLE:
			outLayout = VK_IMAGE_LAYOUT_GENERAL;
			break;
		case ImageLayout::SHADERREADABLE:
			outLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			break;
		case ImageLayout::COLORATTACHMENT:
			outLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			break;
		case ImageLayout::DEPTHSTENCILATTACHMENT:
			outLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			break;
		case ImageLayout::PRESENT:
			outLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
			break;
		case ImageLayout::TRANSFER_DEST_OPTIMAL:
			outLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			break;
		case ImageLayout::TRANSFER_SRC_OPTIMAL:
			outLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			break;
		}

		return outLayout;
	}

	void ConvertVertexInputToVKVertexAttrDescription(VertexInputDescription* inputDescs, int numInputDescs, int vertexBufferLocation, VkVertexInputAttributeDescription* attrs)
	{
		for (int i = 0; i < numInputDescs; i++)
		{
			VkVertexInputAttributeDescription& attr = attrs[i];

			attr.location = i;
			attr.format = ConvertComponentFormatTypeToVulkanFormat(inputDescs[i].format);
			attr.offset = inputDescs[i].byteoffset;
			attr.binding = vertexBufferLocation;
		}
	}

	VkFrontFace ConvertTriangleWinding(TriangleWinding winding)
	{
		VkFrontFace face = VK_FRONT_FACE_COUNTER_CLOCKWISE;

		switch (winding)
		{
		case CW:
			face = VK_FRONT_FACE_CLOCKWISE;
			break;

		case CCW:
			face = VK_FRONT_FACE_COUNTER_CLOCKWISE;
			break;

		default:
			break;
		}

		return face;
	}

	VkCullModeFlags ConvertCullMode(CullMode mode)
	{
		VkCullModeFlags ret = VK_CULL_MODE_NONE;

		switch (mode)
		{
		case CullMode::CULL_NONE:
			ret = VK_CULL_MODE_NONE;
			break;

		case CullMode::CULL_BACK:
			ret = VK_CULL_MODE_BACK_BIT;
			break;

		case CullMode::CULL_FRONT:
			ret = VK_CULL_MODE_FRONT_BIT;
			break;

		default:
			break;
		}

		return ret;
	}

	VkStencilOp ConvertStencilOpToVulkan(StencilOp op)
	{
		switch (op)
		{
		case StencilOp::REPLACE:
			return VK_STENCIL_OP_REPLACE;

		case StencilOp::KEEP:
			return VK_STENCIL_OP_KEEP;

		case StencilOp::ZERO:
			return VK_STENCIL_OP_ZERO;

		default:
			return VK_STENCIL_OP_KEEP;
		}
	}

	VkStencilOpState ConvertFaceStencilDataToVulkan(const FaceStencilData& face)
	{
		VkStencilOpState state{};
		state.failOp = ConvertStencilOpToVulkan(face.failOp);
		state.passOp = ConvertStencilOpToVulkan(face.passOp);
		state.depthFailOp = ConvertStencilOpToVulkan(face.depthFailOp);
		state.compareOp = ConvertCompareOpToVulkanCompareOp(face.stencilCompare);

		state.compareMask = static_cast<uint32_t>(face.compareMask);
		state.writeMask = static_cast<uint32_t>(face.writeMask);
		state.reference = static_cast<uint32_t>(face.reference);

		return state;
	}

	void ConvertGPUFeatureRequestToVkPhysicalDeviceProperties(
		const GPUFeatureRequest* request,
		VkPhysicalDeviceFeatures2* features2,
		VkPhysicalDeviceVulkan12Features* features12)
	{

		features12->sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
		features12->pNext = nullptr;

		features12->descriptorBindingPartiallyBound =
			request->requireDescriptorBindingPartiallyBound ? VK_TRUE : VK_FALSE;
		features12->descriptorBindingSampledImageUpdateAfterBind =
			request->requireDescriptorBindingSampledImageUpdateAfterBind ? VK_TRUE : VK_FALSE;
		features12->descriptorBindingUpdateUnusedWhilePending =
			request->requireDescriptorBindingUpdateUnusedWhilePending ? VK_TRUE : VK_FALSE;
		features12->descriptorBindingVariableDescriptorCount =
			request->requireDescriptorBindingVariableDescriptorCount ? VK_TRUE : VK_FALSE;
		features12->shaderSampledImageArrayNonUniformIndexing =
			request->requireShaderSampledImageArrayNonUniformIndexing ? VK_TRUE : VK_FALSE;
		features12->storageBuffer8BitAccess =
			request->requireStorageBuffer8BitAccess ? VK_TRUE : VK_FALSE;
		features12->drawIndirectCount =
			request->requireDrawIndirectCount ? VK_TRUE : VK_FALSE;
		features12->runtimeDescriptorArray =
			request->requireRuntimeDescriptorArray ? VK_TRUE : VK_FALSE;
		features12->timelineSemaphore = request->requireTimelineSemaphores ? VK_TRUE : VK_FALSE;

		features2->sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
		features2->pNext = features12;

		features2->features.geometryShader =
			request->requireGeometryShader ? VK_TRUE : VK_FALSE;
		features2->features.textureCompressionBC =
			request->requireTextureCompressionBC ? VK_TRUE : VK_FALSE;
		features2->features.tessellationShader =
			request->requireTessellationShader ? VK_TRUE : VK_FALSE;
		features2->features.samplerAnisotropy =
			request->requireSamplerAnisotropy ? VK_TRUE : VK_FALSE;
		features2->features.multiDrawIndirect =
			request->requireMultiDrawIndirect ? VK_TRUE : VK_FALSE;
		features2->features.wideLines =
			request->requireWideLines ? VK_TRUE : VK_FALSE;

		features2->features.logicOp = request->requireLogicOp ? VK_TRUE : VK_FALSE;
	}

	VkImageAspectFlags ConvertImageViewAspectMaskToVulkanImageAspectFlags(ImageViewAspectMask aspectMask)
	{
		return
			((aspectMask & COLOR_IMAGE_ASPECT) ? VK_IMAGE_ASPECT_COLOR_BIT : 0) |
			((aspectMask & DEPTH_IMAGE_ASPECT) ? VK_IMAGE_ASPECT_DEPTH_BIT : 0) |
			((aspectMask & STENCIL_IMAGE_ASPECT) ? VK_IMAGE_ASPECT_STENCIL_BIT : 0);
	}

	VkImageType ConvertImageTypeToVulkanImageType(ImageType imageType)
	{
		VkImageType result = VK_IMAGE_TYPE_2D;

		switch (imageType)
		{
		case ImageType::IMAGE_1D:
			result = VK_IMAGE_TYPE_1D;
			break;

		case ImageType::IMAGE_2D:
			result = VK_IMAGE_TYPE_2D;
			break;

		case ImageType::IMAGE_3D:
			result = VK_IMAGE_TYPE_3D;
			break;

		case ImageType::IMAGE_CUBE:
			result = VK_IMAGE_TYPE_2D;
			break;

		default:
			result = VK_IMAGE_TYPE_2D;
			break;
		}

		return result;
	}

	VkImageViewType ConvertImageTypeToVulkanImageViewType(ImageType imageType)
	{
		VkImageViewType result = VK_IMAGE_VIEW_TYPE_2D;

		switch (imageType)
		{
		case ImageType::IMAGE_1D:
			result = VK_IMAGE_VIEW_TYPE_1D;
			break;

		case ImageType::IMAGE_2D:
			result = VK_IMAGE_VIEW_TYPE_2D;
			break;

		case ImageType::IMAGE_3D:
			result = VK_IMAGE_VIEW_TYPE_3D;
			break;

		case ImageType::IMAGE_CUBE:
			result = VK_IMAGE_VIEW_TYPE_CUBE;
			break;

		default:
			result = VK_IMAGE_VIEW_TYPE_2D;
			break;
		}

		return result;
	}

	VkImageUsageFlags ConvertImageUsageFlagsToVulkanImageUsageFlags(ImageUsageFlags flags)
	{
		VkImageUsageFlags vkFlags = 0;

		vkFlags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT * ((flags & TRANSFER_SRC) != 0);
		vkFlags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT * ((flags & TRANSFER_DEST) != 0);
		vkFlags |= VK_IMAGE_USAGE_SAMPLED_BIT * ((flags & SAMPLED) != 0);
		vkFlags |= VK_IMAGE_USAGE_STORAGE_BIT * ((flags & STORAGE) != 0);
		vkFlags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT * ((flags & DEPTH_ATTACHMENT) != 0);
		vkFlags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT * ((flags & STENCIL_ATTACHMENT) != 0);
		vkFlags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT * ((flags & COLOR_ATTACHMENT) != 0);
		vkFlags |= VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT * ((flags & TRANSIENT_ATTACHMENT) != 0);

		return vkFlags;
	}

	VkMemoryPropertyFlags ConvertMemoryTypeToVkMemoryPropertyFlags(MemoryType memType)
	{
		VkMemoryPropertyFlags retFlags = 0;
		retFlags |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT * ((memType & MemoryTypeBits::DEVICE_MEMORY_TYPE) != 0);
		retFlags |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT * (((memType & MemoryTypeBits::HOST_MEMORY_TYPE) != 0) || ((memType & MemoryTypeBits::HOST_MEMORY_COHERENT_TYPE) != 0));
		retFlags |= VK_MEMORY_PROPERTY_HOST_COHERENT_BIT * ((memType & MemoryTypeBits::HOST_MEMORY_COHERENT_TYPE) != 0);
		return retFlags;
	}

	VkBlendFactor ConvertBlendFactorToVulkanBlendFactor(BlendFactor factor)
	{
		VkBlendFactor vkFactor = VK_BLEND_FACTOR_ZERO;

		switch (factor)
		{
		case BlendFactor::FACTOR_ZERO:
			vkFactor = VK_BLEND_FACTOR_ZERO;
			break;

		case BlendFactor::FACTOR_ONE:
			vkFactor = VK_BLEND_FACTOR_ONE;
			break;

		case BlendFactor::FACTOR_SRC_COLOR:
			vkFactor = VK_BLEND_FACTOR_SRC_COLOR;
			break;

		case BlendFactor::FACTOR_DST_COLOR:
			vkFactor = VK_BLEND_FACTOR_DST_COLOR;
			break;

		case BlendFactor::FACTOR_SRC_ALPHA:
			vkFactor = VK_BLEND_FACTOR_SRC_ALPHA;
			break;

		case BlendFactor::FACTOR_DST_ALPHA:
			vkFactor = VK_BLEND_FACTOR_DST_ALPHA;
			break;

		case BlendFactor::FACTOR_ONE_MINUS_SRC_ALPHA:
			vkFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			break;

		case BlendFactor::FACTOR_ONE_MINUS_DST_ALPHA:
			vkFactor = VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
			break;
		}

		return vkFactor;
	}

	VkBlendOp ConvertBlendOpToVulkanBlendOp(BlendOp op)
	{
		VkBlendOp vkOp = VK_BLEND_OP_ADD;

		switch (op)
		{
		case BlendOp::BLEND_ADD:
			vkOp = VK_BLEND_OP_ADD;
			break;

		case BlendOp::BLEND_SUB:
			vkOp = VK_BLEND_OP_SUBTRACT;
			break;

		case BlendOp::BLEND_REVERSE_SUB:
			vkOp = VK_BLEND_OP_REVERSE_SUBTRACT;
			break;

		case BlendOp::BLEND_MIN:
			vkOp = VK_BLEND_OP_MIN;
			break;

		case BlendOp::BLEND_MAX:
			vkOp = VK_BLEND_OP_MAX;
			break;
		}

		return vkOp;
	}

	VkLogicOp ConvertBlendLogicOpToVulkanLogicOp(BlendLogicOp op)
	{
		VkLogicOp vkOp = VK_LOGIC_OP_CLEAR;

		switch (op)
		{
		case BlendLogicOp::LOGIC_CLEAR:
			vkOp = VK_LOGIC_OP_CLEAR;
			break;

		case BlendLogicOp::LOGIC_AND:
			vkOp = VK_LOGIC_OP_AND;
			break;

		case BlendLogicOp::LOGIC_COPY:
			vkOp = VK_LOGIC_OP_COPY;
			break;
		}

		return vkOp;
	}

	VkFilter ConvertSamplerFilterModeToVulkanFilter(SamplerFilterMode filterMode)
	{
		VkFilter filter = VK_FILTER_NEAREST;

		switch (filterMode)
		{
		case SamplerFilterMode::FILTER_NEAREST:
			filter = VK_FILTER_NEAREST;
			break;

		case SamplerFilterMode::FILTER_LINEAR:
			filter = VK_FILTER_LINEAR;
			break;
		}

		return filter;
	}

	VkSamplerAddressMode ConvertSamplerAddressModeToVulkanSamplerAddressMode(SamplerAddressMode addressMode)
	{
		VkSamplerAddressMode mode = VK_SAMPLER_ADDRESS_MODE_REPEAT;

		switch (addressMode)
		{
		case SamplerAddressMode::ADDRESS_REPEAT:
			mode = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			break;

		case SamplerAddressMode::ADDRESS_MIRRORED_REPEAT:
			mode = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
			break;

		case SamplerAddressMode::ADDRESS_CLAMP_TO_EDGE:
			mode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			break;

		case SamplerAddressMode::ADDRESS_CLAMP_TO_BORDER:
			mode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
			break;

		case SamplerAddressMode::ADDRESS_MIRROR_CLAMP_TO_EDGE:
			mode = VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
			break;
		}

		return mode;
	}

	VkSamplerMipmapMode ConvertSamplerMipmapModeToVulkanSamplerMipmapMode(SamplerMipmapMode mipmapMode)
	{
		VkSamplerMipmapMode mode = VK_SAMPLER_MIPMAP_MODE_NEAREST;

		switch (mipmapMode)
		{
		case SamplerMipmapMode::MIPMAP_MODE_NEAREST:
			mode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
			break;

		case SamplerMipmapMode::MIPMAP_MODE_LINEAR:
			mode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
			break;
		}

		return mode;
	}

	VkBufferUsageFlags ConvertBufferUsageToDriverBufferUsage(BufferUsage usage)
	{
		VkBufferUsageFlags result = 0;

		if (usage & BUFFER_USAGE_TRANSFER_SRC_BIT)
			result |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

		if (usage & BUFFER_USAGE_TRANSFER_DST_BIT)
			result |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;

		if (usage & BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT)
			result |= VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT;

		if (usage & BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT)
			result |= VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;

		if (usage & BUFFER_USAGE_UNIFORM_BUFFER_BIT)
			result |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

		if (usage & BUFFER_USAGE_STORAGE_BUFFER_BIT)
			result |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;

		if (usage & BUFFER_USAGE_INDEX_BUFFER_BIT)
			result |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

		if (usage & BUFFER_USAGE_VERTEX_BUFFER_BIT)
			result |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

		if (usage & BUFFER_USAGE_INDIRECT_BUFFER_BIT)
			result |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;

		return result;
	}
}

static VKAPI_ATTR VkBool32 VKAPI_CALL vulkanDebugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData) {

	if (messageSeverity < VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
	{
		return VK_FALSE;
	}

	Logger* logger = (Logger*)pUserData;

	logger->AddLogMessage(LOGERROR, pCallbackData->pMessage, strlen(pCallbackData->pMessage));

	logger->ProcessMessage();

	return VK_FALSE;
}

#ifdef _MSC_VER
#include <intrin.h>
#endif

static int findLSB(unsigned int input)
{
	if (!input) return -1;
#ifdef _MSC_VER
	unsigned long index;
	_BitScanForward(&index, input);
	return index;
#else
	return __builtin_ctz(input);
#endif
}

static int findMSB(unsigned int input)
{
	if (!input) return -1;

#ifdef _MSC_VER
	unsigned long index;
	_BitScanReverse(&index, input);
	return index;
#else
	return 31 - __builtin_clz(input);
#endif
}

RenderPhysicalDeviceIndex RenderInstance::CreatePhysicalDeviceAdapter(GPUFeatureRequest* requestedPhysicalFeatures, LogicalDeviceFeatures* requestedDeviceFeatures)
{
	if (physicalDeviceCounter == maxPhysicalDevices)
	{
		internalRendererLogger->AddLogMessage(LOGERROR, STRING_VIEW_FROM_LITERAL("CreatePhysicalDeviceAdapter : too many gpus allocated"));
		return {};
	}

	uint32_t deviceExtNameCount = vkInstance.mainInstance->GetLogicalDeviceExtensionsCount(requestedDeviceFeatures);

	const char** deviceFeatureNames = (const char**)cacheAllocator->Allocate(sizeof(char*) * deviceExtNameCount);

	vkInstance.mainInstance->GetLogicalDeviceExtensions(requestedDeviceFeatures, deviceFeatureNames);

	int driverGpuIndex = -1;

	EntryHandle physicalIndex = vkInstance.mainInstance->CreatePhysicalDevice(requestedPhysicalFeatures, deviceFeatureNames, deviceExtNameCount, &driverGpuIndex);

	if (EntryHandle() == physicalIndex)
	{
		GetLastInstanceDriverError(STRING_VIEW_FROM_LITERAL("CreatePhysicalDeviceAdapter : failed to create gpu adapter on driver"));
		return {};
	}

	int physicalEntryIndex = physicalDeviceCounter++;

	RenderPhysicalDeviceContainer* container = &physicalDeviceIndices[physicalEntryIndex];

	CleanInitializePhysicalDeviceIndices(container);

	container->physicalDeviceIndex = physicalIndex;
	container->information.minUniformAlignment = vkInstance.mainInstance->GetMinimumUniformBufferAlignment(physicalIndex);
	container->information.minStorageAlignment = vkInstance.mainInstance->GetMinimumStorageBufferAlignment(physicalIndex);
	container->information.maxMSAALevels = findMSB(vkInstance.mainInstance->GetMaxMSAALevels(physicalIndex));
	container->information.deviceTimeStampPeriodNS = vkInstance.mainInstance->GetTimeStampPeriod(physicalIndex);
	container->information.optimalImageCopyOffsetAlignment = vkInstance.mainInstance->GetOptimalImageCopyOffsetAlignment(physicalIndex);
	container->internalDriverDeviceListIdentifier = driverGpuIndex;

	RenderPhysicalDeviceIndex indexRet(physicalEntryIndex);

	return indexRet;
}

int RenderInstance::OpenPhysicalDevicePicker()
{
	if (physicalDeviceCounter == maxPhysicalDevices)
	{
		internalRendererLogger->AddLogMessage(LOGERROR, STRING_VIEW_FROM_LITERAL("OpenPhysicalDevicePicker: Too many GPUs allocated based on CreateInfo"));
		return -1;
	}

	int gpuCount = vkInstance.mainInstance->GetNumberOfGPUDevices();

	if (gpuCount <= 0)
	{
		GetLastInstanceDriverError(STRING_VIEW_FROM_LITERAL("OpenPhysicalDevicePicker: No GPU reported back"));
		return -1;
	}

	physicalDevicesOnComputerPerDriver = gpuCount;

	return 0;
}

void RenderInstance::ClosePhysicalDevicePicker()
{
	vkInstance.mainInstance->FreePotentialGPUs();
}

RenderPhysicalDeviceIndex RenderInstance::CreatePhysicalDeviceAdapterWithQuerying(GPUFeatureRequest* requestedPhysicalFeatures, LogicalDeviceFeatures* requestedDeviceFeatures)
{
	if (physicalDeviceCounter == maxPhysicalDevices)
	{
		internalRendererLogger->AddLogMessage(LOGERROR, STRING_VIEW_FROM_LITERAL("CreatePhysicalDeviceAdapterWithQuerying: Too many GPUs allocated based on CreateInfo"));
		return {};
	}

	uint32_t deviceExtNameCount = vkInstance.mainInstance->GetLogicalDeviceExtensionsCount(requestedDeviceFeatures);

	const char** deviceFeatureNames = (const char**)cacheAllocator->Allocate(sizeof(char*) * deviceExtNameCount);

	vkInstance.mainInstance->GetLogicalDeviceExtensions(requestedDeviceFeatures, deviceFeatureNames);

	int gpuIndex = 0;

	uint64_t expectedDeviceExtMask = (1ULL << deviceExtNameCount) - 1;

	int* physicalDeviceExlusionList = nullptr;

	if (physicalDeviceCounter)
	{
		physicalDeviceExlusionList = (int*)cacheAllocator->Allocate(sizeof(int) * physicalDeviceCounter);

		for (int i = 0; i < physicalDeviceCounter; i++)
		{
			RenderPhysicalDeviceContainer* container = &physicalDeviceIndices[i];

			physicalDeviceExlusionList[i] = container->internalDriverDeviceListIdentifier;
		}
	}

	char topLineMessageBuffer[64];

	for (; gpuIndex < physicalDevicesOnComputerPerDriver; gpuIndex++)
	{
		bool alreadyUsed = false;

		for (int i = 0; i < physicalDeviceCounter; i++)
		{
			if (physicalDeviceExlusionList[i] == gpuIndex)
			{
				alreadyUsed = true;
				break;
			}
		}

		if (alreadyUsed) continue;

		GPUFeatureRequest currentRequest{};
		uint64_t currentDeviceExtMask = 0;

		if (vkInstance.mainInstance->QuerySpecificPhysicalDeviceFeatures(requestedPhysicalFeatures, &currentRequest, deviceFeatureNames, deviceExtNameCount, gpuIndex, &currentDeviceExtMask))
			break;

		int size = snprintf(topLineMessageBuffer, sizeof(topLineMessageBuffer), "GPU at index %d has mismatched values\n", gpuIndex);

		internalRendererLogger->AddLogMessage(LOGINFO, topLineMessageBuffer, size);

		if (!(currentRequest.deviceType & requestedPhysicalFeatures->deviceType))
			internalRendererLogger->AddLogMessage(LOGINFO, STRING_VIEW_FROM_LITERAL("Desired GPU type is not what was requested\n"));

		if (currentRequest.desiredMaxImageWidth < requestedPhysicalFeatures->desiredMaxImageWidth)
			internalRendererLogger->AddLogMessage(LOGINFO, STRING_VIEW_FROM_LITERAL("Desired Max Image Width is less than requested\n"));

		if (currentRequest.desiredMaxImageHeight < requestedPhysicalFeatures->desiredMaxImageHeight)
			internalRendererLogger->AddLogMessage(LOGINFO, STRING_VIEW_FROM_LITERAL("Desired Max Image Height is less than requested\n"));

		if (requestedPhysicalFeatures->requireDescriptorBindingPartiallyBound &&
			!currentRequest.requireDescriptorBindingPartiallyBound)
			internalRendererLogger->AddLogMessage(LOGINFO, STRING_VIEW_FROM_LITERAL("Descriptor Binding Partially Bound is not supported\n"));

		if (requestedPhysicalFeatures->requireDescriptorBindingSampledImageUpdateAfterBind &&
			!currentRequest.requireDescriptorBindingSampledImageUpdateAfterBind)
			internalRendererLogger->AddLogMessage(LOGINFO, STRING_VIEW_FROM_LITERAL("Descriptor Binding Sampled Image Update After Bind is not supported\n"));

		if (requestedPhysicalFeatures->requireDescriptorBindingUpdateUnusedWhilePending &&
			!currentRequest.requireDescriptorBindingUpdateUnusedWhilePending)
			internalRendererLogger->AddLogMessage(LOGINFO, STRING_VIEW_FROM_LITERAL("Descriptor Binding Update Unused While Pending is not supported\n"));

		if (requestedPhysicalFeatures->requireDescriptorBindingVariableDescriptorCount &&
			!currentRequest.requireDescriptorBindingVariableDescriptorCount)
			internalRendererLogger->AddLogMessage(LOGINFO, STRING_VIEW_FROM_LITERAL("Descriptor Binding Variable Descriptor Count is not supported\n"));

		if (requestedPhysicalFeatures->requireShaderSampledImageArrayNonUniformIndexing &&
			!currentRequest.requireShaderSampledImageArrayNonUniformIndexing)
			internalRendererLogger->AddLogMessage(LOGINFO, STRING_VIEW_FROM_LITERAL("Shader Sampled Image Array Non Uniform Indexing is not supported\n"));

		if (requestedPhysicalFeatures->requireStorageBuffer8BitAccess &&
			!currentRequest.requireStorageBuffer8BitAccess)
			internalRendererLogger->AddLogMessage(LOGINFO, STRING_VIEW_FROM_LITERAL("Storage Buffer 8 Bit Access is not supported\n"));

		if (requestedPhysicalFeatures->requireDrawIndirectCount &&
			!currentRequest.requireDrawIndirectCount)
			internalRendererLogger->AddLogMessage(LOGINFO, STRING_VIEW_FROM_LITERAL("Draw Indirect Count is not supported\n"));

		if (requestedPhysicalFeatures->requireRuntimeDescriptorArray &&
			!currentRequest.requireRuntimeDescriptorArray)
			internalRendererLogger->AddLogMessage(LOGINFO, STRING_VIEW_FROM_LITERAL("Runtime Descriptor Array is not supported\n"));

		if (requestedPhysicalFeatures->requireGeometryShader &&
			!currentRequest.requireGeometryShader)
			internalRendererLogger->AddLogMessage(LOGINFO, STRING_VIEW_FROM_LITERAL("Geometry Shader is not supported\n"));

		if (requestedPhysicalFeatures->requireTextureCompressionBC &&
			!currentRequest.requireTextureCompressionBC)
			internalRendererLogger->AddLogMessage(LOGINFO, STRING_VIEW_FROM_LITERAL("Texture Compression BC is not supported\n"));

		if (requestedPhysicalFeatures->requireTessellationShader &&
			!currentRequest.requireTessellationShader)
			internalRendererLogger->AddLogMessage(LOGINFO, STRING_VIEW_FROM_LITERAL("Tessellation Shader is not supported\n"));

		if (requestedPhysicalFeatures->requireSamplerAnisotropy &&
			!currentRequest.requireSamplerAnisotropy)
			internalRendererLogger->AddLogMessage(LOGINFO, STRING_VIEW_FROM_LITERAL("Sampler Anisotropy is not supported\n"));

		if (requestedPhysicalFeatures->requireMultiDrawIndirect &&
			!currentRequest.requireMultiDrawIndirect)
			internalRendererLogger->AddLogMessage(LOGINFO, STRING_VIEW_FROM_LITERAL("Multi Draw Indirect is not supported\n"));

		if (requestedPhysicalFeatures->requireWideLines &&
			!currentRequest.requireWideLines)
			internalRendererLogger->AddLogMessage(LOGINFO, STRING_VIEW_FROM_LITERAL("Wide Lines is not supported\n"));

		if (currentDeviceExtMask != expectedDeviceExtMask)
		{
			for (uint32_t i = 0; i < deviceExtNameCount; i++)
			{
				if (!(currentDeviceExtMask & (1ull << i)))
				{
					internalRendererLogger->AddLogMessage(LOGINFO, STRING_VIEW_FROM_LITERAL("Device extenstion not supported : "));
					internalRendererLogger->AddLogMessage(LOGINFO, deviceFeatureNames[i], strlen(deviceFeatureNames[i]));
					internalRendererLogger->AddLogMessage(LOGINFO, "\n", 1);
				}
			}
		}
	}

	if (gpuIndex == physicalDevicesOnComputerPerDriver)
	{
		internalRendererLogger->ProcessMessage();
		return {};
	}

	EntryHandle physicalIndex = vkInstance.mainInstance->CreateGPUFromIndex(gpuIndex);

	if (EntryHandle() == physicalIndex)
	{
		GetLastInstanceDriverError(STRING_VIEW_FROM_LITERAL("CreatePhysicalDeviceAdapterWithQuerying: GPU creation by driver failed"));
		return {};
	}

	int physicalEntryIndex = physicalDeviceCounter++;

	RenderPhysicalDeviceContainer* container = &physicalDeviceIndices[physicalEntryIndex];

	CleanInitializePhysicalDeviceIndices(container);

	container->physicalDeviceIndex = physicalIndex;
	container->information.minUniformAlignment = vkInstance.mainInstance->GetMinimumUniformBufferAlignment(physicalIndex);
	container->information.minStorageAlignment = vkInstance.mainInstance->GetMinimumStorageBufferAlignment(physicalIndex);
	container->information.maxMSAALevels = findMSB(vkInstance.mainInstance->GetMaxMSAALevels(physicalIndex));
	container->information.deviceTimeStampPeriodNS = vkInstance.mainInstance->GetTimeStampPeriod(physicalIndex);
	container->information.optimalImageCopyOffsetAlignment = vkInstance.mainInstance->GetOptimalImageCopyOffsetAlignment(physicalIndex);
	container->internalDriverDeviceListIdentifier = gpuIndex;

	RenderPhysicalDeviceIndex indexRet{};

	indexRet = physicalEntryIndex;

	return indexRet;
}

void RenderInstance::GetLastDeviceDriverError(RHIDevice* device, StringView messageHeader)
{
	internalRendererLogger->AddLogMessage(LOGERROR, messageHeader);

	int strLength = 0;

	char* string = device->device->PopErrorOffQueue(&strLength);

	internalRendererLogger->AddLogMessage(LOGERROR, string, strLength);
}

void RenderInstance::GetLastInstanceDriverError(StringView messageHeader)
{
	internalRendererLogger->AddLogMessage(LOGERROR, messageHeader);

	int strLength = 0;

	char* string = vkInstance.mainInstance->PopErrorOffQueue(&strLength);

	internalRendererLogger->AddLogMessage(LOGERROR, string, strLength);
}

uint32_t RenderInstance::BeginFrame(SwapChainIndex swapChainIndex)
{
	RenderSwapchainData* swcData = swapChains.Get(swapChainIndex);

	RHIDevice* rhiDevice = GetDeviceHandle(swcData->deviceIndex);

	VKDevice* dev = rhiDevice->device;

	int32_t res = dev->CommandBufferWaitOn(UINT64_MAX, rhiDevice->container.currentCommandBufferIndex[currentFrame]);

	uint32_t imageIndex = ~0UL;

	if (!res)
	{
		imageIndex = dev->BeginFrameForSwapchain(swcData->swapChainIdx, swcData->rendererWaitSemaphores[currentFrame], currentFrame);
	}

	if (imageIndex == ~0UL)
	{
		GetLastDeviceDriverError(rhiDevice, STRING_VIEW_FROM_LITERAL("BeginFrame failed:"));

		return imageIndex;
	}

	dev->CommandBufferResetFence(rhiDevice->container.currentCommandBufferIndex[currentFrame]);

	return imageIndex;
}

int RenderInstance::SubmitFrame(SwapChainIndex swapChainIndex, uint32_t imageIndex)
{
	RenderSwapchainData* swcData = swapChains.Get(swapChainIndex);

	RHIDevice* rhiDevice = GetDeviceHandle(swcData->deviceIndex);

	VKDevice* dev = rhiDevice->device;

	VkPipelineStageFlags waitStages[2] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

	int res = -1;

	if (EntryHandle() == rhiDevice->container.deviceTimelineSyncObject.driverTimelineObject)
	{
		res = dev->SubmitCommandBuffer(&swcData->rendererWaitSemaphores[currentFrame], &waitStages[0], &swcData->rendererFinishedSemaphores[imageIndex], 1, 1, rhiDevice->container.currentCommandBufferIndex[currentFrame]);
	}
	else
	{
		uint64_t waitCount[2] = { 0, rhiDevice->container.deviceTimelineSyncObject.currentValue };

		uint64_t signalCount[2] = { 0, rhiDevice->container.deviceTimelineSyncObject.currentValue + 1 };

		res = dev->SubmitCommandBuffer(
			&swcData->rendererWaitSemaphores[currentFrame], waitStages, 1,
			&rhiDevice->container.deviceTimelineSyncObject.driverTimelineObject, 1, waitCount,
			&swcData->rendererFinishedSemaphores[imageIndex], 1,
			&rhiDevice->container.deviceTimelineSyncObject.driverTimelineObject,
			1,
			signalCount,
			rhiDevice->container.currentCommandBufferIndex[currentFrame]
		);

		rhiDevice->container.deviceTimelineSyncObject.currentValue++;
	}

	if (res)
	{
		GetLastDeviceDriverError(rhiDevice, STRING_VIEW_FROM_LITERAL("SubmitFrame - Submit Command Buffer failed:"));
		return res;
	}

	if (rhiDevice->container.presentQueue != rhiDevice->container.graphicsComputeTransfer)
	{
		res = dev->PresentSwapChainSeparatePresentQueue(swcData->swapChainIdx, &swcData->rendererFinishedSemaphores[imageIndex], 1, imageIndex, currentFrame, rhiDevice->container.presentQueue);
	}
	else
	{
		res = dev->PresentSwapChainCommandBufferInline(swcData->swapChainIdx, &swcData->rendererFinishedSemaphores[imageIndex], 1, imageIndex, currentFrame, rhiDevice->container.currentCommandBufferIndex[currentFrame]);
	}

	if (res)
	{
		GetLastDeviceDriverError(rhiDevice, STRING_VIEW_FROM_LITERAL("SubmitFrame - Present failed:"));
		dev->CommandBufferWaitOn(UINT64_MAX, rhiDevice->container.currentCommandBufferIndex[currentFrame]);
	}

	return res;
}

RenderDeviceIndex RenderInstance::CreateLogicalDevice(LogicalDeviceCreateInfo* createInfo)
{
	RenderDeviceIndex ret{};

	if (MAX_FRAMES_IN_FLIGHT > MAX_INSTANCE_FRAME_IN_FLIGHT || createInfo->maxQueries > MAX_QUERY_RESULTS)
	{
		return ret;
	}

	if (maxLogicalDevices == logicalDeviceCounter)
	{
		internalRendererLogger->AddLogMessage(LOGERROR, STRING_VIEW_FROM_LITERAL("CreateLogicalDevice: too many device allocated"));
		return ret;
	}

	int physIndex = createInfo->physicalDeviceIndex.index;

	if (physIndex < 0 || physIndex >= maxPhysicalDevices)
	{
		internalRendererLogger->AddLogMessage(LOGERROR, STRING_VIEW_FROM_LITERAL("CreateLogicalDevice: gpu index not in range"));
		return ret;
	}

	RenderPhysicalDeviceContainer* physicalDevice = &physicalDeviceIndices[createInfo->physicalDeviceIndex.index];

	EntryHandle physicalIndex = physicalDevice->physicalDeviceIndex;

	int currentLogicalDeviceIndex = logicalDeviceCounter++;

	RHIDevice* rhiDevice = &logicalDeviceIndices[currentLogicalDeviceIndex];

	CleanInitializeRHIDevice(rhiDevice);

	rhiDevice->container.relatedPhysDeviceInfo = &physicalDevice->information;
	rhiDevice->container.gpuIndex = createInfo->physicalDeviceIndex;

	uint32_t deviceExtNameCount = vkInstance.mainInstance->GetLogicalDeviceExtensionsCount(createInfo->requestedDeviceFeatures);

	const char** deviceFeatureNames = (const char**)cacheAllocator->Allocate(sizeof(char*) * deviceExtNameCount);

	vkInstance.mainInstance->GetLogicalDeviceExtensions(createInfo->requestedDeviceFeatures, deviceFeatureNames);

	VkPhysicalDeviceVulkan12Features features12{};
	VkPhysicalDeviceFeatures2 features2{};

	API::ConvertGPUFeatureRequestToVkPhysicalDeviceProperties(createInfo->requestedPhysicalFeatures, &features2, &features12);

	EntryHandle deviceIndex = vkInstance.mainInstance->CreateLogicalDevice(physicalIndex);

	if (EntryHandle() == deviceIndex)
	{
		GetLastInstanceDriverError(STRING_VIEW_FROM_LITERAL("CreateLogicalDevice: device creation failed from instance"));
		logicalDeviceCounter--;
		return ret;
	}

	rhiDevice->container.logicalDeviceIndex = deviceIndex;

	VKDevice* majorDevice = vkInstance.mainInstance->GetLogicalDevice(deviceIndex);

	rhiDevice->device = majorDevice;

	uint32_t totalQueueFamilyCount = majorDevice->QueueFamilyDetailsCount();

	VkQueueFamilyProperties* famPropsContainer = (VkQueueFamilyProperties*)cacheAllocator->CAllocate(totalQueueFamilyCount * sizeof(VkQueueFamilyProperties));

	QueueIndex queueIndices[2]{};
	uint32_t queueCounts[2]{};

	uint32_t queueCount = 0, totalQueuePrios = 0;

	int queueSuccessful = majorDevice->GetQueueByMask(&queueIndices[0], &queueCounts[0], VK_QUEUE_COMPUTE_BIT | VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT, famPropsContainer);

	if (queueSuccessful)
	{
		internalRendererLogger->AddLogMessage(LOGERROR, STRING_VIEW_FROM_LITERAL("CreateLogicalDevice: Could not find a direct type	queue"));
		logicalDeviceCounter--;
		return ret;
	}

	queueSuccessful = majorDevice->GetPresentQueue(&queueIndices[1], &queueCounts[1], vkInstance.mainInstance->GetRenderSurface(windowsSurfaces[createInfo->surfaceIndexForPresent]()), famPropsContainer);

	if (queueSuccessful)
	{
		internalRendererLogger->AddLogMessage(LOGERROR, STRING_VIEW_FROM_LITERAL("CreateLogicalDevice: Could not find a present queue"));
		logicalDeviceCounter--;
		return ret;
	}

	if (queueIndices[0] == queueIndices[1])
	{
		queueCount = 1;
		totalQueuePrios = queueCounts[0];
	}
	else
	{
		queueCount = 2;
		totalQueuePrios = queueCounts[0] + queueCounts[1];
	}

	float* queuePriorites = reinterpret_cast<float*>(cacheAllocator->Allocate(sizeof(float) * totalQueuePrios));

	for (int i = 0; i < totalQueuePrios; i++)
	{
		queuePriorites[i] = 1.0f;
	}

	void* driverDeviceDataHead = AllocateFromStorageAllocator(createInfo->driverPermanentSize + createInfo->driverCacheSize, 64);
	void* deviceDataHead = AllocateFromStorageAllocator(createInfo->deviceInstPermanentSize + createInfo->deviceInstHandleSize + createInfo->deviceInstCacheSize + 192, 64);

	int createRet = majorDevice->CreateLogicalDevice(
		deviceFeatureNames,
		deviceExtNameCount,
		&features2,
		queueIndices,
		queueCounts,
		queuePriorites,
		queueCount,
		createInfo->deviceInstPermanentSize,
		createInfo->deviceInstHandleSize,
		createInfo->deviceInstCacheSize,
		createInfo->driverPermanentSize,
		createInfo->driverCacheSize,
		driverDeviceDataHead,
		deviceDataHead
	);

	if (createRet < 0)
	{
		GetLastDeviceDriverError(rhiDevice, STRING_VIEW_FROM_LITERAL("CreateLogicalDevice: device creation when creating logical device"));

		vkInstance.mainInstance->DestroyLogicalDevice(deviceIndex);

		logicalDeviceCounter--;

		return ret;
	}

	rhiDevice->container.graphicsComputeTransfer = majorDevice->CreateQueueManager(queueIndices[0], queueCounts[0], VK_QUEUE_COMPUTE_BIT | VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT, (queueCount == 1) ? true : false);

	if (queueCount > 1)
	{
		rhiDevice->container.presentQueue = majorDevice->CreateQueueManager(queueIndices[1], queueCounts[1], 0, true);
	}
	else
	{
		rhiDevice->container.presentQueue = rhiDevice->container.graphicsComputeTransfer;
	}

	for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		EntryHandle* lprimaryCommandBuffers = majorDevice->CreateReusableCommandBuffers(rhiDevice->container.graphicsComputeTransfer, 1, true, VK_COMMAND_BUFFER_LEVEL_PRIMARY);
		rhiDevice->container.currentCommandBufferIndex[i] = *lprimaryCommandBuffers;
	}

	rhiDevice->container.maxQueryResults = createInfo->maxQueries;

	rhiDevice->container.queryPoolIndex = majorDevice->CreateQueryPool(VK_QUERY_TYPE_TIMESTAMP, MAX_FRAMES_IN_FLIGHT * createInfo->maxQueries);

	rhiDevice->container.queriesAreActive = 0;

	if (createInfo->requestedPhysicalFeatures->requireTimelineSemaphores)
	{
		rhiDevice->container.deviceTimelineSyncObject.currentValue = 0;

		rhiDevice->container.deviceTimelineSyncObject.driverTimelineObject = *majorDevice->CreateTimelineSemaphores(1, rhiDevice->container.deviceTimelineSyncObject.currentValue);
	}

	ret.index = currentLogicalDeviceIndex;

	return ret;
}

int RenderInstance::CreateRenderPass(AttachmentGraphInstance* graphInstance)
{
	RHIDevice* rhiDevice = GetDeviceHandle(graphInstance->deviceIndex);

	VKDevice* dev = rhiDevice->device;

	AttachmentGraph* graph = graphInstance->graphLayout;

	AttachmentResource* resources = graph->resources;

	int totalRenderPassesCreated = 0;

	for (int b = 0; b < graph->passesCount; b++)
	{
		AttachmentRenderPassInstance* rpInst = &graphInstance->passes[b];

		int sampleCount = rpInst->maxSampleCount;

		totalRenderPassesCreated += sampleCount;
	}

	if (!renderPasses.DoIHaveNFreeElements(totalRenderPassesCreated))
	{
		return -1;
	}

	totalRenderPassesCreated = 0;

	for (int b = 0; b < graph->passesCount; b++)
	{
		AttachmentRenderPass* currentPassDesc = &graph->holders[b];

		int attachmentCount = currentPassDesc->attachmentCount;

		VKRenderPassBuilder rpb = dev->CreateRenderPassBuilder(attachmentCount, 1, 1);

		uint32_t currResolve = 0, currColor = 0, currDepthStencil = 0;

		AttachmentRenderPassInstance* rpInst = &graphInstance->passes[b];

		AttachmentInstance* currentPassInstance = rpInst->attachInst;

		int sampLo = 1;

		uint32_t* remappedIndices = (uint32_t*)cacheAllocator->CAllocate(sizeof(uint32_t) * attachmentCount);

		for (int c = 0; c < attachmentCount; c++)
		{
			AttachmentDescription* desc = &currentPassDesc->descs[c];

			AttachmentResource* resDesc = &graph->resources[desc->resourceIndex];

			VkFormat attachFormat = API::ConvertImageFormatToVulkanFormat(resources[desc->resourceIndex].format);

			VkSampleCountFlags vkSampleCountLo = (resDesc->msaa ? VK_SAMPLE_COUNT_2_BIT : VK_SAMPLE_COUNT_1_BIT);

			sampLo = RENDER_MAX((int)vkSampleCountLo, sampLo);

			AttachmentResourceInstance* currResource = &graphInstance->resources[desc->resourceIndex];

			VkImageLayout referenceLayout = VK_IMAGE_LAYOUT_UNDEFINED;

			switch (desc->attachType)
			{
			case ImageUsageFlagBits::COLOR_ATTACHMENT:
				remappedIndices[c] = currColor++;
				referenceLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				break;
			case ImageUsageFlagBits::RESOLVE_ATTACHMENT:
				remappedIndices[c] = (currentPassDesc->colorCount + currentPassDesc->depthStencilCount) + currResolve++;
				referenceLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				break;
			case ImageUsageFlagBits::DEPTH_ATTACHMENT:
			case ImageUsageFlagBits::STENCIL_ATTACHMENT:
			case ImageUsageFlagBits::DEPTH_ATTACHMENT | ImageUsageFlagBits::STENCIL_ATTACHMENT:
				remappedIndices[c] = (currentPassDesc->colorCount) + currDepthStencil++;
				referenceLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
				break;
			}

			VkAttachmentLoadOp dsvrtvLoad = API::ConvertAttachLoadOpToVulkanLoadOp(desc->loadOp);
			VkAttachmentLoadOp stencilLoad = dsvrtvLoad;

			VkAttachmentStoreOp dsvrtvStore = API::ConvertAttachStoreOpToVulkanStoreOp(desc->storeOp);
			VkAttachmentStoreOp stencilStore = dsvrtvStore;

			VkImageLayout srcLayout = API::ConvertImageLayoutToVulkanImageLayout(desc->srcLayout),
				dstLayout = API::ConvertImageLayoutToVulkanImageLayout(desc->dstLayout);

			rpb.CreateAttachment(
				referenceLayout,
				attachFormat, (VkSampleCountFlagBits)vkSampleCountLo,
				dsvrtvLoad, dsvrtvStore,
				stencilLoad, stencilStore,
				srcLayout, dstLayout, remappedIndices[c], remappedIndices[c]
			);

			currentPassInstance[remappedIndices[c]].descLayout = desc;
			currentPassInstance[remappedIndices[c]].attachmentResource = desc->resourceIndex;
		}

		rpb.CreateSubPassDescription(VK_PIPELINE_BIND_POINT_GRAPHICS, currentPassDesc->colorCount, currentPassDesc->resolveCount, currentPassDesc->depthStencilCount);

		rpb.CreateSubPassDependency(VK_SUBPASS_EXTERNAL, 0,
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, 0,
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
			VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT);

		rpb.CreateInfo();

		int renderPassSampleCount = 0;

		int sampleCount = rpInst->maxSampleCount;

		while (sampleCount--)
		{
			EntryHandle returnValue = dev->CreateRenderPasses(rpb);

			if (EntryHandle() == returnValue)
			{
				GetLastDeviceDriverError(logicalDeviceIndices, STRING_VIEW_FROM_LITERAL("RenderPass Creation Failed:"));

				return -1;
			}

			OldStyleRenderPassIndex rpIndex = renderPasses.Allocate();

			RenderOldStyleVulkanRenderPassInfo* info = renderPasses.Get(rpIndex);

			info->deviceIndex = graphInstance->deviceIndex;
			info->renderPassHandle = returnValue;

			rpInst->baseRenderPass[renderPassSampleCount] = rpIndex;

			sampLo <<= 1;

			for (int c = 0; c < attachmentCount; c++)
			{
				AttachmentDescription* desc = &currentPassDesc->descs[c];

				AttachmentResource* resDesc = &graph->resources[desc->resourceIndex];

				VkSampleCountFlags sampleCount = VK_SAMPLE_COUNT_1_BIT;

				if (resDesc->msaa)
				{
					sampleCount = sampLo;
				}

				rpb.SetSampleCount(remappedIndices[c], (VkSampleCountFlagBits)sampleCount);

			}

			renderPassSampleCount++;
		}

		totalRenderPassesCreated += renderPassSampleCount;
	}

	return totalRenderPassesCreated;
}

int DestroyDriverPhysicalDevice(RHIInstance* instance, EntryHandle handle)
{
	if (EntryHandle() == handle)
	{
		return -1;
	}
	instance->mainInstance->DestroyPhysicalDevice(handle);
	return 0;
}

int DestroyDriverLogicalDevice(RHIInstance* instance, EntryHandle handle)
{
	if (EntryHandle() == handle)
	{
		return -1;
	}
	instance->mainInstance->DestroyLogicalDevice(handle);
	return 0;
}

int DestroyDriverWindowsSurface(RHIInstance* instance, EntryHandle handle)
{
	if (EntryHandle() == handle)
	{
		return -1;
	}
	instance->mainInstance->DestroyRenderSurface(handle);
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

int WaitOnSwapChain(RHIDevice* device, EntryHandle swapChainIndex)
{
	VKSwapChain* swc = device->device->GetSwapChain(swapChainIndex);

	swc->Wait();

	return 0;
}

int WaitOnDevice(RHIDevice* device, uint64_t timeout)
{
	VKDevice* dev = device->device;

	dev->WaitOnDevice();

	return 0;
}

EntryHandle GetSwapChainViewHandles(RHIDevice* device, EntryHandle swapChainIndex, uint32_t imageIndex)
{
	VKSwapChain* swc = device->device->GetSwapChain(swapChainIndex);

	if (imageIndex >= swc->imageCount)
	{
		return EntryHandle();
	}

	return swc->imageViews[imageIndex];
}

uint32_t GetSwapChainImageCount(RHIDevice* device, EntryHandle swapChainIndex)
{
	VKSwapChain* swc = device->device->GetSwapChain(swapChainIndex);

	return swc->imageCount;
}

int GetImageMemorySizeAndAlignment(RHIDevice* device, DriverImageCreationInfo* info)
{
	device->device->GetImageMemorySizeAndAlignment(info->imageWidth, info->imageHeight,
		info->mipCount, API::ConvertImageFormatToVulkanFormat(info->format), info->layerCount,
		API::ConvertImageUsageFlagsToVulkanImageUsageFlags(info->usageFlags),
		info->sampleCount,
		API::ConvertImageLayoutToVulkanImageLayout(info->initialLayout),
		VK_IMAGE_TILING_OPTIMAL, (info->flags ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : 0),
		API::ConvertImageTypeToVulkanImageType(info->imageType), &info->imageSize, &info->imageAlignment);

	if (!info->imageSize || !info->imageAlignment)
	{
		return -1;
	}

	return 0;
}

EntryHandle CreateDriverImageHandle(RHIDevice* device, DriverImageCreationInfo* info)
{
	EntryHandle imageHandle = device->device->CreateImage(info->imageWidth, info->imageHeight,
			info->mipCount, API::ConvertImageFormatToVulkanFormat(info->format), info->layerCount,
			API::ConvertImageUsageFlagsToVulkanImageUsageFlags(info->usageFlags),
			info->sampleCount, info->imageAddress,
			API::ConvertImageLayoutToVulkanImageLayout(info->initialLayout),
			VK_IMAGE_TILING_OPTIMAL, (info->flags ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : 0),
			API::ConvertImageTypeToVulkanImageType(info->imageType), info->imagePoolHandle);

	return imageHandle;
}

EntryHandle CreateDriverImageViewHandle(RHIDevice* device, DriverImageViewCreationInfo* info)
{
	EntryHandle viewHandle = device->device->CreateImageView(info->textureIndex,
		info->firstMip, info->firstLayer,
		info->mipCount, info->layerCount,
		API::ConvertImageFormatToVulkanFormat(info->format),
		API::ConvertImageViewAspectMaskToVulkanImageAspectFlags(info->aspectMask),
		API::ConvertImageTypeToVulkanImageViewType(info->imageType)
	);

	return viewHandle;
}

EntryHandle CreateDriverBufferView(RHIDevice* device, EntryHandle bufferHandle, ComponentFormatType format, size_t viewSize, size_t offset, uint32_t copiesOfRangeSize)
{
	return device->device->CreateBufferView(bufferHandle, API::ConvertComponentFormatTypeToVulkanFormat(format), viewSize, offset, copiesOfRangeSize);
}

EntryHandle CreateDriverImageMemoryPool(RHIDevice* device, DriverImageMemoryPoolCreationInfo* info)
{
	VKDevice* dev = device->device;

	VkFormat vkFormat = API::ConvertImageFormatToVulkanFormat(info->format);
	VkImageUsageFlags vkUsageFlags = API::ConvertImageUsageFlagsToVulkanImageUsageFlags(info->usageFlags);
	VkMemoryPropertyFlags vkMemPropertyFlags = API::ConvertMemoryTypeToVkMemoryPropertyFlags(info->memoryType);

	MemoryTypeInfo poolInfo = dev->FindImageMemoryIndexForPool(
		info->maxWidth, info->maxHeight,
		(uint32_t)log2(RENDER_MIN(info->maxWidth, info->maxHeight)), vkFormat, info->maxArrayLayers,
		vkUsageFlags,
		1,
		vkMemPropertyFlags
	);

	if (~0ul == poolInfo.memoryIndex)
	{
		return EntryHandle();
	}

	return dev->CreateImageMemoryPool(info->poolSize, poolInfo.memoryIndex);
}

EntryHandle CreateDriverSampler(RHIDevice* device, DriverSamplerCreationInfo* info)
{
	VkSamplerAddressMode mode = API::ConvertSamplerAddressModeToVulkanSamplerAddressMode(info->addressMode);

	return device->device->CreateSampler(
		API::ConvertSamplerFilterModeToVulkanFilter(info->minFilter),
		API::ConvertSamplerFilterModeToVulkanFilter(info->magFilter),
		mode,
		mode,
		mode,
		API::ConvertCompareOpToVulkanCompareOp(info->compareOp),
		API::ConvertSamplerMipmapModeToVulkanSamplerMipmapMode(info->mipmapMode),
		static_cast<float>(info->baseLod),
		static_cast<float>(info->maxLod)
	);
}

EntryHandle CreateShaderCode(RHIDevice* device, char* shaderData, size_t length, ShaderStageType type)
{
	VkShaderStageFlags shaderFlags = API::ConvertShaderStageToVulkanShaderStage(type);

	return device->device->CreateShader(shaderData, length , shaderFlags);
}

int CreateDriverGraphicsPipeline(RHIDevice* device,
	GraphicsPipelineCreationInfo* info,
	EntryHandle* outHandles, uint32_t outHandlePointer
)
{
	uint32_t* pushConstantsSizes = (uint32_t*)info->tempAllocator->CAllocate(sizeof(uint32_t) * info->constantRangeCount);
	VkShaderStageFlags* shaderStages = (VkShaderStageFlags*)info->tempAllocator->CAllocate(sizeof(VkShaderStageFlags) * info->constantRangeCount);

	for (int i = 0; i < info->templateCount; i++)
	{
		ShaderResourceTemplate* resource = &info->templates[i];

		if (resource->type == ShaderResourceType::CONSTANT_BUFFER)
		{
			int rangeIndex = resource->rangeIndex;

			pushConstantsSizes[rangeIndex] += resource->size;

			shaderStages[rangeIndex] |= API::ConvertShaderStageToVulkanShaderStage(resource->stages);
		}
	}

	const uint32_t dynamicStateCount = 2;

	VKGraphicsPipelineBuilder* pipelineBuilder = device->device->CreateGraphicsPipelineBuilder(EntryHandle(), info->stateInfo->blendAttachmentCount, info->layoutCount, dynamicStateCount, info->constantRangeCount);

	uint32_t globalPushOffset = 0;

	for (int i = 0; i < info->constantRangeCount; i++)
	{
		pipelineBuilder->AddPushConstantRange(globalPushOffset, pushConstantsSizes[i], shaderStages[i], i);
		globalPushOffset += pushConstantsSizes[i];
	}

	VkDynamicState dynamicStates[dynamicStateCount] = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};

	pipelineBuilder->CreateDynamicStateInfo(dynamicStates, 2);

	VkVertexInputBindingDescription* bindingDescriptions = (VkVertexInputBindingDescription*)info->tempAllocator->Allocate(sizeof(VkVertexInputBindingDescription) * (info->stateInfo->vertexBufferDescCount));

	int descCount = 0;

	for (int i = 0; i < info->stateInfo->vertexBufferDescCount; i++)
	{
		bindingDescriptions[i] = VK::Utils::CreateVertexInputBindingDescription(i, info->stateInfo->vertexBufferDesc[i].perInputSize);

		descCount += info->stateInfo->vertexBufferDesc[i].descCount;
	}

	VkVertexInputAttributeDescription* vertexBufferInput = (VkVertexInputAttributeDescription*)info->tempAllocator->Allocate(sizeof(VkVertexInputAttributeDescription) * (descCount));

	int vertexBufferIter = 0;

	for (int i = 0; i < info->stateInfo->vertexBufferDescCount; i++)
	{
		API::ConvertVertexInputToVKVertexAttrDescription(info->stateInfo->vertexBufferDesc[i].descriptions, info->stateInfo->vertexBufferDesc[i].descCount, i, &vertexBufferInput[vertexBufferIter]);

		vertexBufferIter += info->stateInfo->vertexBufferDesc[i].descCount;
	}

	pipelineBuilder->CreateVertexInput(bindingDescriptions, info->stateInfo->vertexBufferDescCount, vertexBufferInput, descCount);

	pipelineBuilder->CreateInputAssembly(API::ConvertTopology(info->stateInfo->primType), false);

	pipelineBuilder->CreateViewportState(1, 1);

	pipelineBuilder->CreateRasterizer(API::ConvertCullMode(info->stateInfo->cullMode), API::ConvertTriangleWinding(info->stateInfo->windingOrder), info->stateInfo->lineWidth);

	for (int i = 0; i < info->stateInfo->blendAttachmentCount; i++)
	{
		BlendAttachments* attach = &info->stateInfo->blendAttachments[i];

		pipelineBuilder->CreateColorBlendAttachment(i, VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
			attach->blendingEnabled,
			API::ConvertBlendFactorToVulkanBlendFactor(attach->srcColorFactor),
			API::ConvertBlendFactorToVulkanBlendFactor(attach->dstColorFactor),
			API::ConvertBlendFactorToVulkanBlendFactor(attach->srcAlphaFactor),
			API::ConvertBlendFactorToVulkanBlendFactor(attach->dstAlphaFactor),
			API::ConvertBlendOpToVulkanBlendOp(attach->colorOp),
			API::ConvertBlendOpToVulkanBlendOp(attach->alphaOp)
		);
	}

	pipelineBuilder->CreateColorBlending(info->stateInfo->blendEnable, API::ConvertBlendLogicOpToVulkanLogicOp(info->stateInfo->blendOp));

	VkStencilOpState frontState = API::ConvertFaceStencilDataToVulkan(info->stateInfo->frontFace);
	VkStencilOpState backState = API::ConvertFaceStencilDataToVulkan(info->stateInfo->backFace);

	pipelineBuilder->CreateDepthStencil(API::ConvertCompareOpToVulkanCompareOp(info->stateInfo->depthTest), info->stateInfo->depthEnable, info->stateInfo->depthWrite, info->stateInfo->StencilEnable, &frontState, &backState);

	uint32_t sampleCount = info->renderPassMaxSampleCount;

	uint32_t lowSample = (info->renderPassMaxSampleCount > 1) ? 1 : 0;

	int pipelinesCreated = 0;

	for (; pipelinesCreated < sampleCount; pipelinesCreated++)
	{
		int msaaLevel = (1 << (lowSample + pipelinesCreated));
		if (msaaLevel > info->stateInfo->sampleCountHigh) break;

		pipelineBuilder->CreateMultiSampling((VkSampleCountFlagBits)msaaLevel);
		pipelineBuilder->renderPass = device->device->GetRenderPass(info->renderPasses[pipelinesCreated]);

		EntryHandle handle = pipelineBuilder->CreateGraphicsPipeline(info->layoutHandles, info->layoutCount, info->shaderHandles, info->shaderCount);

		if (EntryHandle() == handle)
		{
			pipelinesCreated = -1;

			break;
		}

		outHandles[outHandlePointer + pipelinesCreated] = handle;
	}

	return pipelinesCreated;
}

EntryHandle CreateDriverComputePipeline(RHIDevice* device, ComputePipelineCreationInfo* info)
{
	EntryHandle* layoutHandles = info->layouts;

	EntryHandle shaderHandle = info->shaderHandle;

	int pushRangeSize = info->pushRangeCount;

	int descriptorCount = info->layoutsCount;

	VKComputePipelineBuilder* pipelineBuilder = device->device->CreateComputePipelineBuilder(descriptorCount, pushRangeSize);

	uint32_t globalOffset = 0;

	for (int g = 0; g < pushRangeSize; g++)
	{
		pipelineBuilder->AddPushConstantRange(globalOffset, info->pushRangeSize[g], VK_SHADER_STAGE_COMPUTE_BIT, g);

		globalOffset += info->pushRangeSize[g];
	}

	return pipelineBuilder->CreateComputePipeline(layoutHandles, descriptorCount, shaderHandle);
}

int CreateDriverSwapChainData(RHIDevice* rhiDevice, EntryHandle swapChainIndex, uint32_t width, uint32_t height, bool recreate)
{
	VKDevice* dev = rhiDevice->device;

	VKSwapChain* swapChain = dev->GetSwapChain(swapChainIndex);

	int success = 0;

	if (recreate)
	{
		swapChain->ResetSwapChain();
		success = swapChain->RecreateSwapChain(width, height);
	}
	else
	{
		success = swapChain->CreateSwapChain(width, height, rhiDevice->container.graphicsComputeTransfer, rhiDevice->container.presentQueue);
	}

	if (!success)
	{
		EntryHandle* views = swapChain->CreateSwapchainViews();
		if (views) return success;
	}

	return -1;
}

EntryHandle CreateDriverShaderResourceLayout(RHIDevice* device, ShaderResourceSetTemplateCreator* creator)
{
	VKDevice* dev = device->device;

	DescriptorSetLayoutBuilder* descriptorBuilder = dev->CreateDescriptorSetLayoutBuilder(creator->bindingCount);

	for (int j = 0; j < creator->bindingCount; j++)
	{
		ResourceSetTemplateBindingInfo* info = &creator->info[j];

		VkShaderStageFlags stageFlags = API::ConvertShaderStageToVulkanShaderStage(info->stageType);

		int arrayCount = info->arrayCount;

		VkDescriptorBindingFlags bindingFlags = 0;

		if (arrayCount & UNBOUNDED_DESCRIPTOR_ARRAY)
		{
			bindingFlags |= VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT;
		}

		static bool useUpdateAfterBind = false;

		arrayCount &= DESCRIPTOR_COUNT_MASK;

		switch (info->resourceType)
		{
		case ShaderResourceType::UNIFORM_BUFFER:
			descriptorBuilder->AddBufferLayout(j, stageFlags, arrayCount, bindingFlags);
			break;
		case ShaderResourceType::IMAGESTORE2D:
			descriptorBuilder->AddStorageImageLayout(j, stageFlags, arrayCount, bindingFlags);
			break;
		case ShaderResourceType::IMAGE2D:
			if (useUpdateAfterBind)
			{
				bindingFlags |=
					VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT |
					VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT |
					VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT;

				descriptorBuilder->layoutFlags |= VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
			}
			descriptorBuilder->AddImageResourceLayout(j, stageFlags, arrayCount, bindingFlags);
			break;
		case ShaderResourceType::SAMPLERSTATE:
			if (useUpdateAfterBind)
			{
				bindingFlags |=
					VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT |
					VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT |
					VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT;

				descriptorBuilder->layoutFlags |= VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
			}
			descriptorBuilder->AddSamplerStateLayout(j, stageFlags, arrayCount, bindingFlags);

			break;
		case ShaderResourceType::SAMPLER2D:
		case ShaderResourceType::SAMPLER3D:
		case ShaderResourceType::SAMPLERCUBE:
			if (useUpdateAfterBind)
			{
				bindingFlags |=
					VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT |
					VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT |
					VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT;

				descriptorBuilder->layoutFlags |= VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
			}
			descriptorBuilder->AddBindlessCombinedSamplersLayout(j, stageFlags, arrayCount, bindingFlags);
			break;
		case ShaderResourceType::STORAGE_BUFFER:
			descriptorBuilder->AddStorageBufferLayout(j, stageFlags, arrayCount, bindingFlags);
			break;
		case ShaderResourceType::BUFFER_VIEW:
			if (info->action == ShaderResourceAction::SHADERREAD)
				descriptorBuilder->AddUniformBufferViewLayout(j, stageFlags, arrayCount, bindingFlags);
			else if (info->action == ShaderResourceAction::SHADERWRITE || info->action == ShaderResourceAction::SHADERREADWRITE)
				descriptorBuilder->AddStorageBufferViewLayout(j, stageFlags, arrayCount, bindingFlags);
			break;
		}
	}

	EntryHandle descHandle = dev->CreateDescriptorSetLayout(descriptorBuilder);

	return descHandle;
}

EntryHandle CreateDriverDescriptorHeap(RHIDevice* device, DescriptorTypes* types, uint32_t* descriptorCountPerFrame, uint32_t numDescriptorTypesCount, uint32_t maxDescriptorSets, uint32_t framesInFlight)
{
	DescriptorPoolBuilder builder = device->device->CreateDescriptorPoolBuilder(numDescriptorTypesCount, VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT);

	for (uint32_t i = 0; i < numDescriptorTypesCount; i++)
	{
		uint32_t individualCount = descriptorCountPerFrame[i];

		switch (types[i])
		{
		case DescriptorTypes::UNIFORM_DESCRIPTOR:
		{
			builder.AddUniformPoolSize(framesInFlight * individualCount);
			break;
		}
		case DescriptorTypes::UNORDERED_ACCESS_DESCRIPTOR:
		{
			builder.AddStoragePoolSize(framesInFlight * individualCount);
			break;
		}
		case DescriptorTypes::SAMPLED_IMAGE_DESCRIPTOR:
		{
			builder.AddSampledImage(framesInFlight * individualCount);
			break;
		}
		case DescriptorTypes::STORAGE_IMAGE_DESCRIPTOR:
		{
			builder.AddStorageImage(framesInFlight * individualCount);
			break;
		}
		case DescriptorTypes::SAMPLER_DESCRIPTOR:
		{
			builder.AddSampler(framesInFlight * individualCount);
			break;
		}
		case DescriptorTypes::COMBINED_IMAGE_SAMPLER_DESCRIPTOR:
		{
			builder.AddImageSamplerCombined(framesInFlight * individualCount);
			break;
		}
		default:
		{
			break;
		}
		}
	};

	return device->device->CreateDesciptorPool(&builder, framesInFlight * maxDescriptorSets);
}

EntryHandle CreateDriverBufferMemoryPool(RHIDevice* device, size_t poolSize, BufferUsage usage, MemoryType memoryType)
{
	return device->device->CreateBuffer(poolSize, API::ConvertBufferUsageToDriverBufferUsage(usage), API::ConvertMemoryTypeToVkMemoryPropertyFlags(memoryType));
}

int ReadDriverHostData(RHIDevice* device, EntryHandle bufferHandle, void* dataOut, size_t size, size_t offset)
{
	return device->device->ReadHostBuffer(dataOut, bufferHandle, size, offset);
}

EntryHandle CreateDriverSwapChain(RHIDevice* device, uint32_t requestedImageCount, uint32_t maxFramesInFlight, ImageFormat requestedFormat, EntryHandle renderSurfaceIndex)
{
	return device->device->CreateSwapChain(requestedImageCount, maxFramesInFlight, API::ConvertImageFormatToVulkanFormat(requestedFormat), renderSurfaceIndex);
}

EntryHandle* CreateDriverSemaphores(RHIDevice* device, uint32_t semaphoreCount)
{
	return device->device->CreateSemaphores(semaphoreCount);
}

int ReadBackQueryResults(RHIDevice* device, EntryHandle queryPoolIndex, uint32_t queryOffset, uint32_t queryCount, void* queryResults, size_t queryResultsSizeBytes, size_t individualQueryResultSize, int queryFlags)
{
	return device->device->ReadbackResultsFromQueries(
		queryPoolIndex,
		queryOffset,
		queryCount,
		queryResults,
		queryResultsSizeBytes,
		individualQueryResultSize,
		(queryFlags ? VK_QUERY_RESULT_WAIT_BIT : 0)
	);
}

int FindDriverSupportedDepthFormat(RHIInstance* instance, EntryHandle gpuIndex, ImageFormat format)
{
	VkFormat vkFormat = API::ConvertImageFormatToVulkanFormat(format);

	return instance->mainInstance->IsSupportedImageFormatForFeature(gpuIndex, vkFormat, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

int FindDriverSupportBackbufferFormat(RHIInstance* instance, EntryHandle gpuIndex, EntryHandle surfaceIndex, ImageFormat requestedFormat)
{
	VkFormat vkFormat = API::ConvertImageFormatToVulkanFormat(requestedFormat);

	bool ret = instance->mainInstance->ValidateSwapChainFormatSupport(gpuIndex, vkFormat, surfaceIndex);

	return ret;
}

static int WriteToDescriptorSet(VKDevice* dev, ShaderResourceSetInstanceCreator* creator, DescriptorSetBuilder* builder)
{
	int updateCount = creator->updateCount;

	for (int i = 0; i < updateCount; i++)
	{
		ShaderResourceAction action = creator->info[i].action;
		ShaderResourceType type = creator->info[i].resourceType;

		int bindingIndex = creator->info[i].bindingIndex;
		int copiesToMake = creator->info[i].descriptorCopies;
		int startForCopies = creator->info[i].perResourceDescriptorSlotStart;
		int destArraySlot = creator->info[i].destArraySlot;

		switch (type)
		{
		case ShaderResourceType::SAMPLERSTATE:
		{
			EntryHandle samplerHandle = creator->info[i].driverResourceHandle[0];

			builder->AddSamplerDescription(samplerHandle, destArraySlot, bindingIndex, startForCopies, copiesToMake);

			break;
		}
		case ShaderResourceType::IMAGE2D:
		{
			EntryHandle viewHandle = creator->info[i].driverResourceHandle[0];

			builder->AddImageResourceDescription(viewHandle, API::ConvertImageLayoutToVulkanImageLayout(creator->info[i].destinationLayout), destArraySlot, bindingIndex, startForCopies, copiesToMake);

			break;
		}
		case ShaderResourceType::IMAGESTORE2D:
		{
			EntryHandle viewHandle = creator->info[i].driverResourceHandle[0];

			builder->AddStorageImageDescription(viewHandle, API::ConvertImageLayoutToVulkanImageLayout(creator->info[i].destinationLayout), destArraySlot, bindingIndex, startForCopies, copiesToMake);

			break;
		}
		case ShaderResourceType::SAMPLER3D:
		case ShaderResourceType::SAMPLER2D:
		case ShaderResourceType::SAMPLERCUBE:
		{
			EntryHandle viewHandle = creator->info[i].driverResourceHandle[0];

			EntryHandle samplerHandle = creator->info[i].driverResourceHandle[1];

			builder->AddCombinedTextureArray(viewHandle, samplerHandle, API::ConvertImageLayoutToVulkanImageLayout(creator->info[i].destinationLayout), destArraySlot, bindingIndex, startForCopies, copiesToMake);

			break;
		}
		case ShaderResourceType::STORAGE_BUFFER:
		{
			EntryHandle buffer = creator->info[i].driverResourceHandle[0];

			if (creator->info[i].allocCopies > 1)
				builder->AddStorageBuffer(dev->GetBufferHandle(buffer), creator->info[i].allocSize, bindingIndex, copiesToMake, creator->info[i].allocOffset, startForCopies, destArraySlot);
			else
				builder->AddStorageBufferDirect(dev->GetBufferHandle(buffer), creator->info[i].allocSize, bindingIndex, copiesToMake, creator->info[i].allocOffset, startForCopies, destArraySlot);
			break;
		}
		case ShaderResourceType::UNIFORM_BUFFER:
		{
			EntryHandle buffer = creator->info[i].driverResourceHandle[0];

			if (creator->info[i].allocCopies > 1)
				builder->AddUniformBuffer(dev->GetBufferHandle(buffer), creator->info[i].allocSize, bindingIndex, copiesToMake, creator->info[i].allocOffset, startForCopies, destArraySlot);
			else
				builder->AddUniformBufferDirect(dev->GetBufferHandle(buffer), creator->info[i].allocSize, bindingIndex, copiesToMake, creator->info[i].allocOffset, startForCopies, destArraySlot);
			break;
		}
		case ShaderResourceType::BUFFER_VIEW:
		{
			int frameCount = creator->info[i].allocCopies;

			EntryHandle viewHandle = creator->info[i].driverResourceHandle[0];

			for (int g = 0; g < copiesToMake; g++)
			{
				VkBufferView handle = dev->GetBufferView(viewHandle, (frameCount > 1) ? startForCopies + g : 0);

				if (action == ShaderResourceAction::SHADERREAD)
				{
					builder->AddUniformBufferView(handle, bindingIndex, startForCopies + g, 1, destArraySlot);
				}
				else if (action == ShaderResourceAction::SHADERWRITE || action == ShaderResourceAction::SHADERREADWRITE)
				{
					builder->AddStorageBufferView(handle, bindingIndex, startForCopies + g, 1, destArraySlot);
				}
			}

			break;
		}
		}
	}

	return 0;
}

EntryHandle CreateDriverShaderResourceSet(RHIDevice* device, ShaderResourceSetInstanceCreator* creator, EntryHandle descriptorHeapIndex, EntryHandle descriptorLayoutHandle, uint32_t numberOfSubResourceHandle, uint32_t variableSizeRequestForLastDescriptor)
{
	VKDevice* dev = device->device;

	DescriptorSetBuilder* builder = device->device->CreateDescriptorSetBuilder(descriptorHeapIndex, descriptorLayoutHandle, numberOfSubResourceHandle, variableSizeRequestForLastDescriptor);

	WriteToDescriptorSet(dev, creator, builder);

	return builder->AddDescriptorsToCache();
}

int UpdateDriverShaderResourceSet(RHIDevice* device, ShaderResourceSetInstanceCreator* creator, EntryHandle descriptorSetHandle)
{
	VKDevice* dev = device->device;

	DescriptorSetBuilder* builder = device->device->UpdateDescriptorSet(descriptorSetHandle);

	WriteToDescriptorSet(dev, creator, builder);

	return 0;
}

void CreateDriverInstanceMemory(RHIInstance* instance, Allocator* allocator)
{
	instance->mainInstance = (VKInstance*)allocator->Allocate(sizeof(VKInstance), alignof(VKInstance));
}

void DestroyDriverInstance(RHIInstance* instance)
{
	if (instance->mainInstance) instance->mainInstance->~VKInstance();
}

int CreateDriverInstance(
	RHIInstance* instance, WindowManagementType windowType,
	uint32_t driverSpecificMemory, uint32_t driverCacheSize, 
	uint32_t instancePermanentSpecificMemory, uint32_t instanceCacheMemory, 
	Logger* logger, Allocator* allocator)
{
	void* driverInstanceDataHead = allocator->Allocate(driverSpecificMemory + driverCacheSize);
	void* instanceDataHead = allocator->Allocate(instancePermanentSpecificMemory + instanceCacheMemory);

	instance->mainInstance->SetInstanceDataAndSize(driverInstanceDataHead, driverSpecificMemory, driverCacheSize);

	VKInstanceDebugData vkDebugData{};

	vkDebugData.userCallback = vulkanDebugCallback;
	vkDebugData.userData = logger;
	vkDebugData.flags = 0;
	vkDebugData.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	vkDebugData.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT; // | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
	vkDebugData.enables[0] = VK_VALIDATION_FEATURE_ENABLE_DEBUG_PRINTF_EXT;
	vkDebugData.enablesFeaturesCount = 0;

	VKInstanceDebugData* vkDebugDataTemp = &vkDebugData;

	RenderingInstanceFeatures instanceFeaturesRequest{};

	instanceFeaturesRequest.useSurface = true;
	instanceFeaturesRequest.useSwapChainMaintenance = true;
	instanceFeaturesRequest.useValidation = true;
	instanceFeaturesRequest.useDebugExt = true;
	instanceFeaturesRequest.windowManagementType = windowType;

	int ret = instance->mainInstance->CreateRenderInstance(instanceDataHead, instancePermanentSpecificMemory, instanceCacheMemory, vkDebugDataTemp, &instanceFeaturesRequest);

	return ret;
}

EntryHandle CreateDriverWindowSurface(RHIInstance* instance, OSWindowInternalData* windowData)
{
#if defined(_WIN32)
	EntryHandle renderSurfaceIndex = instance->mainInstance->CreateWindowedSurface(windowData->inst, windowData->wnd);
#else
	EntryHandle renderSurfaceIndex = EntryHandle();
#endif
	return renderSurfaceIndex;
}