#include "RenderInstance.h"

#include <memory>

#include <string.h>
#include <assert.h>

#include "ShaderResourceSet.h"

#include "VKInstance.h"
#include "VKDevice.h"
#include "VKDescriptorLayoutBuilder.h"
#include "VKDescriptorSetBuilder.h"
#include "VKRenderPassBuilder.h"
#include "VKSwapChain.h"
#include "VKPipelineBuilder.h"

namespace GlobalRenderer 
{
	RenderInstance gRenderInstance;
}

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
}

#define RENDER_MIN(a, b) ((a) > (b) ? (b) : (a))
#define RENDER_MAX(a, b) ((a) < (b) ? (b) : (a))
#define RENDER_PWR2UP(size, align) (((size) + ((align)-1)) & ~((align)-1))

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

int findLSB(unsigned int input)
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

int findMSB(unsigned int input)
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

void GetAllocationDetails(RenderAllocation* alloc, size_t* requestedSize, size_t* offset, int* resourceIndex, int currentFrame)
{
	size_t rsize = alloc->requestedSize * alloc->structureCopies;
	size_t align = alloc->alignment;
	size_t iOffset = 0;
	int iResourceIndex = 0;
	rsize = RENDER_PWR2UP(rsize, align);

	if (alloc->allocType == AllocationType::PERFRAME)
	{
		iResourceIndex = currentFrame;
		iOffset = (currentFrame * rsize) + alloc->offset;
	}
	else if (alloc->allocType == AllocationType::STATIC)
	{
		iOffset = alloc->offset;
	}

	if (offset)
		*offset = iOffset;

	if (requestedSize)
		*requestedSize = rsize;

	if (resourceIndex)
		*resourceIndex = iResourceIndex;
}

void* RenderInstance::AllocateFromStorageAllocator(size_t size, size_t alignment)
{
	void* ret = storageAllocator->Allocate(size, alignment);
	return ret;
}

void* RenderInstance::AllocateFromStorageAllocator(size_t size)
{
	void* ret = storageAllocator->Allocate(size);
	return ret;
}

void RenderInstance::FreeFromStorageAllocator(void* address)
{
	storageAllocator->Free(address);
}

RHIDevice* RenderInstance::GetDeviceHandle(RenderDeviceIndex deviceSelection)
{
	RHIDevice* deviceContainer = &logicalDeviceIndices[deviceSelection.index];

	return deviceContainer;
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

	char* string = vkInstance->PopErrorOffQueue(&strLength);

	internalRendererLogger->AddLogMessage(LOGERROR, string, strLength);
}

void RenderInstance::CreateRenderInstance(RenderInstanceCreateInfo* info, Allocator* instanceStorageAllocator, RingAllocator* instanceCacheAllocator)
{
	cacheAllocator = instanceCacheAllocator;
	storageAllocator = instanceStorageAllocator;
	
	vkInstance = (VKInstance*)AllocateFromStorageAllocator(sizeof(VKInstance), alignof(VKInstance));

	internalRendererLogger = (Logger*)AllocateFromStorageAllocator(sizeof(Logger), alignof(Logger));
	internalRendererLogger->InitLogger((char*)AllocateFromStorageAllocator(info->internalLoggerRingSize, 64), info->internalLoggerRingSize);
	internalRendererLogger->fileHandle = info->internalRendererHandle;

	updateCommandsCache = (RingAllocator*)AllocateFromStorageAllocator(sizeof(RingAllocator), alignof(RingAllocator));

	StringView commandCacheName = STRING_VIEW_FROM_LITERAL("Commands Cache Allocator");

	std::construct_at(updateCommandsCache, AllocateFromStorageAllocator(info->commandsCacheSize, 64), info->commandsCacheSize, commandCacheName, internalRendererLogger);

	for (uint32_t i = 0; i < 2; i++)
	{
		updateCommandBuffers[i] = (SlabAllocator*)AllocateFromStorageAllocator(sizeof(SlabAllocator));

		StringView updateName = STRING_VIEW_FROM_LITERAL("Commands Cache Allocator");

		std::construct_at(updateCommandBuffers[i], AllocateFromStorageAllocator(info->commandBuffersSize, 32), info->commandBuffersSize, updateName, internalRendererLogger);
	}

	int driverHostLinkedSize = driverHostMemoryUpdater.GetSize(info->numberOfDriverHostAllocations);
	int commandLinkedSize = transferCommandPool.GetSize(info->numberOfTransferCommandAllocations);
	int resourceUpdateLinkedSize = descriptorUpdatePool.GetSize(info->numberOfResourceUpdateAllocations);
	int driverDeviceLinkedSize = driverDeviceMemoryUpdater.GetSize(info->numberOfDriverDeviceAllocations);
	int imageMemoryLinkedSize = imageMemoryUpdateManager.GetSize(info->numberOfImageMemoryAllocations);

	driverHostMemoryUpdater.AllocateList(AllocateFromStorageAllocator(driverHostLinkedSize, alignof(uintptr_t)), driverHostLinkedSize);
	transferCommandPool.AllocateList(AllocateFromStorageAllocator(commandLinkedSize, alignof(uintptr_t)), commandLinkedSize);
	driverDeviceMemoryUpdater.AllocateList(AllocateFromStorageAllocator(driverDeviceLinkedSize, alignof(uintptr_t)), driverDeviceLinkedSize);
	imageMemoryUpdateManager.AllocateList(AllocateFromStorageAllocator(imageMemoryLinkedSize, alignof(uintptr_t)), imageMemoryLinkedSize);
	descriptorUpdatePool.AllocateList(AllocateFromStorageAllocator(resourceUpdateLinkedSize, alignof(uintptr_t)), resourceUpdateLinkedSize);

	attachmentGraphsInstances.Create(storageAllocator, info->maxAttachmentGraphInstances, STRING_VIEW_FROM_LITERAL("Attachment Graph Instances Allocator"), internalRendererLogger);
	
	attachmentGraphs.Create(storageAllocator, info->maxAttachmentGraphTemplates, STRING_VIEW_FROM_LITERAL("Attachment Graph Templates Allocator"), internalRendererLogger);

	bufferHandles.Create(storageAllocator, info->maxBufferPoolsCount, STRING_VIEW_FROM_LITERAL("Buffer Handles Allocator"), internalRendererLogger);

	imagePools.Create(storageAllocator, info->maxImagePoolsCount, STRING_VIEW_FROM_LITERAL("Image Pools Allocator"), internalRendererLogger);

	pipelineHandles.Create(storageAllocator, info->maxPipelineHandles, STRING_VIEW_FROM_LITERAL("Pipeline Handles Allocator"), internalRendererLogger);

	gpuCommandStreams.Create(storageAllocator, info->maxGPUCommandsStreams, STRING_VIEW_FROM_LITERAL("GPU Command Streams Allocator"), internalRendererLogger);

	renderTargetQueues.Create(storageAllocator, info->maxRenderQueues, STRING_VIEW_FROM_LITERAL("Render Target Queues Allocator"), internalRendererLogger);

	computeQueues.Create(storageAllocator, info->maxComputeQueues, STRING_VIEW_FROM_LITERAL("Compute Queues Allocator"), internalRendererLogger);

	samplerResourceHandles.Create(storageAllocator, info->maxSamplerHandles, STRING_VIEW_FROM_LITERAL("Sampler Resource Handles Allocator"), internalRendererLogger);

	textureResourceHandles.Create(storageAllocator, info->maxTextureHandles, STRING_VIEW_FROM_LITERAL("Texture Resource Handles Allocator"), internalRendererLogger);

	textureViewsResourceHandles.Create(storageAllocator, info->maxTextureHandles, STRING_VIEW_FROM_LITERAL("Texture Views Resource Handles Allocator"), internalRendererLogger);

	resourceStatuses.Create(storageAllocator, info->maxResourceStatuses, STRING_VIEW_FROM_LITERAL("Resource Statuses Allocator"), internalRendererLogger);

	pipelineInfos.Create(storageAllocator, info->maxPipelineTemplates, STRING_VIEW_FROM_LITERAL("Pipeline Infos Allocator"), internalRendererLogger);

	mainRenderTargets.Create(storageAllocator, info->maxRenderTargets, STRING_VIEW_FROM_LITERAL("Main Render Targets Allocator"), internalRendererLogger);

	renderPasses.Create(storageAllocator, info->maxRenderTargets, STRING_VIEW_FROM_LITERAL("Render Passes Allocator"), internalRendererLogger);

	shaderResourceTemplates.Create(storageAllocator, info->maxShaderResourceTemplates, STRING_VIEW_FROM_LITERAL("Shader Resource Templates Allocator"), internalRendererLogger);

	allocations.Create(storageAllocator, info->maxAllocations + info->maxSubAllocations, STRING_VIEW_FROM_LITERAL("Allocations Allocator"), internalRendererLogger);

	descriptorManagers.Create(storageAllocator, info->maxDescriptorManagers, STRING_VIEW_FROM_LITERAL("Descriptor Managers Allocator"), internalRendererLogger);

	shaderGraphs.Create(storageAllocator, info->maxShaderGraphs, info->maxShaderHandles, STRING_VIEW_FROM_LITERAL("Shader Graphs Allocator"), STRING_VIEW_FROM_LITERAL("Shader Handles Allocator"), internalRendererLogger);

	windowsSurfaces.Create(storageAllocator, info->maxWindows, STRING_VIEW_FROM_LITERAL("Windows Surfaces Allocator"), internalRendererLogger);

	swapChains.Create(storageAllocator, info->maxSwapChains, STRING_VIEW_FROM_LITERAL("Swap Chains Allocator"), internalRendererLogger);

	graphPipelineDescriptions.Create(storageAllocator, info->maxPipelineInstances, STRING_VIEW_FROM_LITERAL("Pipeline Graph Instance Allocator"), internalRendererLogger);

	physicalDeviceIndices = (RenderPhysicalDeviceContainer*)AllocateFromStorageAllocator(sizeof(RenderPhysicalDeviceContainer) * info->maxGPUS, alignof(RenderPhysicalDeviceContainer));

	logicalDeviceIndices = (RHIDevice*)AllocateFromStorageAllocator(sizeof(RHIDevice) * info->maxLogicalDevices, alignof(RHIDevice));

	maxLogicalDevices = info->maxLogicalDevices;
	maxPhysicalDevices = info->maxGPUS;

	barriersQueue = (uint32_t*)AllocateFromStorageAllocator(sizeof(uint32_t) * info->maxConcurrentRecordings);

	barrierAccumulators = (BarrierAccumulator*)AllocateFromStorageAllocator(sizeof(BarrierAccumulator) * info->maxConcurrentRecordings);

	maxBarrierAccumulationCount = info->maxConcurrentRecordings;

	currentBarrierAccumulationTop = 0;

	for (uint32_t i = 0; i < info->maxConcurrentRecordings; i++)
	{
		CreateDriverSpecificBarrierArenas(&barrierAccumulators[i], info->maxTextureHandles, info->maxAllocations);
		barriersQueue[i] = i;
	}

	return;
}

#define MAX_MIPS_FOR_BARRIER 16
#define MAX_ARRAYS_FOR_BARRIER 8

void RenderInstance::CreateDriverSpecificBarrierArenas(BarrierAccumulator* barrierAccumulator, int maxTextures, int maxAllocations)
{
	barrierAccumulator->accumulators[BUFFER_BARRIER_ACCUMULATOR].allocator = (SlabAllocator*)AllocateFromStorageAllocator(sizeof(SlabAllocator), alignof(SlabAllocator));
	barrierAccumulator->accumulators[IMAGE_BARRIER_ACCUMULATOR].allocator = (SlabAllocator*)AllocateFromStorageAllocator(sizeof(SlabAllocator), alignof(SlabAllocator));

	int imageSize = (sizeof(VkImageMemoryBarrier) * MAX_ARRAYS_FOR_BARRIER * MAX_MIPS_FOR_BARRIER * maxTextures) + sizeof(PipelineStage) * 2;

	int bufferSize = (sizeof(VkBufferMemoryBarrier) * maxAllocations) + sizeof(PipelineStage) * 2;

	barrierAccumulator->intraPassCount = 0;
	barrierAccumulator->intraPassTop = 0;

	StringView imgBarrierName = STRING_VIEW_FROM_LITERAL("Image Barrier Allocator");
	StringView bufBarrierName = STRING_VIEW_FROM_LITERAL("Buffer Barrier Allocator");
	StringView intraBarrierName = STRING_VIEW_FROM_LITERAL("Intra Pass Barrier Allocator");

	std::construct_at(barrierAccumulator->accumulators[IMAGE_BARRIER_ACCUMULATOR].allocator, AllocateFromStorageAllocator(imageSize, alignof(VkImageMemoryBarrier)), imageSize, imgBarrierName, internalRendererLogger);
	std::construct_at(barrierAccumulator->accumulators[BUFFER_BARRIER_ACCUMULATOR].allocator, AllocateFromStorageAllocator(bufferSize, alignof(VkBufferMemoryBarrier)), bufferSize, bufBarrierName, internalRendererLogger);
	std::construct_at(&barrierAccumulator->intraPassBarrierAllocator, AllocateFromStorageAllocator(12 * KiB, alignof(VkBufferMemoryBarrier)), 12 * KiB, intraBarrierName, internalRendererLogger);

	for (int i = 0; i < MAX_INTRA_PASS_BARRIERS; i++)
	{
		barrierAccumulator->intraPassBarriers[i].pipelineInst = -1;
		barrierAccumulator->intraPassBarriers[i].barrierType = BarrierType::NULL_BARRIER;
		barrierAccumulator->intraPassBarriers[i].barrierCount = 0;
	}

	barrierAccumulator->intraPassCount = 0;
	barrierAccumulator->intraPassTop = 0;
	barrierAccumulator->intraPassBarrierAllocator.Reset();
}

uint32_t RenderInstance::PopBarrierAccumulator()
{
	if (currentBarrierAccumulationTop == maxBarrierAccumulationCount)
		return ~0ul;

	uint32_t barrierAccumIndex = barriersQueue[currentBarrierAccumulationTop++];

	BarrierAccumulator* barrierAccumulator = &barrierAccumulators[barrierAccumIndex];

	ResetIntraBarrierAccumulator(barrierAccumulator);

	return barrierAccumIndex;
}

void RenderInstance::ResetIntraBarrierAccumulator(BarrierAccumulator* accumulator)
{
	for (int i = 0; i < accumulator->intraPassCount; i++)
	{
		accumulator->intraPassBarriers[i].pipelineInst = -1;
		accumulator->intraPassBarriers[i].barrierType = BarrierType::NULL_BARRIER;
		accumulator->intraPassBarriers[i].barrierCount = 0;
	}

	accumulator->intraPassCount = 0;
	accumulator->intraPassTop = 0;
	accumulator->intraPassBarrierAllocator.Reset();
}

void RenderInstance::ReturnBarrierAccumulator(uint32_t returnIndex)
{
	if (!currentBarrierAccumulationTop)
		return;

	barriersQueue[--currentBarrierAccumulationTop] = returnIndex;
}

RenderInstance::~RenderInstance()
{
	if (vkInstance) vkInstance->~VKInstance();
};

void RenderInstance::DestroySwapChainAttachments()
{
	for (int a = 0; a < attachmentGraphsInstances.count; a++) 
	{
		AttachmentGraphInstanceIndex index = a;

		AttachmentGraphInstance* graph = attachmentGraphsInstances.Get(index);

		RHIDevice* rhiDevice = GetDeviceHandle(graph->deviceIndex);

		AttachmentResourceInstance* rescs = graph->resources;
		AttachmentResource* descs = graph->graphLayout->resources;

		for (uint32_t c = 0; c < graph->graphLayout->resourceCount; c++)
		{
			AttachmentResourceInstance* inst = &rescs[c];
			AttachmentResource* desc = &descs[c];

			if (desc->viewType == AttachmentViewType::SWAPCHAIN)
				continue;

			int sampLo = inst->sampLo;
			int sampHi = inst->sampHi;

			int sampIndex = 0;

			while (sampLo <= sampHi)
			{
				for (uint32_t d = 0; d < inst->imageCount; d++)
				{
					RenderTextureDescription* texDesc = textureResourceHandles.Get(inst->textureIds[sampIndex][d]);

					for (int viewIndex = 0; viewIndex < texDesc->viewCount; viewIndex++)
					{
						RenderImageViewDescription* imageViewDesc = textureViewsResourceHandles.Get(texDesc->viewIndex[viewIndex]);
						
						DestroyDriverImageView(rhiDevice, imageViewDesc->viewIndex);
						
						textureViewsResourceHandles.Free(texDesc->viewIndex[viewIndex]);
					}

					DestroyDriverImage(rhiDevice, texDesc->textureIndex);
					
					resourceStatuses.Free(texDesc->resourceStatusIndex);

					textureResourceHandles.Free(inst->textureIds[sampIndex][d]);
				}

				sampLo <<= 1;

				sampIndex++;
			}
		}
	}
}

int RenderInstance::RecreateSwapChain(SwapChainIndex& swapChainIndex, uint32_t width, uint32_t height)
{
	int ret = 0;

	RenderSwapchainData* data = swapChains.Get(swapChainIndex);

	RHIDevice* rhiDevice = GetDeviceHandle(data->deviceIndex);

	VKDevice* dev = rhiDevice->device;

	if (!data)
	{
		return ret;
	}

	if (width && height) 
	{
		VKSwapChain* swc = dev->GetSwapChain(data->swapChainIdx);
		
		swc->Wait();

		DestroySwapChainAttachments();

		CreateDriverSwapChainData(rhiDevice, data->swapChainIdx, width, height, true);

		for (uint32_t i = 0; i < swc->imageCount; i++)
		{
			RenderTextureDescription* desc = textureResourceHandles.Get(data->textureIds[i]);

			RenderImageViewDescription* viewDesc = textureViewsResourceHandles.Get(desc->viewIndex[0]);

			viewDesc->viewIndex = swc->imageViews[i];
		}

		data->width = width;

		data->height = height;

		ret = 1;
	}

	return ret;
}

AttachmentGraphInstanceIndex RenderInstance::CreateAttachmentGraphInstance(RenderDeviceIndex deviceSelection, AttachmentGraph* graph)
{
	RHIDevice* rhiDevice = GetDeviceHandle(deviceSelection);

	int totalAttachmentCount = 0;

	int totalRenderTargetsCreated = 0;

	for (int b = 0; b < graph->passesCount; b++)
	{
		AttachmentRenderPass* currentPassDesc = &graph->holders[b];

		int attachmentCount = currentPassDesc->attachmentCount;

		totalAttachmentCount += attachmentCount;

		int sampHi = 1;

		for (int c = 0; c < attachmentCount; c++)
		{
			AttachmentDescription* desc = &currentPassDesc->descs[c];

			AttachmentResource* resDesc = &graph->resources[desc->resourceIndex];

			int sampleCountHi = (resDesc->msaa ? (1 << (rhiDevice->container.relatedPhysDeviceInfo->maxMSAALevels)) : 1);

			sampHi = RENDER_MIN(RENDER_MAX(sampleCountHi, sampHi), MAX_SAMPLE_COUNT);
		}

		int renderPassSampleCount = RENDER_MAX(findMSB(sampHi), 1);

		totalRenderTargetsCreated += renderPassSampleCount;
	}

	bool renderTargetBaseAddress = mainRenderTargets.DoIHaveNFreeElements(totalRenderTargetsCreated);

	if (!renderTargetBaseAddress)
	{
		return {};
	}

	AttachmentGraphInstanceIndex attachmentInstanceIndex = attachmentGraphsInstances.Allocate();

	if (AttachmentGraphInstanceIndex() == attachmentInstanceIndex)
	{
		return {};
	}

	AttachmentGraphInstance* graphInstance = attachmentGraphsInstances.Get(attachmentInstanceIndex);

	CleanInitializeAttachmentGraphsInstance(graphInstance);

	graphInstance->graphLayout = graph;

	graphInstance->deviceIndex = deviceSelection;

	totalRenderTargetsCreated = 0;

	totalAttachmentCount = 0;

	for (int b = 0; b < graph->passesCount; b++)
	{
		AttachmentRenderPass* currentPassDesc = &graph->holders[b];

		int attachmentCount = currentPassDesc->attachmentCount;

		AttachmentRenderPassInstance* rpInst = &graphInstance->passes[b];

		rpInst->attachInstCount = attachmentCount;

		totalAttachmentCount += attachmentCount;

		int sampLo = 1, sampHi = 1;

		for (int c = 0; c < attachmentCount; c++)
		{
			AttachmentDescription* desc = &currentPassDesc->descs[c];

			AttachmentResource* resDesc = &graph->resources[desc->resourceIndex];

			AttachmentResourceInstance* currResource = &graphInstance->resources[desc->resourceIndex];

			int sampleCountLo = (resDesc->msaa ? 2 : 1);

			int sampleCountHi = (resDesc->msaa ? (1 << (rhiDevice->container.relatedPhysDeviceInfo->maxMSAALevels)) : 1);

			sampHi = RENDER_MIN(RENDER_MAX(sampleCountHi, sampHi), MAX_SAMPLE_COUNT);

			currResource->usage = desc->attachType;
			currResource->sampLo = sampleCountLo;
			currResource->sampHi = sampleCountHi;

			if (desc->dstLayout == ImageLayout::SHADERREADABLE)
			{
				currResource->usage |= ImageUsageFlagBits::SAMPLED;
			}
			else
			{
				currResource->usage |= ImageUsageFlagBits::TRANSIENT_ATTACHMENT;
			}
		}

		int renderPassSampleCount = RENDER_MAX(findMSB(sampHi), 1);

		rpInst->maxSampleCount = renderPassSampleCount;
	}
 
	return attachmentInstanceIndex;
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

	VkPipelineStageFlags waitStages[2] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

	int res = -1;
	
	if (EntryHandle() == rhiDevice->container.deviceTimelineSyncObject.driverTimelineObject)
	{
		res = dev->SubmitCommandBuffer(&swcData->rendererWaitSemaphores[currentFrame], &waitStages[0], &swcData->rendererFinishedSemaphores[imageIndex], 1, 1, rhiDevice->container.currentCommandBufferIndex[currentFrame]);
	}
	else
	{
		uint64_t waitCount[2] = {0, rhiDevice->container.deviceTimelineSyncObject.currentValue};

		uint64_t signalCount[2] = {0, rhiDevice->container.deviceTimelineSyncObject.currentValue + 1};

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

void RenderInstance::WaitOnRender(RenderDeviceIndex deviceSelection)
{
	RHIDevice* rhiDevice = GetDeviceHandle(deviceSelection);

	VKDevice* dev = rhiDevice->device;

	dev->WaitOnDevice();
}

int RenderInstance::CreateSwapChainAttachment(AttachmentGraphInstanceIndex& graphIndex, int renderPassIndex, SwapChainIndex swapChainIndex, AttachmentClear* clears, DeviceSlabAllocator* rtvAllocator, DeviceSlabAllocator* dsvAllocator, ImageMemoryIndex rtvPoolIndex, ImageMemoryIndex dsvPoolIndex)
{
	RenderSwapchainData* swcData = swapChains.Get(swapChainIndex);

	if (!swcData)
	{
		return -1;
	}

	return CreateAttachmentResources(graphIndex, renderPassIndex, swcData->imageCount, swcData->textureIds, swcData->width, swcData->height, RenderPassType::SWAPCHAIN_IMAGE_COUNT, clears, rtvAllocator, dsvAllocator, rtvPoolIndex, dsvPoolIndex);
}

int RenderInstance::CreatePerFrameAttachment(AttachmentGraphInstanceIndex& graphIndex, int renderPassIndex, int imageCount, uint32_t width, uint32_t height, AttachmentClear* clears, DeviceSlabAllocator* rtvAllocator, DeviceSlabAllocator* dsvAllocator, ImageMemoryIndex rtvPoolIndex, ImageMemoryIndex dsvPoolIndex)
{
	return CreateAttachmentResources(graphIndex, renderPassIndex, imageCount, nullptr, width, height, RenderPassType::PER_FRAME_IMAGE_COUNT, clears, rtvAllocator, dsvAllocator, rtvPoolIndex, dsvPoolIndex);
}

int RenderInstance::CreateResourceStatusActions(ResourceStatus* status, int numberOfCurrentActions, int numberOfCurrentStages, int numberOfCurrentLayouts)
{
	status->currAction = (BarrierAction*)AllocateFromStorageAllocator(sizeof(BarrierAction) * numberOfCurrentActions);

	if (status->currAction)
	{
		status->currStage = (PipelineStage*)AllocateFromStorageAllocator(sizeof(PipelineStage) * numberOfCurrentStages);

		if (status->currStage)
		{
			status->currentLayout = (ImageLayout*)AllocateFromStorageAllocator(sizeof(ImageLayout) * numberOfCurrentLayouts);

			if (status->currentLayout)
			{
				return 0;
			}
		}
	}

	internalRendererLogger->AddLogMessage(LOGERROR, STRING_VIEW_FROM_LITERAL("CreateResourceStatusAction: out of memory for trackings"));
	
	storageAllocator->Free(status->currAction);
	storageAllocator->Free(status->currStage);
	storageAllocator->Free(status->currentLayout);

	status->currAction = nullptr;
	status->currStage = nullptr;
	status->currentLayout = nullptr;

	return -1;
}

void RenderInstance::InitializeResourceStatus(ResourceStatus* status, int numberOfCurrentActions, int numberOfCurrentStages, int numberOfCurrentLayouts, BarrierAction action, PipelineStage stage, ImageLayout imageLayout)
{
	for (int i = 0; i < numberOfCurrentActions; i++)
		status->currAction[i] = action;

	for (int i = 0; i < numberOfCurrentStages; i++)
		status->currStage[i] = stage;

	for (int i = 0; i < numberOfCurrentLayouts; i++)
		status->currentLayout[i] = imageLayout;
}

TextureIndex RenderInstance::CreateAttachmentImage
(
	uint32_t width, uint32_t height, 
	uint32_t arrayLayers, uint32_t mipCount,
	ImageType imageType, int sampleCount, 
	ImageFormat format, ImageUsageFlags usageFlags, 
	DeviceSlabAllocator* attachmentAllocator, ImageLayout initialLayout,
	RenderDeviceIndex devSelection, ImageMemoryIndex imageMemoryPoolIndex, ResourceStatusType resourceType)
{
	RHIDevice* dev = GetDeviceHandle(devSelection);

	TextureIndex textureIndex = textureResourceHandles.Allocate();

	ResourceIndex resourceStatus = resourceStatuses.Allocate();

	if (TextureIndex() == textureIndex || ResourceIndex() == resourceStatus)
	{
		resourceStatuses.Free(resourceStatus);
		textureResourceHandles.Free(textureIndex);
		return {};
	}

	uint32_t totalResourceCount = mipCount * arrayLayers;

	ResourceStatus* status = resourceStatuses.Get(resourceStatus);

	CleanInitializeResourceStatus(status);

	RenderTextureDescription* desc = textureResourceHandles.Get(textureIndex);

	CleanInitializeTextureResourceHandle(desc);
	
	desc->resourceStatusIndex = resourceStatus;

	TextureIndex indexRet(textureIndex);

	int createdResourceStatus = CreateResourceStatusActions(status, totalResourceCount, totalResourceCount, totalResourceCount);

	if (createdResourceStatus < 0)
	{
		DestroyTextureResourceHandle(indexRet);
		return {};
	}

	VkFormat vkAttachmentFormat = API::ConvertImageFormatToVulkanFormat(format);

	VkImageType vkImageType = API::ConvertImageTypeToVulkanImageType(imageType);

	VkImageUsageFlags vkUsageFlags = API::ConvertImageUsageFlagsToVulkanImageUsageFlags(usageFlags);

	VkImageLayout vkInitialLayot = API::ConvertImageLayoutToVulkanImageLayout(initialLayout);

	size_t actualImageSize = 0, actualImageAlignment = 0;

	dev->device->GetImageMemorySizeAndAlignment(width, height,
		mipCount, vkAttachmentFormat, arrayLayers,
		vkUsageFlags,
		sampleCount,
		vkInitialLayot,
		VK_IMAGE_TILING_OPTIMAL, 0,
		vkImageType, &actualImageSize, &actualImageAlignment);

	if (!actualImageSize || !actualImageAlignment)
	{
		GetLastDeviceDriverError(dev, STRING_VIEW_FROM_LITERAL("CreateAttachmentImage: GetImageMemorySizeAndAlignment failed"));
		DestroyTextureResourceHandle(indexRet);
		return {};
	}

	size_t actualMemAddr = attachmentAllocator->Allocate(actualImageSize, actualImageAlignment);

	if (actualMemAddr < 0)
	{
		internalRendererLogger->AddLogMessage(LOGERROR, STRING_VIEW_FROM_LITERAL("Create Attachment Image: Allocator Failed"));
		DestroyTextureResourceHandle(indexRet);
		return {};
	}

	EntryHandle imageHandle = dev->device->CreateImage(
		width, height,
		mipCount, vkAttachmentFormat, arrayLayers,
		vkUsageFlags,
		sampleCount, actualMemAddr,
		vkInitialLayot,
		VK_IMAGE_TILING_OPTIMAL, 0,
		vkImageType, imagePools[imageMemoryPoolIndex].imagePoolHandle
	);

	if (EntryHandle() == imageHandle)
	{
		GetLastDeviceDriverError(dev, STRING_VIEW_FROM_LITERAL("CreateAttachmentImage: CreateImage failed"));
		DestroyTextureResourceHandle(indexRet);
		return {};
	}

	desc->arrayLayers = arrayLayers;
	desc->mipLayers = mipCount;
	desc->imageHeight = height;
	desc->imageWidth = width;
	desc->format = format;
	desc->textureIndex = imageHandle;
	desc->imageType = imageType;
	desc->viewCount = 0;
	desc->deviceIndex = devSelection;

	status->resourceType = resourceType;

	InitializeResourceStatus(status, totalResourceCount, totalResourceCount, totalResourceCount, 0, BEGINNING_OF_PIPE, initialLayout);

	return indexRet;
}

int RenderInstance::CreateAttachmentImageView(TextureIndex& textureIndex, uint32_t firstMip, uint32_t mipCount, uint32_t firstArrayLayer, uint32_t arrayLayerCount, ImageViewAspectMask mask, ImageLayout desiredLayout)
{
	TextureViewIndex viewIndex;

	RenderTextureDescription* desc = textureResourceHandles.Get(textureIndex);

	if (!desc)
	{
		return -1;
	}

	if (desc->viewCount == MAX_VIEWS_ATTACHED_TO_TEXTURE)
	{
		internalRendererLogger->AddLogMessage(LOGERROR, STRING_VIEW_FROM_LITERAL("Create Attachment View -- Too Many Textures Views"));
		return -1;
	}

	viewIndex =  textureViewsResourceHandles.Allocate();

	if (TextureViewIndex() == viewIndex)
	{
		return -1;
	}

	RHIDevice* dev = GetDeviceHandle(desc->deviceIndex);

	VkFormat vkAttachmentFormat = API::ConvertImageFormatToVulkanFormat(desc->format);

	VkImageViewType vkImageViewType = API::ConvertImageTypeToVulkanImageViewType(desc->imageType);

	VkImageAspectFlags aspectFlags = API::ConvertImageViewAspectMaskToVulkanImageAspectFlags(mask);

	EntryHandle imageViewHandle = dev->device->CreateImageView(desc->textureIndex, firstMip, firstArrayLayer, mipCount, arrayLayerCount, vkAttachmentFormat, aspectFlags, vkImageViewType);

	if (EntryHandle() == imageViewHandle)
	{
		GetLastDeviceDriverError(dev, STRING_VIEW_FROM_LITERAL("Create Attachment View -- Image View Created Failed"));
		textureViewsResourceHandles.Free(viewIndex);
		return -1;
	}

	int texViewCount = desc->viewCount++;

	desc->viewIndex[texViewCount] = viewIndex;

	RenderImageViewDescription* imageViewDesc = textureViewsResourceHandles.Get(viewIndex);

	CleanInitializeTextureViewsResourceHandle(imageViewDesc);

	imageViewDesc->firstLayer = firstArrayLayer;
	imageViewDesc->firstMipLevel = firstMip;
	imageViewDesc->layerCount = arrayLayerCount;
	imageViewDesc->mipLevelCount = mipCount;
	imageViewDesc->mask = mask;
	imageViewDesc->viewIndex = imageViewHandle;
	imageViewDesc->desiredLayoutForView = desiredLayout;

	return texViewCount;
}

int RenderInstance::CreateAttachmentImageView(AttachmentGraphInstanceIndex& attachmentGraphInstance, int attachmentResourceIndex, uint32_t firstMip, uint32_t mipCount, uint32_t firstArrayLayer, uint32_t arrayLayerCount, ImageViewAspectMask mask, ImageLayout desiredLayout)
{
	AttachmentGraphInstance* graphInstance = attachmentGraphsInstances.Get(attachmentGraphInstance);

	if (!graphInstance)
	{
		return -1;
	}

	RHIDevice* rhiDevice = GetDeviceHandle(graphInstance->deviceIndex);

	AttachmentResourceInstance* resource = &graphInstance->resources[attachmentResourceIndex];

	int imageCount = resource->imageCount;

	int sampleCount = RENDER_MAX(findMSB(resource->sampHi), 1);

	int texViewIndex = -1;

	if (!textureViewsResourceHandles.DoIHaveNFreeElements(sampleCount * imageCount))
	{
		return texViewIndex;
	}

	for (int currSampleCount = 0; currSampleCount < sampleCount; currSampleCount++)
	{
		for (int i = 0; i < imageCount; i++)
		{
			TextureIndex textureHandle = resource->textureIds[currSampleCount][i];

			texViewIndex = CreateAttachmentImageView(textureHandle, firstMip, mipCount, firstArrayLayer, arrayLayerCount, mask, desiredLayout);
		}
	}

	return texViewIndex;
}

int RenderInstance::CreateAttachmentResources(
	AttachmentGraphInstanceIndex& graphIndex, int renderPassIndex, int imageCount,
	TextureIndex* backBufferTexturesIds, uint32_t width, uint32_t height, 
	RenderPassType rpType, AttachmentClear* clears,
	DeviceSlabAllocator* rtvAllocator, DeviceSlabAllocator* dsvAllocator, ImageMemoryIndex rtvPoolIndex, ImageMemoryIndex dsvPoolIndex)
{
	AttachmentGraphInstance* graphInstance = attachmentGraphsInstances.Get(graphIndex);

	if (!graphInstance)
	{
		return -1;
	}

	RHIDevice* rhiDevice = GetDeviceHandle(graphInstance->deviceIndex);

	VKDevice* dev = rhiDevice->device;

	AttachmentRenderPassInstance* currentRenderPass = &graphInstance->passes[renderPassIndex];

	int attachmentCount = currentRenderPass->attachInstCount;

	EntryHandle* attachmentViews = (EntryHandle*)cacheAllocator->Allocate(sizeof(EntryHandle) * attachmentCount);

	AttachmentInstance* attachInsts = currentRenderPass->attachInst;

	currentRenderPass->rpType = rpType;

	int success = 0;

	for (int b = 0; b < attachmentCount && !success; b++)
	{
		AttachmentInstance* attachDesc = &attachInsts[b];

		int resourceIndex = attachDesc->attachmentResource;

		AttachmentResourceInstance* resourceInst = &graphInstance->resources[resourceIndex];
		
		AttachmentResource* resourceTempl = &graphInstance->graphLayout->resources[resourceIndex];

		int sampHi = resourceInst->sampHi;
		int sampLo = resourceInst->sampLo;

		int sampleCount = RENDER_MAX(findMSB(sampHi), 1);

		int imageWidth = width;
		int imageHeight = height;

		if (clears)
		{
			attachDesc->clear = clears[resourceIndex];
		}

		if (resourceTempl->viewType == AttachmentViewType::SWAPCHAIN)
		{
			for (int i = 0; i < imageCount; i++)
			{
				resourceInst->textureIds[0][i] = backBufferTexturesIds[i];
			}
		}
		else
		{
			resourceInst->imageCount = imageCount;

			ImageUsageFlags usage = resourceInst->usage;

			ImageMemoryIndex poolIndex = usage & (ImageUsageFlagBits::COLOR_ATTACHMENT | ImageUsageFlagBits::RESOLVE_ATTACHMENT) ? rtvPoolIndex : dsvPoolIndex;

			DeviceSlabAllocator* allocator = usage & (ImageUsageFlagBits::COLOR_ATTACHMENT | ImageUsageFlagBits::RESOLVE_ATTACHMENT) ? rtvAllocator : dsvAllocator;

			ImageViewAspectMask mask = 0;

			ImageLayout imageViewLayout = ImageLayout::UNDEFINED;

			if (usage & (ImageUsageFlagBits::DEPTH_ATTACHMENT | ImageUsageFlagBits::STENCIL_ATTACHMENT))
			{
				if (usage & ImageUsageFlagBits::DEPTH_ATTACHMENT)
				{
					mask |= DEPTH_IMAGE_ASPECT;
					imageViewLayout = ImageLayout::DEPTHATTACHMENT;
				}

				if (usage & ImageUsageFlagBits::STENCIL_ATTACHMENT)
				{
					mask |= STENCIL_IMAGE_ASPECT;
					imageViewLayout = ImageLayout::STENCILATTACHMENT;
				}

				if ((usage & (ImageUsageFlagBits::DEPTH_ATTACHMENT | ImageUsageFlagBits::STENCIL_ATTACHMENT)) == (ImageUsageFlagBits::DEPTH_ATTACHMENT | ImageUsageFlagBits::STENCIL_ATTACHMENT))
				{
					imageViewLayout = ImageLayout::DEPTHSTENCILATTACHMENT;
				}
			}
			else
			{
				mask = COLOR_IMAGE_ASPECT;
			}

			for (int v = 0; v < sampleCount; v++)
			{
				for (int g = 0; g < imageCount; g++)
				{
					TextureIndex textureIndex = resourceInst->textureIds[v][g] = CreateAttachmentImage(imageWidth, imageHeight, 1, 1,
						ImageType::IMAGE_2D, sampLo, resourceTempl->format,
						usage, allocator, ImageLayout::UNDEFINED, graphInstance->deviceIndex, poolIndex, MANAGED_IMAGE_RESOURCE);

					int viewSuccess = CreateAttachmentImageView(textureIndex, 0, 1, 0, 1, mask, imageViewLayout);

					if (TextureIndex() == textureIndex || viewSuccess < 0)
					{
						success = -1;
						break;
					}
				}

				sampLo <<= 1;
			}
		}
	}

	if (!success)
	{
		for (int sampleCount = 0; sampleCount < currentRenderPass->maxSampleCount; sampleCount++)
		{
			DriverRenderTargetIndex rtIndex = currentRenderPass->baseRenderTarget[sampleCount];

			OldStyleRenderPassIndex rpIndex = currentRenderPass->baseRenderPass[sampleCount];

			if (DriverRenderTargetIndex() == rtIndex)
			{
				rtIndex = mainRenderTargets.Allocate();
				currentRenderPass->baseRenderTarget[sampleCount] = rtIndex;
			}

			RenderTargetInfo* rtInfo = mainRenderTargets.Get(rtIndex);

			RenderOldStyleVulkanRenderPassInfo* rpInfo = renderPasses.Get(rpIndex);

			if (EntryHandle() != rtInfo->driverRenderTargetInfo)
			{
				dev->DestroyRenderTarget(rtInfo->driverRenderTargetInfo);
			}

			rtInfo->driverRenderTargetInfo = dev->CreateRenderTarget(rpInfo->renderPassHandle, imageCount, width, height, 0, 0);

			rtInfo->deviceIndex = graphInstance->deviceIndex;

			RenderTarget* renderTarget = dev->GetRenderTarget(rtInfo->driverRenderTargetInfo);

			for (int d = 0; d < imageCount; d++)
			{
				for (int e = 0; e < attachmentCount; e++)
				{
					AttachmentInstance* attachInst = &attachInsts[e];

					AttachmentResourceInstance* resourceInst = &graphInstance->resources[attachInst->attachmentResource];

					int sampleIndex = sampleCount;

					if (resourceInst->sampHi == 1)
					{
						sampleIndex = 0;
					}

					TextureIndex textureIndex = resourceInst->textureIds[sampleIndex][d];

					RenderTextureDescription* texDesc = textureResourceHandles.Get(textureIndex);

					RenderImageViewDescription* imageViewDesc = textureViewsResourceHandles.Get(texDesc->viewIndex[0]);

					attachmentViews[e] = imageViewDesc->viewIndex;
				}

				renderTarget->framebufferIndices[d] =
					dev->CreateFrameBuffer(
						attachmentViews,
						attachmentCount,
						rpInfo->renderPassHandle,
						{ width, height }
					);
			}
		}
	}
	
	if (success)
	{
		DestroyAttachmentGraphInstance(graphIndex);
	}
	
	return success;
}

int RenderInstance::CreateDriverSwapChainData(RHIDevice* rhiDevice, EntryHandle swapChainIndex, uint32_t width, uint32_t height, bool recreate)
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

	GetLastDeviceDriverError(rhiDevice, STRING_VIEW_FROM_LITERAL("CreateDriverSwapChainData: Failed"));

	return -1;
}

int RenderInstance::CreateShaderResourceMap(RHIDevice* device, ShaderGraph* graph)
{
	VKDevice* dev = device->device;

	int resourceSetCount = graph->resourceSetCount;

	int descriptorLayoutIndex = shaderResourceTemplates.DoIHaveNFreeElements(resourceSetCount);

	if (!descriptorLayoutIndex)
	{
		return -1;
	}

	DescriptorSetLayoutBuilder** descriptorBuilders = (DescriptorSetLayoutBuilder**)cacheAllocator->Allocate(sizeof(DescriptorSetLayoutBuilder*) * resourceSetCount);

	for (int j = 0; j < resourceSetCount; j++)
	{
		ShaderResourceSetTemplate* set = &graph->shaderResourceSetTemplates[j];
		descriptorBuilders[j] = dev->CreateDescriptorSetLayoutBuilder(set->bindingCount);
	}

	for (int j = 0; j < graph->resourceCount; j++)
	{
		ShaderResourceTemplate* resource = &graph->shaderResources[j];

		VkShaderStageFlags stageFlags = API::ConvertShaderStageToVulkanShaderStage(resource->stages);

		if (resource->type == ShaderResourceType::CONSTANT_BUFFER) 
		{
			continue;
		}
			
		DescriptorSetLayoutBuilder* descriptorBuilder = descriptorBuilders[resource->set];

		int arrayCount = resource->arrayCount;

		VkDescriptorBindingFlags bindingFlags = 0;

		if (arrayCount & UNBOUNDED_DESCRIPTOR_ARRAY)
		{
			bindingFlags |= VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT;
		}

		static bool useUpdateAfterBind = false;

		arrayCount &= DESCRIPTOR_COUNT_MASK;

		switch (resource->type)
		{
		case ShaderResourceType::UNIFORM_BUFFER:
			descriptorBuilder->AddBufferLayout(resource->binding, stageFlags, arrayCount, bindingFlags);
			break;
		case ShaderResourceType::IMAGESTORE2D:
			descriptorBuilder->AddStorageImageLayout(resource->binding, stageFlags, arrayCount, bindingFlags);
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
			descriptorBuilder->AddImageResourceLayout(resource->binding, stageFlags, arrayCount, bindingFlags);
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
			descriptorBuilder->AddSamplerStateLayout(resource->binding, stageFlags, arrayCount, bindingFlags);
			
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
			descriptorBuilder->AddBindlessCombinedSamplersLayout(resource->binding, stageFlags, arrayCount, bindingFlags);
			break;
		case ShaderResourceType::STORAGE_BUFFER:
			descriptorBuilder->AddStorageBufferLayout(resource->binding, stageFlags, arrayCount, bindingFlags);
			break;
		case ShaderResourceType::BUFFER_VIEW:
			if (resource->action == ShaderResourceAction::SHADERREAD)
				descriptorBuilder->AddUniformBufferViewLayout(resource->binding, stageFlags, arrayCount, bindingFlags);
			else if (resource->action == ShaderResourceAction::SHADERWRITE)
				descriptorBuilder->AddStorageBufferViewLayout(resource->binding, stageFlags, arrayCount, bindingFlags);
			break;
		}
	}

	int success = 0;

	for (int j = 0; j < resourceSetCount; j++)
	{
		ShaderResourceSetTemplate* set = &graph->shaderResourceSetTemplates[j];

		ShaderResourceTemplateInstanceIndex currDescriptorLayout = shaderResourceTemplates.Allocate();

		RenderShaderResourceTemplateInfo* info = shaderResourceTemplates.Get(currDescriptorLayout);

		EntryHandle descHandle = dev->CreateDescriptorSetLayout(descriptorBuilders[j]);

		if (EntryHandle() == descHandle)
		{
			GetLastDeviceDriverError(device, STRING_VIEW_FROM_LITERAL("CreateShaderResourceMap: layout creation failed"));

			success = -1;

			break;
		}
		
		info->resourceTemplateInstanceHandle = descHandle;

		info->deviceIndex = graph->deviceIndex;

		set->vulkanDescLayout = currDescriptorLayout;
	}

	return success;
}


RenderShaderGraphIndex RenderInstance::CreateShaderGraphInstance(RenderDeviceIndex deviceSelection, StringView shaderGraphLayout)
{
	RHIDevice* rhiDevice = GetDeviceHandle(deviceSelection);

	VKDevice* dev = rhiDevice->device;

	int totalDetailSize = 0;

	ShaderDetails cachedDetails[5];

	int success = 0;

	int createdShaderGraph = 0;

	int detailsSize = 0;

	RenderShaderGraphIndex shaderGraphIndex = shaderGraphs.shaderGraphPtrs.Allocate();

	if (RenderShaderGraphIndex() == shaderGraphIndex)
	{
		return shaderGraphIndex;
	}

	ShaderGraph* graph = shaderGraphs.shaderGraphPtrs.Get(shaderGraphIndex);

	CleanInitializeShaderGraph(graph);

	int sgCode = CreateShaderGraph
	(
		shaderGraphLayout,
		cacheAllocator,
		graph,
		cachedDetails,
		&detailsSize, 
		internalRendererLogger
	);

	success = -1;

	if (!sgCode)
	{
		graph->deviceIndex = deviceSelection;

		sgCode = CreateShaderResourceMap(rhiDevice, graph);

		if (!sgCode)
		{
			success = 0;

			totalDetailSize += detailsSize;

			for (int i = 0; i < detailsSize; i++)
			{
				ShaderMap* map = &graph->shaderMaps[i];

				int detailsIndex = shaderGraphs.shaderDetails.Allocate();

				map->shaderReference = detailsIndex;

				memcpy(shaderGraphs.shaderDetails.Get(detailsIndex), &cachedDetails[i], sizeof(ShaderDetails));
			}

			int mapCount = graph->shaderMapCount;

			for (int j = 0; j < mapCount; j++)
			{
				ShaderMap* map = &graph->shaderMaps[j];

				ShaderDetails* details = shaderGraphs.shaderDetails.Get(map->shaderReference);

				char* str = details->glslShaderName;

				int shaderNameLength = details->glslShaderNameSize;

				char* shaderData;

				OSFileHandle handle;

				int shaderLength = 0;

				char* stringBuffer = (char*)cacheAllocator->Allocate(shaderNameLength + 4);

				memcpy(stringBuffer, str, shaderNameLength);

				memcpy(stringBuffer + (shaderNameLength - 1), ".spv", 5);

				StringView nameView{ stringBuffer, shaderNameLength + 3 };

				VkShaderStageFlags shaderFlags = API::ConvertShaderStageToVulkanShaderStage(map->type);

				int64_t fileRet = 0;

				if (OSFileExist(nameView.stringData, nameView.charCount, OSFileFlagsTypes::READ))
				{
					nameView.charCount -= 4;
				}

				fileRet = OSOpenFile(nameView.stringData, nameView.charCount, READ, &handle);

				if (!fileRet)
				{
					shaderLength = handle.fileLength;

					shaderData = (char*)cacheAllocator->CAllocate(shaderLength + 1);

					fileRet = OSReadFile(&handle, shaderLength, shaderData);

					if (fileRet > 0)
					{
						if (shaderData[shaderLength - 1] != '\0')
						{
							shaderData[shaderLength] = '\0';
							shaderLength++;
						}

						OSCloseFile(&handle);

						EntryHandle shaderHandle = details->shaderHandle = dev->CreateShader(shaderData, shaderLength, shaderFlags);

						if (shaderHandle != EntryHandle())
						{
							continue;
						}

						GetLastDeviceDriverError(rhiDevice, STRING_VIEW_FROM_LITERAL("CreateShaderGraph : Shader Creation Failed"));

						success = -1;
					}
				}

				if (!success)
				{
					internalRendererLogger->AddLogMessage(LOGERROR, STRING_VIEW_FROM_LITERAL("CreateShaderGraph : File Handling"));
					internalRendererLogger->AddLogMessage(LOGERROR, nameView);
				}

				success = -1;
				break;
			}
		}
	}	

	if (success)
	{
		DestroyShaderGraph(shaderGraphIndex);	
	}

	return shaderGraphIndex;
}

GeneratedPipelineInstanceIndex RenderInstance::CreateGraphicRenderStateObject(RenderShaderGraphIndex& shaderGraphIndex, GenericRenderPipelineInfoIndex& pipelineDescriptionIndex, AttachmentGraphInstanceIndex* frameGraphAttachments, int* perFrameRenderPassSelection, int frameGraphCount)
{
	GeneratedPipelineInstanceIndex pipelineInstIndex{};

	ShaderGraph* graph = shaderGraphs.shaderGraphPtrs.Get(shaderGraphIndex);

	if (!graph)
	{
		return pipelineInstIndex;
	}

	ShaderMap* map = &graph->shaderMaps[0];

	if (map->type != COMPUTESHADERSTAGE)
	{
		uint32_t pipelineVariationsCounter = 0;

		uint32_t totalPiplineVariations = 0;

		int success = 0;

		pipelineInstIndex = graphPipelineDescriptions.Allocate();

		if (GeneratedPipelineInstanceIndex() != pipelineInstIndex)
		{
			GraphPipelineDescription* desc = graphPipelineDescriptions.Get(pipelineInstIndex);

			CleanInitializeGraphPipeline(desc);

			PipelineInstanceData* instData = &desc->instanceData;

			instData->frameGraphCount = frameGraphCount;

			for (int i = 0; i < frameGraphCount; i++)
			{
				totalPiplineVariations += attachmentGraphsInstances[frameGraphAttachments[i]].passes[perFrameRenderPassSelection[i]].maxSampleCount;
				instData->frameGraphIndices[i] = frameGraphAttachments[i];
				instData->frameGraphRenderPasses[i] = perFrameRenderPassSelection[i];
			}

			EntryHandle* pipelineHandles = &desc->pipelineIndices[0];

			instData->pipelineCount = 0;

			desc->instanceData.deviceIndex = graph->deviceIndex;

			GenericPipelineStateInfo* info = pipelineInfos.Get(pipelineDescriptionIndex);

			for (int i = 0; i < frameGraphCount; i++)
			{
				instData->frameGraphPipelineIndices[i] = pipelineVariationsCounter;

				int count = CreatePipelineFromGraphAndSpec(
					graph->deviceIndex,
					info,
					graph,
					pipelineHandles, pipelineVariationsCounter,
					attachmentGraphsInstances.Get(frameGraphAttachments[i]), perFrameRenderPassSelection[i]
				);

				if (count <= 0)
				{
					success = -1;
					break;
				}

				pipelineVariationsCounter += count;

				instData->pipelineCount += count;
			}

			if (success)
			{
				DestroyGraphPipelineDescription(pipelineInstIndex);

				return {};
			}
		}
	}
	else
	{
		internalRendererLogger->AddLogMessage(LOGERROR, STRING_VIEW_FROM_LITERAL("CreateGraphicRenderStateObject : Passed Compute Shader Graph"));
	}

	return pipelineInstIndex;
}

GeneratedPipelineInstanceIndex RenderInstance::CreateComputePipelineStateObject(RenderShaderGraphIndex& shaderGraphIndex)
{
	GeneratedPipelineInstanceIndex pipelineInstIndex;

	ShaderGraph* graph = shaderGraphs.shaderGraphPtrs.Get(shaderGraphIndex);

	if (!graph)
	{
		return pipelineInstIndex;
	}
	
	ShaderMap* map = &graph->shaderMaps[0];
	
	if (map->type == COMPUTESHADERSTAGE)
	{
		pipelineInstIndex = graphPipelineDescriptions.Allocate();

		if (GeneratedPipelineInstanceIndex() != pipelineInstIndex)
		{
			GraphPipelineDescription* desc = graphPipelineDescriptions.Get(pipelineInstIndex);

			CleanInitializeGraphPipeline(desc);

			EntryHandle pipelineID = CreateVulkanComputePipelineTemplate(graph);

			if (EntryHandle() == pipelineID)
			{
				DestroyGraphPipelineDescription(pipelineInstIndex);
				return {};
			}

			desc->pipelineIndices[0] = pipelineID;

			desc->instanceData.pipelineCount = 1;

			desc->instanceData.deviceIndex = graph->deviceIndex;
		}
	}
	else
	{
		internalRendererLogger->AddLogMessage(LOGERROR, STRING_VIEW_FROM_LITERAL("CreateComputeStateObject : Passed Render Shader Graph"));
	}

	return pipelineInstIndex;
}

GenericRenderPipelineInfoIndex RenderInstance::CreateGenericRenderPipelineDescription(StringView pipelineDescriptionFileName)
{
	GenericRenderPipelineInfoIndex infoIndex;

	infoIndex = pipelineInfos.Allocate();

	if (GenericRenderPipelineInfoIndex() != infoIndex)
	{
		GenericPipelineStateInfo* stateInfo = pipelineInfos.Get(infoIndex);

		CleanInitializePipelineInfo(stateInfo);

		int fileRet = CreatePipelineDescription(pipelineDescriptionFileName, stateInfo, cacheAllocator, internalRendererLogger);

		if (fileRet)
		{
			DestroyPipelineInfo(infoIndex);
			return GenericRenderPipelineInfoIndex();
		}
	}
	
	return infoIndex;
}

int RenderInstance::CreatePipelineFromGraphAndSpec(RenderDeviceIndex deviceSelection, GenericPipelineStateInfo* stateInfo, ShaderGraph* graph, EntryHandle* outHandles, uint32_t outHandlePointer, AttachmentGraphInstance* graphInstance, uint32_t graphRenderPassIndex)
{
	RHIDevice* rhiDevice = GetDeviceHandle(deviceSelection);

	VKDevice* dev = rhiDevice->device;

	EntryHandle* layoutHandles = (EntryHandle*)cacheAllocator->Allocate(sizeof(EntryHandle) * graph->resourceSetCount);
	EntryHandle* shaderHandle = (EntryHandle*)cacheAllocator->Allocate(sizeof(EntryHandle) * graph->shaderMapCount);

	for (int i = 0; i < graph->shaderMapCount; i++)
	{
		ShaderMap* map = &graph->shaderMaps[i];
		shaderHandle[i] = shaderGraphs.shaderDetails.Get(map->shaderReference)->shaderHandle;
	}

	int pushConstantRangeCount = 0;

	for (int i = 0; i < graph->resourceSetCount; i++)
	{
		ShaderResourceSetTemplate* resourceSet = &graph->shaderResourceSetTemplates[i];
		layoutHandles[i] = shaderResourceTemplates[resourceSet->vulkanDescLayout].resourceTemplateInstanceHandle;
		pushConstantRangeCount += resourceSet->constantStageCount;
	}

	uint32_t* pushConstantsSizes = (uint32_t*)cacheAllocator->CAllocate(sizeof(uint32_t) * pushConstantRangeCount);
	VkShaderStageFlags* shaderStages = (VkShaderStageFlags*)cacheAllocator->CAllocate(sizeof(VkShaderStageFlags) * pushConstantRangeCount);

	for (int i = 0; i < graph->resourceCount; i++)
	{
		ShaderResourceTemplate* resource = &graph->shaderResources[i];
		if (resource->type == ShaderResourceType::CONSTANT_BUFFER)
		{
			int rangeIndex = resource->rangeIndex;

			pushConstantsSizes[rangeIndex] += resource->size;
			
			shaderStages[rangeIndex] |= API::ConvertShaderStageToVulkanShaderStage(resource->stages);
		}
	}

	const uint32_t dynamicStateCount = 2;

	VKGraphicsPipelineBuilder* pipelineBuilder = dev->CreateGraphicsPipelineBuilder(EntryHandle(), stateInfo->blendAttachmentCount, graph->resourceSetCount, dynamicStateCount, pushConstantRangeCount);

	uint32_t globalPushOffset = 0;

	for (int i = 0; i < pushConstantRangeCount; i++)
	{
		pipelineBuilder->AddPushConstantRange(globalPushOffset, pushConstantsSizes[i], shaderStages[i], i);
		globalPushOffset += pushConstantsSizes[i];
	}
	
	VkDynamicState dynamicStates[dynamicStateCount] = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};

	pipelineBuilder->CreateDynamicStateInfo(dynamicStates, 2);

	VkVertexInputBindingDescription* bindingDescriptions = (VkVertexInputBindingDescription*)cacheAllocator->Allocate(sizeof(VkVertexInputBindingDescription) * (stateInfo->vertexBufferDescCount));

	int descCount = 0;

	for (int i = 0; i < stateInfo->vertexBufferDescCount; i++)
	{
		bindingDescriptions[i] =  VK::Utils::CreateVertexInputBindingDescription(i, stateInfo->vertexBufferDesc[i].perInputSize);

		descCount += stateInfo->vertexBufferDesc[i].descCount;
	}

	VkVertexInputAttributeDescription* vertexBufferInput = (VkVertexInputAttributeDescription*)cacheAllocator->Allocate(sizeof(VkVertexInputAttributeDescription) * (descCount));

	int vertexBufferIter = 0;
	
	for (int i = 0; i < stateInfo->vertexBufferDescCount; i++)
	{
		API::ConvertVertexInputToVKVertexAttrDescription(stateInfo->vertexBufferDesc[i].descriptions, stateInfo->vertexBufferDesc[i].descCount, i, &vertexBufferInput[vertexBufferIter]);

		vertexBufferIter += stateInfo->vertexBufferDesc[i].descCount;
	}

	pipelineBuilder->CreateVertexInput(bindingDescriptions, stateInfo->vertexBufferDescCount, vertexBufferInput, descCount);
	
	pipelineBuilder->CreateInputAssembly(API::ConvertTopology(stateInfo->primType), false);

	pipelineBuilder->CreateViewportState(1, 1);

	pipelineBuilder->CreateRasterizer(API::ConvertCullMode(stateInfo->cullMode), API::ConvertTriangleWinding(stateInfo->windingOrder), stateInfo->lineWidth);

	for (int i = 0; i < stateInfo->blendAttachmentCount; i++)
	{
		BlendAttachments* attach = &stateInfo->blendAttachments[i];

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

	pipelineBuilder->CreateColorBlending(stateInfo->blendEnable, API::ConvertBlendLogicOpToVulkanLogicOp(stateInfo->blendOp));

	VkStencilOpState frontState = API::ConvertFaceStencilDataToVulkan(stateInfo->frontFace);
	VkStencilOpState backState = API::ConvertFaceStencilDataToVulkan(stateInfo->backFace);

	pipelineBuilder->CreateDepthStencil(API::ConvertCompareOpToVulkanCompareOp(stateInfo->depthTest), stateInfo->depthEnable, stateInfo->depthWrite, stateInfo->StencilEnable, &frontState, &backState);

	AttachmentRenderPassInstance* rendPassInst = &graphInstance->passes[graphRenderPassIndex];

	uint32_t sampleCount = (uint32_t)rendPassInst->maxSampleCount;

	uint32_t lowSample = (sampleCount > 1) ? 1 : 0;
	
	int pipelinesCreated = 0;

	for (; pipelinesCreated >= 0 && pipelinesCreated < sampleCount; pipelinesCreated++)
	{
		int msaaLevel = (1 << (lowSample + pipelinesCreated));
		if (msaaLevel > stateInfo->sampleCountHigh) break;

		pipelineBuilder->CreateMultiSampling((VkSampleCountFlagBits)msaaLevel);
		pipelineBuilder->renderPass = dev->GetRenderPass(renderPasses[graphInstance->passes[graphRenderPassIndex].baseRenderPass[pipelinesCreated]].renderPassHandle);

		 EntryHandle handle = pipelineBuilder->CreateGraphicsPipeline(layoutHandles, graph->resourceSetCount, shaderHandle, graph->shaderMapCount);

		 if (EntryHandle() == handle)
		 {
			 GetLastDeviceDriverError(rhiDevice, STRING_VIEW_FROM_LITERAL("CreatePipelinesFromGraphAndSpec : CreatedGraphicsPipeline failed"));

			 pipelinesCreated = -1;

			 continue;
		 }

		 outHandles[outHandlePointer + pipelinesCreated] = handle;
	}

	return pipelinesCreated;
}

EntryHandle RenderInstance::CreateVulkanComputePipelineTemplate(ShaderGraph* graph)
{
	RHIDevice* rhiDevice = GetDeviceHandle(graph->deviceIndex);

	VKDevice* dev = rhiDevice->device;

	EntryHandle* layoutHandles = (EntryHandle*)cacheAllocator->Allocate(sizeof(EntryHandle) * (graph->resourceSetCount));

	ShaderMap* map = &graph->shaderMaps[0];

	EntryHandle shaderHandle = shaderGraphs.shaderDetails.Get(map->shaderReference)->shaderHandle;

	int pushRangeSize = 0;

	for (int a = 0; a < graph->resourceSetCount; a++)
	{
		ShaderResourceSetTemplate* setLayout = &graph->shaderResourceSetTemplates[a];

		pushRangeSize += setLayout->constantStageCount;
	}

	int* pushConstantSize = (int*)cacheAllocator->CAllocate(sizeof(int) * pushRangeSize);

	for (int g = 0; g < graph->resourceCount; g++)
	{
		ShaderResourceTemplate* resource = &graph->shaderResources[g];

		if (resource->type == ShaderResourceType::CONSTANT_BUFFER)
		{
			int pushRangeIndex = resource->rangeIndex;

			pushConstantSize[pushRangeIndex] += resource->size;
		}
	}

	int descriptorCount = graph->resourceSetCount;

	VKComputePipelineBuilder* pipelineBuilder = dev->CreateComputePipelineBuilder(descriptorCount, pushRangeSize);

	uint32_t globalOffset = 0;

	for (int g = 0; g < pushRangeSize; g++)
	{
		pipelineBuilder->AddPushConstantRange(globalOffset, pushConstantSize[g], VK_SHADER_STAGE_COMPUTE_BIT, g);

		globalOffset += pushConstantSize[g];
	}
	
	for (int i = 0; i < descriptorCount; i++)
	{
		ShaderResourceSetTemplate* resourceSet = &graph->shaderResourceSetTemplates[i];

		layoutHandles[i] = shaderResourceTemplates[resourceSet->vulkanDescLayout].resourceTemplateInstanceHandle;
	}

	return pipelineBuilder->CreateComputePipeline(layoutHandles, descriptorCount, shaderHandle);
}

void RenderInstance::UploadHostTransfers(CommandRecorder* recorder)
{
	int memCount = driverHostMemoryUpdater.linkCount;

	if (!memCount) return;

	VKDevice* dev = recorder->device->device;

	BufferMemoryTransferRegion region;
	int link = driverHostMemoryUpdater.linkHead;
	int* linkprev = &driverHostMemoryUpdater.linkHead;
	
	EntryHandle previousBuffer = EntryHandle();
	size_t previousMin = 0;
	size_t previousMax = 0;
	size_t batchCounter = 0;

	void** batchAddresses = (void**)cacheAllocator->Allocate(sizeof(void*) * memCount);
	size_t* batchSizes = (size_t*)cacheAllocator->Allocate(sizeof(size_t) * memCount);
	size_t* batchOffsets = (size_t*)cacheAllocator->Allocate(sizeof(size_t) * memCount);

	while (link >= 0)
	{
		link = driverHostMemoryUpdater.PopLink(&region, link, &linkprev);

		size_t intSize = region.size;

		size_t rsize = 0, intOffset = 0;

		RenderAllocation* alloc = allocations.Get(region.allocationIndex);

		GetAllocationDetails(alloc, &rsize, &intOffset, nullptr, currentFrame);

		intOffset += region.allocoffset;
		
		EntryHandle index = bufferHandles[alloc->memIndex].bufferHandle;

		void* data = region.data;

		if (index != previousBuffer)
		{
			if (EntryHandle() != previousBuffer)
			{
				dev->WriteToHostBufferBatch(previousBuffer, batchAddresses, batchSizes, batchOffsets, previousMax - previousMin, previousMin, batchCounter);
			}

			previousBuffer = index;
			batchCounter = 0;
			previousMin = 0;
			previousMax = 0;
		}

		batchAddresses[batchCounter] = data;
		batchOffsets[batchCounter] = intOffset;
		batchSizes[batchCounter] = intSize;

		batchCounter++;

		previousMin = RENDER_MIN(intOffset, previousMin);
		previousMax = RENDER_MAX(intOffset + intSize, previousMax);
	}

	dev->WriteToHostBufferBatch(previousBuffer, batchAddresses, batchSizes, batchOffsets, previousMax - previousMin, previousMin, batchCounter);
}

void RenderInstance::UploadDescriptorsUpdates(CommandRecorder* recorder)
{
	int memCount = descriptorUpdatePool.linkCount;

	if (!memCount) return;

	VKDevice* dev = recorder->device->device;

	int link = descriptorUpdatePool.linkHead;
	int* linkprev = &descriptorUpdatePool.linkHead;
	ShaderResourceUpdate region;

	EntryHandle previousBuffer = EntryHandle();

	DescriptorSetBuilder* builder = nullptr;

	ShaderResourceSet* set = nullptr;

	while (link >= 0)
	{
		link = descriptorUpdatePool.PopLink(&region, link, &linkprev);

		ShaderResourceManager* manager = descriptorManagers.Get(region.descriptorManagerIndex);

		EntryHandle index = manager->descriptorSetHandles[region.descriptorSet];

		void* data = region.data;

		if (index != previousBuffer)
		{
			builder = dev->UpdateDescriptorSet(index);
			
			previousBuffer = index;

			set = manager->descriptorSets[region.descriptorSet];
		}

		switch (region.type)
		{
		case ShaderResourceType::SAMPLERSTATE:
		{
			DeviceHandleArrayUpdate* update = (DeviceHandleArrayUpdate*)region.data;

			ShaderResourceSampler* samplerHeader = (ShaderResourceSampler*)&set->resourceBindings[region.bindingIndex].resourceArray.samplers;

			SamplerIndex* samplerHandlesFromUpdate = (SamplerIndex*)update->resourceHandles;

			for (int iter = 0; iter < update->resourceCount; iter++)
			{
				EntryHandle handle = samplerResourceHandles[samplerHandlesFromUpdate[iter]];

				if (EntryHandle() == handle)
				{
					internalRendererLogger->AddLogMessage(LOGERROR, STRING_VIEW_FROM_LITERAL("Sampler State Update : Invalid sampler handle"));
					continue;
				}

				builder->AddSamplerDescription(handle, update->resourceDstBegin + iter, region.bindingIndex, currentFrame, 1);

				if (region.copyCount == (MAX_FRAMES_IN_FLIGHT))
				{
					samplerHeader->samplerHandles[iter + update->resourceDstBegin] = samplerHandlesFromUpdate[iter];
					samplerHeader->samplerCount = RENDER_MAX(samplerHeader->samplerCount, (iter + update->resourceDstBegin) + 1);
				}
			}
			break;
		}
		case ShaderResourceType::IMAGE2D:
		{
			DeviceHandleArrayUpdate* update = (DeviceHandleArrayUpdate*)region.data;

			ShaderResourceImage* imageResource = &set->resourceBindings[region.bindingIndex].resourceArray.images;

			DeviceHandleArrayUpdateTextureView* texViews = (DeviceHandleArrayUpdateTextureView*)update->resourceHandles;

			for (int iter = 0; iter < update->resourceCount; iter++)
			{
				RenderTextureDescription* desc = textureResourceHandles.Get(texViews[iter].imageHandle);

				if (!desc) 
				{
					internalRendererLogger->AddLogMessage(LOGERROR, STRING_VIEW_FROM_LITERAL("Image 2D Update : Invalid image handler"));
					continue;
				}

				RenderImageViewDescription* imageViewDesc = textureViewsResourceHandles.Get(desc->viewIndex[texViews[iter].viewIndex]);

				if (!imageViewDesc) 
				{
					internalRendererLogger->AddLogMessage(LOGERROR, STRING_VIEW_FROM_LITERAL("Image 2D Update : Invalid image view handler"));
					continue;
				}

				builder->AddImageResourceDescription(imageViewDesc->viewIndex, API::ConvertImageLayoutToVulkanImageLayout(imageViewDesc->desiredLayoutForView), update->resourceDstBegin + iter, region.bindingIndex, currentFrame, 1);

				if (region.copyCount == (MAX_FRAMES_IN_FLIGHT))
				{
					imageResource->textureDetails[iter + update->resourceDstBegin].textureHandle = texViews[iter].imageHandle;
					imageResource->textureDetails[iter + update->resourceDstBegin].viewIndex = texViews[iter].viewIndex;
					imageResource->textureCount = RENDER_MAX(imageResource->textureCount, (iter + update->resourceDstBegin) + 1);
				}
			}
			break;
		}
		case ShaderResourceType::SAMPLER3D:
		case ShaderResourceType::SAMPLER2D:
		case ShaderResourceType::SAMPLERCUBE:
		{
			DeviceHandleArrayUpdate* update = (DeviceHandleArrayUpdate*)region.data;

			ShaderResourceCombinedImage* imageResource = &set->resourceBindings[region.bindingIndex].resourceArray.combinedImages;

			DeviceHandleArrayUpdateTextureViewSampler* texViews = (DeviceHandleArrayUpdateTextureViewSampler*)update->resourceHandles;

			for (int iter = 0; iter < update->resourceCount; iter++)
			{
				RenderTextureDescription* desc = textureResourceHandles.Get(texViews[iter].imageHandle);

				if (!desc)
				{
					internalRendererLogger->AddLogMessage(LOGERROR, STRING_VIEW_FROM_LITERAL("Combined Sampler Update : Invalid image handler"));
					continue;
				}

				RenderImageViewDescription* imageViewDesc = textureViewsResourceHandles.Get(desc->viewIndex[texViews[iter].viewIndex]);

				if (!imageViewDesc)
				{
					internalRendererLogger->AddLogMessage(LOGERROR, STRING_VIEW_FROM_LITERAL("Combined Sampler Update: Invalid image view handler"));
					continue;
				}

				EntryHandle* samplerHandle = samplerResourceHandles.Get(texViews[iter].samplerHandle);

				if (!samplerHandle)
				{
					internalRendererLogger->AddLogMessage(LOGERROR, STRING_VIEW_FROM_LITERAL("Combined Sampler Update : Invalid sampler handle"));
					continue;
				}

				builder->AddCombinedTextureArray(imageViewDesc->viewIndex, *samplerHandle, API::ConvertImageLayoutToVulkanImageLayout(imageViewDesc->desiredLayoutForView), update->resourceDstBegin + iter, region.bindingIndex, currentFrame, 1);

				if (region.copyCount == (MAX_FRAMES_IN_FLIGHT))
				{
					imageResource->textureDetails[iter + update->resourceDstBegin].textureHandle = texViews[iter].imageHandle;
					imageResource->textureDetails[iter + update->resourceDstBegin].viewIndex = texViews[iter].viewIndex;
					imageResource->textureDetails[iter + update->resourceDstBegin].samplerHandle = texViews[iter].samplerHandle;
					imageResource->textureCount = RENDER_MAX(imageResource->textureCount, (iter + update->resourceDstBegin) + 1);
				}
			}
			break;
		}
		case ShaderResourceType::STORAGE_BUFFER:
		{
			BufferArrayUpdate* update = (BufferArrayUpdate*)region.data;

			ShaderResourceBuffer* bufferResource = (ShaderResourceBuffer*)&set->resourceBindings[region.bindingIndex].resourceArray.buffers;
	
			int arrayCount = update->allocationCount;
			int firstBuffer = update->resourceDstBegin;

			for (int j = 0; j < arrayCount; j++)
			{
				RenderAllocation* alloc = allocations.Get(update->allocationIndices[j]);

				if (!alloc) 
				{
					internalRendererLogger->AddLogMessage(LOGERROR, STRING_VIEW_FROM_LITERAL("Storage Update: Invalid Buffer handle"));
					continue;
				}

				if (alloc->allocType == AllocationType::PERFRAME)
					builder->AddStorageBuffer(dev->GetBufferHandle(bufferHandles[alloc->memIndex].bufferHandle), alloc->deviceAllocSize / MAX_FRAMES_IN_FLIGHT, region.bindingIndex, 1, alloc->offset, currentFrame, firstBuffer + j);
				else
					builder->AddStorageBufferDirect(dev->GetBufferHandle(bufferHandles[alloc->memIndex].bufferHandle), alloc->deviceAllocSize, region.bindingIndex, 1, alloc->offset, currentFrame, firstBuffer + j);

				if (region.copyCount == (MAX_FRAMES_IN_FLIGHT))
				{
					bufferResource->allocationIndex[j + update->resourceDstBegin] = update->allocationIndices[j];
					bufferResource->bufferCount = RENDER_MAX(bufferResource->bufferCount, (j + update->resourceDstBegin) + 1);
				}
			}
			break;
		}
		case ShaderResourceType::UNIFORM_BUFFER:
		{
			BufferArrayUpdate* update = (BufferArrayUpdate*)region.data;
			
			ShaderResourceBuffer* bufferResource = (ShaderResourceBuffer*)&set->resourceBindings[region.bindingIndex].resourceArray.buffers;

			int arrayCount = update->allocationCount;
			int firstBuffer = update->resourceDstBegin;

			for (int j = 0; j < arrayCount; j++)
			{
				RenderAllocation* alloc = allocations.Get(update->allocationIndices[j]);

				if (!alloc)
				{
					internalRendererLogger->AddLogMessage(LOGERROR, STRING_VIEW_FROM_LITERAL("Uniform Update: Invalid Buffer handle"));
					continue;
				}

				if (alloc->allocType == AllocationType::PERFRAME)
					builder->AddUniformBuffer(dev->GetBufferHandle(bufferHandles[alloc->memIndex].bufferHandle), alloc->deviceAllocSize / MAX_FRAMES_IN_FLIGHT, region.bindingIndex, 1, alloc->offset, currentFrame, firstBuffer + j);
				else
					builder->AddUniformBufferDirect(dev->GetBufferHandle(bufferHandles[alloc->memIndex].bufferHandle), alloc->deviceAllocSize, region.bindingIndex, 1, alloc->offset, currentFrame, firstBuffer + j);
			
				if (region.copyCount == (MAX_FRAMES_IN_FLIGHT))
				{
					bufferResource->allocationIndex[j + update->resourceDstBegin] = update->allocationIndices[j];
					bufferResource->bufferCount = RENDER_MAX(bufferResource->bufferCount, (j + update->resourceDstBegin) + 1);
				}
			}
			break;
		}
		case ShaderResourceType::BUFFER_VIEW:
		{
			BufferArrayUpdate* update = (BufferArrayUpdate*)region.data;
			
			ShaderResourceBuffer* bufferResource = (ShaderResourceBuffer*)&set->resourceBindings[region.bindingIndex].resourceArray.views;

			int arrayCount = update->allocationCount;
			int firstBuffer = update->resourceDstBegin;

			for (int j = 0; j < arrayCount; j++)
			{
				RenderAllocation* alloc = allocations.Get(update->allocationIndices[j]);

				if (!alloc)
				{
					internalRendererLogger->AddLogMessage(LOGERROR, STRING_VIEW_FROM_LITERAL("Buffer View Update: Invalid Buffer handle"));
					continue;
				}

				int viewGrab = (alloc->allocType == AllocationType::PERFRAME) ? currentFrame : 0;

				VkBufferView handle = dev->GetBufferView(alloc->viewIndex, viewGrab);

				ShaderResourceHeader* resource = (ShaderResourceHeader*)&set->resourceBindings[region.bindingIndex];

				if (resource->action == ShaderResourceAction::SHADERREAD)
				{
					builder->AddUniformBufferView(handle, region.bindingIndex, currentFrame, 1, j + firstBuffer);
				}
				else if (resource->action == ShaderResourceAction::SHADERWRITE)
				{
					builder->AddStorageBufferView(handle, region.bindingIndex, currentFrame, 1, j + firstBuffer);
				}
				
				if (region.copyCount == (MAX_FRAMES_IN_FLIGHT))
				{
					bufferResource->allocationIndex[j + update->resourceDstBegin] = update->allocationIndices[j];
					bufferResource->bufferCount = RENDER_MAX(bufferResource->bufferCount, (j + update->resourceDstBegin) + 1);
				}
			}
			break;
		}
		}
	}
}

void RenderInstance::UploadImageMemoryTransfers(CommandRecorder* recorder)
{
	int memCount = imageMemoryUpdateManager.linkCount;

	if (!memCount) return;

	VKDevice* dev = recorder->device->device;

	int link = imageMemoryUpdateManager.linkHead;

	DeviceSlabAllocator* stagingAlloc = &recorder->device->container.stagingBufferAllocators[currentFrame];

	TextureMemoryRegion* regions = (TextureMemoryRegion*)cacheAllocator->Allocate(sizeof(TextureMemoryRegion) * memCount, alignof(TextureMemoryRegion));

	int regionCount = 0;

	while (link >= 0)
	{
		link = imageMemoryUpdateManager.PopLink(&regions[regionCount++], link);
	}

	for (int i = 0; i < regionCount; i++)
	{
		TextureMemoryRegion* region = &regions[i];

		RenderTextureDescription* desc = textureResourceHandles.Get(region->textureIndex);

		EntryHandle handle = desc->textureIndex;

		ResourceIndex resourceIndex = textureResourceHandles[region->textureIndex].resourceStatusIndex;

		ResourceStatus* resourceStatus = resourceStatuses.Get(resourceIndex);

		PipelineHandleIndex fakeIndex = PipelineHandleIndex();

		TransitionImageLayout(dev, handle, region->mipStart, region->mipLevels,
			desc->mipLayers, region->layerStart, region->layerCount,
			region->transferMask, ImageLayout::TRANSFER_DEST_OPTIMAL,
			resourceStatus, TRANSFER_BARRIER, TRANSFER_WRITE_DATA_RESOURCE, recorder->accumulator, fakeIndex);
	}

	InsertAccumulatedBarriers(recorder);

	for (int i = 0; i < regionCount; i++)
	{
		TextureMemoryRegion* region = &regions[i];

		RenderTextureDescription* desc = textureResourceHandles.Get(region->textureIndex);

		EntryHandle handle = desc->textureIndex;

		size_t currentImageOffsetInUploadArena = stagingAlloc->Allocate(region->totalSize, 16);

		dev->UploadImageData(
			handle,
			(char*)region->data,
			region->totalSize,
			recorder->device->container.stagingBuffers[currentFrame],
			region->width,
			region->height,
			region->mipLevels,
			region->layerCount,
			API::ConvertImageFormatToVulkanFormat(desc->format),
			API::ConvertImageViewAspectMaskToVulkanImageAspectFlags(region->transferMask),
			currentImageOffsetInUploadArena,
			recorder->rbo
		);
	}

	imageMemoryUpdateManager.ddsRegionAlloc = 0;
	imageMemoryUpdateManager.linkHead = -1;
}


void RenderInstance::UploadDeviceLocalTransfers(CommandRecorder* recorder)
{
	int memCount = driverDeviceMemoryUpdater.linkCount;

	if (!memCount) return;

	VKDevice* dev = recorder->device->device;

	BufferMemoryTransferRegion region;
	int link = driverDeviceMemoryUpdater.linkHead;
	int* linkprev = &driverDeviceMemoryUpdater.linkHead;

	size_t* batchSizes = (size_t*)cacheAllocator->Allocate(sizeof(size_t) * (memCount));
	size_t* batchOffsets = (size_t*)cacheAllocator->Allocate(sizeof(size_t) * (memCount));
	void** batchData = (void**)cacheAllocator->Allocate(sizeof(void*) * (memCount));
	size_t* uploadArenaOffset = (size_t*)cacheAllocator->Allocate(sizeof(size_t) * (memCount));

	size_t batchCounter = 0;
	size_t cumulativeSize = 0;

	EntryHandle previousBuffer = EntryHandle();

	DeviceSlabAllocator* stagingAlloc = &recorder->device->container.stagingBufferAllocators[currentFrame];

	while (link >= 0)
	{
		link = driverDeviceMemoryUpdater.PopLink(&region, link, &linkprev);

		size_t rsize = 0, intOffset = 0;

		RenderAllocation* alloc = allocations.Get(region.allocationIndex);

		GetAllocationDetails(alloc, &rsize, &intOffset, nullptr, currentFrame);

		intOffset += region.allocoffset;

		EntryHandle index = bufferHandles[alloc->memIndex].bufferHandle;

		InsertBufferBarrier(dev, region.allocationIndex, StageBits::TRANSFER_BARRIER, BarrierActionBits::TRANSFER_WRITE_DATA_RESOURCE, recorder->accumulator);
	
		if (index != previousBuffer)
		{
			if (previousBuffer != EntryHandle())
			{
				InsertAccumulatedBarriers(recorder);
				cumulativeSize = (uploadArenaOffset[batchCounter - 1] - uploadArenaOffset[0]) + batchSizes[batchCounter - 1];
				dev->WriteToDeviceBufferBatch(previousBuffer, recorder->device->container.stagingBuffers[currentFrame], batchData, batchSizes, batchOffsets, cumulativeSize, uploadArenaOffset, batchCounter, recorder->rbo);
			}

			previousBuffer = index;
			batchCounter = 0;
			cumulativeSize = 0;
		}

		uploadArenaOffset[batchCounter] = stagingAlloc->Allocate(region.size, 64);
		batchSizes[batchCounter] = region.size;
		batchData[batchCounter] = region.data;
		batchOffsets[batchCounter] = intOffset;

		batchCounter++;
	}

	InsertAccumulatedBarriers(recorder);

	cumulativeSize = (uploadArenaOffset[batchCounter - 1] - uploadArenaOffset[0]) + batchSizes[batchCounter - 1];

	dev->WriteToDeviceBufferBatch(previousBuffer, recorder->device->container.stagingBuffers[currentFrame], batchData, batchSizes, batchOffsets, cumulativeSize, uploadArenaOffset, batchCounter, recorder->rbo);
}

void RenderInstance::InvokeTransferCommands(CommandRecorder* recorder)
{
	int memCount = transferCommandPool.linkCount;

	if (!memCount) return;

	VKDevice* dev = recorder->device->device;
	
	TransferCommand region;
	int link = transferCommandPool.linkHead;
	int* linkprev = &transferCommandPool.linkHead;

	while (link >= 0)
	{
		link = transferCommandPool.PopLink(&region, link, &linkprev);

		size_t rsize = 0, intOffset = 0;

		int resourceIndex = 0;

		RenderAllocation* alloc = allocations.Get(region.allocationIndex);

		GetAllocationDetails(alloc, &rsize, &intOffset, &resourceIndex, currentFrame);

		intOffset += region.offset;
		
		ResourceStatus* status = resourceStatuses.Get(alloc->resourceStatus);

		EntryHandle index = bufferHandles[alloc->memIndex].bufferHandle;

		recorder->rbo->FillBuffer(index, region.size, intOffset, region.fillVal);

		status->currAction[resourceIndex] = TRANSFER_WRITE_DATA_RESOURCE;
		status->currStage[resourceIndex] = TRANSFER_BARRIER;
	}
}

AllocationInstanceIndex RenderInstance::GetAllocFromBuffer(BufferMemoryIndex bufferHandle, size_t structureSize, size_t copiesOfStructure, size_t alignment, AllocationType allocType, ComponentFormatType formatType, BufferAlignmentType bufferAlignmentType, AllocationInstanceIndex parentIndex, DeviceSlabAllocator* allocator)
{
	RenderBufferDescription* bufferDesc = bufferHandles.Get(bufferHandle);

	if (!bufferDesc)
	{
		return -1;
	}

	RHIDevice* rhiDevice = GetDeviceHandle(bufferDesc->deviceIndex);

	VKDevice* dev = rhiDevice->device;

	switch (bufferAlignmentType)
	{
	case BufferAlignmentType::UNIFORM_BUFFER_ALIGNMENT:
		alignment = RENDER_PWR2UP(alignment, rhiDevice->container.relatedPhysDeviceInfo->minUniformAlignment);
		break;
	case BufferAlignmentType::STORAGE_BUFFER_ALIGNMENT:
		alignment = RENDER_PWR2UP(alignment, rhiDevice->container.relatedPhysDeviceInfo->minStorageAlignment);
		break;
	}

	size_t allocSize = RENDER_PWR2UP((copiesOfStructure * structureSize), alignment);

	size_t copies = 1;

	switch (allocType)
	{
	case AllocationType::STATIC:
		break;
	case AllocationType::PERFRAME:
		copies = MAX_FRAMES_IN_FLIGHT;
		break;
	case AllocationType::PERDRAW:
		break;
	}

	ResourceIndex resourceIndex = resourceStatuses.Allocate();

	if (ResourceIndex() == resourceIndex)
	{
		return -1;
	}

	AllocationInstanceIndex allocIndex = allocations.Allocate();

	if (AllocationInstanceIndex() == allocIndex)
	{
		resourceStatuses.Free(resourceIndex);
		return allocIndex;
	}

	RenderAllocation* alloc = allocations.Get(allocIndex);

	ResourceStatus* resourceStatus = resourceStatuses.Get(resourceIndex);

	CleanInitializeAllocation(alloc);

	alloc->deviceIndex = bufferDesc->deviceIndex;

	CleanInitializeResourceStatus(resourceStatus);

	resourceStatus->resourceType = BUFFER_RESOURCE;

	alloc->resourceStatus = resourceIndex;

	int createRet = CreateResourceStatusActions(resourceStatus, copies, copies, 0);

	if (createRet < 0)
	{
		DestroyAllocation(allocIndex);
		return createRet;
	}
	
	size_t parentOffset = 0;

	if (AllocationInstanceIndex() != parentIndex)
	{
		RenderAllocation* alloc = allocations.Get(parentIndex);
		parentOffset = alloc->offset;
	}

	size_t location = allocator->Allocate(allocSize * copies, alignment);

	if (location < 0)
	{
		DestroyAllocation(allocIndex);
		return location;
	}

	alloc->memIndex = bufferHandle.index;
	alloc->offset = location + parentOffset;
	alloc->deviceAllocSize = allocSize * copies;
	alloc->requestedSize = structureSize;
	alloc->alignment = alignment;
	alloc->allocType = allocType;
	alloc->formatType = formatType;
	alloc->structureCopies = copiesOfStructure;
	alloc->parentAllocation = -1;
	
	if (ComponentFormatType::NO_BUFFER_FORMAT != formatType  && ComponentFormatType::RAW_8BIT_BUFFER != formatType)
	{
		alloc->viewIndex = dev->CreateBufferView(bufferDesc->bufferHandle, API::ConvertComponentFormatTypeToVulkanFormat(formatType), allocSize, location + parentOffset, copies);

		if (EntryHandle() == alloc->viewIndex)
		{
			GetLastDeviceDriverError(rhiDevice, STRING_VIEW_FROM_LITERAL("GetAllocFromBuffer : CreateBufferView failed"));
			DestroyAllocation(allocIndex);
			return -1;
		}
	}

	InitializeResourceStatus(resourceStatus, copies, copies, 0, 0, BEGINNING_OF_PIPE, ImageLayout::UNDEFINED);

	return allocIndex;
}

TextureIndex RenderInstance::CreateImageHandle(
	RenderDeviceIndex deviceSelection,
	size_t gpuMemAddress,
	uint32_t width, uint32_t height,
	uint32_t mipLevels, uint32_t arrayLayers, ImageFormat format, ImageType imageType, ImageUsageFlags usageFlags, ImageMemoryIndex poolIndex)
{
	RHIDevice* rhiDevice = GetDeviceHandle(deviceSelection);

	VKDevice* dev = rhiDevice->device;

	ResourceIndex resourceIndex = resourceStatuses.Allocate();

	if (ResourceIndex() == resourceIndex)
	{
		return {};
	}

	TextureIndex textureIndex = textureResourceHandles.Allocate();

	if (TextureIndex() == textureIndex)
	{
		resourceStatuses.Free(resourceIndex);
		return textureIndex;
	}

	ResourceStatus* textureStatus = resourceStatuses.Get(resourceIndex);

	CleanInitializeResourceStatus(textureStatus);

	RenderTextureDescription* renderTexDesc = textureResourceHandles.Get(textureIndex);

	CleanInitializeTextureResourceHandle(renderTexDesc);

	int totalTrackingLayers = mipLevels * arrayLayers;

	int createRet = CreateResourceStatusActions(textureStatus, totalTrackingLayers, totalTrackingLayers, totalTrackingLayers);

	if (createRet < 0)
	{
		resourceStatuses.Free(resourceIndex);
		textureResourceHandles.Free(textureIndex);
		return -1;
	}

	renderTexDesc->resourceStatusIndex = resourceIndex;

	VkFormat actualFormat = API::ConvertImageFormatToVulkanFormat(format);

	EntryHandle textureHandle = dev->CreateImage(
		width,
		height,
		mipLevels,
		actualFormat,
		arrayLayers,
		API::ConvertImageUsageFlagsToVulkanImageUsageFlags(usageFlags),
		1,
		gpuMemAddress,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_TILING_OPTIMAL,
		(imageType == ImageType::IMAGE_CUBE) ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : 0,
		API::ConvertImageTypeToVulkanImageType(imageType),
		imagePools[poolIndex].imagePoolHandle
	);

	TextureIndex indexRet(textureIndex);

	if (EntryHandle() == textureHandle)
	{
		DestroyTextureResourceHandle(indexRet);
		GetLastDeviceDriverError(rhiDevice, STRING_VIEW_FROM_LITERAL("CreateImageHandle : Driver Image Creation Failed:"));
		return -1;
	}

	renderTexDesc->arrayLayers = arrayLayers;
	renderTexDesc->mipLayers = mipLevels;
	renderTexDesc->imageWidth = width;
	renderTexDesc->imageHeight = height;
	renderTexDesc->format = format;
	renderTexDesc->textureIndex = textureHandle;
	renderTexDesc->imageType = imageType;
	renderTexDesc->viewCount = 0;
	renderTexDesc->deviceIndex = deviceSelection;

	InitializeResourceStatus(textureStatus, totalTrackingLayers, totalTrackingLayers, totalTrackingLayers, 0, BEGINNING_OF_PIPE, ImageLayout::UNDEFINED);

	return indexRet;
}

int RenderInstance::CreateImageView(TextureIndex& imageHandle, int firstMip, int mipCount, int firstLayer, int layerCount, ImageViewAspectMask imageAspect, ImageLayout desiredImageLayoutUsage)
{
	TextureViewIndex viewIndex;

	RenderTextureDescription* renderTexDesc = textureResourceHandles.Get(imageHandle);

	if (!renderTexDesc)
	{
		return -1;
	}

	RHIDevice* rhiDevice = GetDeviceHandle(renderTexDesc->deviceIndex);

	VKDevice* dev = rhiDevice->device;

	if (renderTexDesc->viewCount == MAX_VIEWS_ATTACHED_TO_TEXTURE)
	{
		return -1;
	}

	viewIndex = textureViewsResourceHandles.Allocate();

	if (TextureViewIndex() == viewIndex)
	{
		return -1;
	}

	EntryHandle viewHandle = dev->CreateImageView(renderTexDesc->textureIndex, 
		firstMip, firstLayer, 
		mipCount, layerCount, 
		API::ConvertImageFormatToVulkanFormat(renderTexDesc->format), 
		API::ConvertImageViewAspectMaskToVulkanImageAspectFlags(imageAspect),
		API::ConvertImageTypeToVulkanImageViewType(renderTexDesc->imageType)
	);

	if (EntryHandle() == viewHandle)
	{
		textureViewsResourceHandles.Free(viewIndex);
		GetLastDeviceDriverError(rhiDevice, STRING_VIEW_FROM_LITERAL("CreateImageView: Driver Image View Failed"));
		return -1;
	}

	renderTexDesc->viewIndex[renderTexDesc->viewCount] = viewIndex;

	int retIndex = renderTexDesc->viewCount++;

	RenderImageViewDescription* imageViewDesc = textureViewsResourceHandles.Get(viewIndex);

	CleanInitializeTextureViewsResourceHandle(imageViewDesc);

	imageViewDesc->firstLayer = firstLayer;
	imageViewDesc->firstMipLevel = firstMip;
	imageViewDesc->mask = imageAspect;
	imageViewDesc->layerCount = ((layerCount == IMAGE_VIEW_ALL_LAYERS) ? renderTexDesc->arrayLayers : layerCount);
	imageViewDesc->mipLevelCount = ((mipCount == IMAGE_VIEW_ALL_MIPS) ? renderTexDesc->mipLayers : mipCount);
	imageViewDesc->desiredLayoutForView = desiredImageLayoutUsage;
	imageViewDesc->viewIndex = viewHandle;

	return retIndex;
}

int RenderInstance::GetGPURequestedImageSizeAndAlignment(RenderDeviceIndex deviceSelection, uint32_t width, uint32_t height, uint32_t mipLevels, uint32_t layers, ImageFormat type, ImageUsageFlags usageFlags, size_t* actualImageSize, size_t* actualAlignment)
{
	RHIDevice* rhiDevice = GetDeviceHandle(deviceSelection);

	VKDevice* dev = rhiDevice->device;

	VkFormat actualFormat = API::ConvertImageFormatToVulkanFormat(type);

	dev->GetImageMemorySizeAndAlignment(
		width, height,
		mipLevels, actualFormat, layers,
		API::ConvertImageUsageFlagsToVulkanImageUsageFlags(usageFlags),
		1,
		VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_TILING_OPTIMAL, 0, VK_IMAGE_TYPE_2D,
		actualImageSize, actualAlignment
	);

	if (*actualImageSize == 0 && *actualAlignment == 0)
	{
		GetLastDeviceDriverError(rhiDevice, STRING_VIEW_FROM_LITERAL("GetGPURequestedImageSizeAndAlignment : call failed"));
		return -1;
	}

	return 0;
}

ImageMemoryIndex RenderInstance::CreateImagePool(RenderDeviceIndex deviceSelection, size_t size, ImageFormat format, int maxWidth, int maxHeight, ImageUsageFlags usageFlags, MemoryType memType)
{
	ImageMemoryIndex poolIndex;

	poolIndex = imagePools.Allocate();

	if (poolIndex.index < 0)
	{
		return poolIndex;
	}

	RHIDevice* rhiDevice = GetDeviceHandle(deviceSelection);

	VKDevice* dev = rhiDevice->device;

	VkFormat vkFormat = API::ConvertImageFormatToVulkanFormat(format);
	VkImageUsageFlags vkUsageFlags = API::ConvertImageUsageFlagsToVulkanImageUsageFlags(usageFlags);
	VkMemoryPropertyFlags vkMemPropertyFlags = API::ConvertMemoryTypeToVkMemoryPropertyFlags(memType);

	MemoryTypeInfo poolInfo = dev->FindImageMemoryIndexForPool(
		maxWidth, maxHeight,
		(uint32_t)log2(RENDER_MIN(maxWidth, maxHeight)), vkFormat, MAX_ARRAYS_FOR_BARRIER,
		vkUsageFlags,
		1, 
		vkMemPropertyFlags
	);

	if (~0ul == poolInfo.memoryIndex)
	{
		GetLastDeviceDriverError(rhiDevice, STRING_VIEW_FROM_LITERAL("CreateImagePool : finding memory pool index failed"));
		imagePools.Free(poolIndex);
		return {};
	}

	ImagePoolDescription* poolDesc = imagePools.Get(poolIndex);

	CleanInitializeImagePool(poolDesc);

	EntryHandle index = poolDesc->imagePoolHandle = dev->CreateImageMemoryPool(size, poolInfo.memoryIndex);

	if (EntryHandle() == index)
	{
		GetLastDeviceDriverError(rhiDevice, STRING_VIEW_FROM_LITERAL("CreateImagePool : finding memory pool index failed"));
		imagePools.Free(poolIndex);
		return {};
	}

	poolDesc->imagePoolSize = size;
	poolDesc->imagePoolType = memType;
	poolDesc->deviceIndex = deviceSelection;

	return poolIndex;
}

ShaderResourceSetBuilder RenderInstance::AllocateShaderResourceSet(ShaderResourceManagerIndex descriptorManagerIndex, RenderShaderGraphIndex& shaderGraphIndex, int targetSet, int setCount)
{ 
	ShaderResourceManager* manager = descriptorManagers.Get(descriptorManagerIndex);

    ShaderResourceSet* set = (ShaderResourceSet*)AllocateFromStorageAllocator(sizeof(ShaderResourceSet));
   
    ShaderGraph* graph = shaderGraphs.shaderGraphPtrs.Get(shaderGraphIndex);

    ShaderResourceSetTemplate* resourceSet = &graph->shaderResourceSetTemplates[targetSet];

    set->setCount = setCount;
	set->templateMetaData = resourceSet;

	int constantIndex = 0, resourceViewsBinding = 0;

	int descriptorSetIndex = -1;

	int success = 0;

	for (int h = 0; h < resourceSet->totalResourceCount; h++)
	{
		ShaderResourceTemplate* resource = &graph->shaderResources[resourceSet->resourceStart+h];

		if (resource->set != targetSet) continue;

		ShaderResourceHeader* desc = (ShaderResourceHeader*)&set->resourceBindings[resourceViewsBinding];

		ShaderResourceArray* descArray = (ShaderResourceArray*)desc;

		int actualRequestedArraySize = (DESCRIPTOR_COUNT_MASK & resource->arrayCount);

		switch (resource->type)
		{
		case ShaderResourceType::SAMPLERSTATE:
		{
			descArray->resourceArray.samplers.samplerHandles = (SamplerIndex*)AllocateFromStorageAllocator(sizeof(SamplerIndex) * actualRequestedArraySize, alignof(SamplerIndex));
			
			if (!descArray->resourceArray.samplers.samplerHandles)
			{
				success = -1;
			}
			
			descArray->resourceArray.samplers.samplerCount = 0;
			resourceViewsBinding++;
			break;
		}
		case ShaderResourceType::IMAGE2D:
		case ShaderResourceType::IMAGESTORE2D:
		{
			descArray->resourceArray.images.textureDetails = (ShaderResourceImageContainer*)AllocateFromStorageAllocator(sizeof(ShaderResourceImageContainer) * actualRequestedArraySize, alignof(ShaderResourceImageContainer));
			
			if (!descArray->resourceArray.images.textureDetails)
			{
				success = -1;
			}
			
			descArray->resourceArray.images.textureCount = 0;
			resourceViewsBinding++;
			break;
		}
		case ShaderResourceType::SAMPLER3D:
		case ShaderResourceType::SAMPLER2D:
		case ShaderResourceType::SAMPLERCUBE:
		{
			descArray->resourceArray.combinedImages.textureDetails = (ShaderResourceCombinedImageContainer*)AllocateFromStorageAllocator(
				sizeof(ShaderResourceCombinedImageContainer) * actualRequestedArraySize, alignof(ShaderResourceCombinedImageContainer));

			if (!descArray->resourceArray.combinedImages.textureDetails)
			{
				success = -1;
			}

			descArray->resourceArray.combinedImages.textureCount = 0;
			resourceViewsBinding++;
			break;
		}
		case ShaderResourceType::CONSTANT_BUFFER:
		{
			ShaderResourceConstantBuffer* constants = &set->constantBuffers[constantIndex++];

			desc = (ShaderResourceHeader*)constants;

			constants->size = resource->size;
			constants->offset = resource->offset;
			constants->stage = resource->stages;
			constants->rangeindex = resource->rangeIndex;

			break;
		}
		case ShaderResourceType::STORAGE_BUFFER:
		case ShaderResourceType::UNIFORM_BUFFER:
		{
			descArray->resourceArray.buffers.allocationIndex = (AllocationInstanceIndex*)AllocateFromStorageAllocator(sizeof(AllocationInstanceIndex) * actualRequestedArraySize, alignof(AllocationInstanceIndex));

			if (!descArray->resourceArray.buffers.allocationIndex)
			{
				success = -1;
			}

			descArray->resourceArray.buffers.bufferCount = 0;
			resourceViewsBinding++;
			break;
		}
		case ShaderResourceType::BUFFER_VIEW:
		{
			descArray->resourceArray.views.bufferCount = 0;
			descArray->resourceArray.views.allocationIndex = (AllocationInstanceIndex*)AllocateFromStorageAllocator(sizeof(AllocationInstanceIndex) * actualRequestedArraySize, alignof(AllocationInstanceIndex));

			if (!descArray->resourceArray.views.allocationIndex)
			{
				success = -1;
			}

			resourceViewsBinding++;
			break;
		}
		}

		desc->binding = resource->binding;
		desc->type = resource->type;
		desc->action = resource->action;
		desc->arrayCount = resource->arrayCount;
		desc->stage = resource->stages;
    }

	if (success)
	{
		//FIXME

		// DestroyShaderResourceSet(set);

		return { ShaderResourceManagerIndex(), -1, nullptr};
	}

	descriptorSetIndex = manager->AddShaderToSets(set);

	return { descriptorManagerIndex, descriptorSetIndex, set };
}

AttachmentGraphInstanceIndex RenderInstance::CreateAttachmentGraph(RenderDeviceIndex deviceSelection, StringView attachmentLayout)
{
	int attachmentGraphTemplateIndex = attachmentGraphs.Allocate();

	if (attachmentGraphTemplateIndex < 0)
	{
		return {};
	}

	AttachmentGraph* graph = attachmentGraphs.Get(attachmentGraphTemplateIndex);

	CleanInitializeAttachmentGraph(graph);

	int createRet = CreateAttachmentGraphFromFile(attachmentLayout, graph, cacheAllocator, internalRendererLogger);

	if (createRet)
	{
		return {};
	}

	AttachmentGraphInstanceIndex currentGraphInstance = CreateAttachmentGraphInstance(deviceSelection, graph);

	if (AttachmentGraphInstanceIndex() != currentGraphInstance)
	{
		int currentRenderPassCount = CreateRenderPass(attachmentGraphsInstances.Get(currentGraphInstance));

		if (currentRenderPassCount < 0)
		{
			DestroyAttachmentGraph(attachmentGraphTemplateIndex);
			DestroyAttachmentGraphInstance(currentGraphInstance);
			return {};
		}

		return currentGraphInstance;
	}

	DestroyAttachmentGraph(attachmentGraphTemplateIndex);
	
	return currentGraphInstance;
}

RenderPhysicalDeviceIndex RenderInstance::CreatePhysicalDeviceAdapter(GPUFeatureRequest* requestedPhysicalFeatures, LogicalDeviceFeatures* requestedDeviceFeatures)
{
	if (physicalDeviceCounter == maxPhysicalDevices)
	{
		internalRendererLogger->AddLogMessage(LOGERROR, STRING_VIEW_FROM_LITERAL("CreatePhysicalDeviceAdapter : too many gpus allocated"));
		return {};
	}

	uint32_t deviceExtNameCount = vkInstance->GetLogicalDeviceExtensionsCount(requestedDeviceFeatures);

	const char** deviceFeatureNames = (const char**)cacheAllocator->Allocate(sizeof(char*) * deviceExtNameCount);

	vkInstance->GetLogicalDeviceExtensions(requestedDeviceFeatures, deviceFeatureNames);

	int driverGpuIndex = -1;

	EntryHandle physicalIndex = vkInstance->CreatePhysicalDevice(requestedPhysicalFeatures, deviceFeatureNames, deviceExtNameCount, &driverGpuIndex);

	if (EntryHandle() == physicalIndex)
	{
		GetLastInstanceDriverError(STRING_VIEW_FROM_LITERAL("CreatePhysicalDeviceAdapter : failed to create gpu adapter on driver"));
		return {};
	}

	int physicalEntryIndex = physicalDeviceCounter++;

	RenderPhysicalDeviceContainer* container = &physicalDeviceIndices[physicalEntryIndex];

	CleanInitializePhysicalDeviceIndices(container);

	container->physicalDeviceIndex = physicalIndex;
	container->information.minUniformAlignment = vkInstance->GetMinimumUniformBufferAlignment(physicalIndex);
	container->information.minStorageAlignment = vkInstance->GetMinimumStorageBufferAlignment(physicalIndex);
	container->information.maxMSAALevels = findMSB(vkInstance->GetMaxMSAALevels(physicalIndex));
	container->information.deviceTimeStampPeriodNS = vkInstance->GetTimeStampPeriod(physicalIndex);
	container->information.optimalImageCopyOffsetAlignment = vkInstance->GetOptimalImageCopyOffsetAlignment(physicalIndex);
	container->internalDriverDeviceListIdentifier = driverGpuIndex;

	RenderPhysicalDeviceIndex indexRet{};

	indexRet = physicalEntryIndex;

	return indexRet;
}

int RenderInstance::OpenPhysicalDevicePicker()
{
	if (physicalDeviceCounter == maxPhysicalDevices)
	{
		internalRendererLogger->AddLogMessage(LOGERROR, STRING_VIEW_FROM_LITERAL("OpenPhysicalDevicePicker: Too many GPUs allocated based on CreateInfo"));
		return -1;
	}

	int gpuCount = vkInstance->GetNumberOfGPUDevices();

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
	vkInstance->FreePotentialGPUs();
}

RenderPhysicalDeviceIndex RenderInstance::CreatePhysicalDeviceAdapterWithQuerying(GPUFeatureRequest* requestedPhysicalFeatures, LogicalDeviceFeatures* requestedDeviceFeatures)
{
	if (physicalDeviceCounter == maxPhysicalDevices)
	{
		internalRendererLogger->AddLogMessage(LOGERROR, STRING_VIEW_FROM_LITERAL("CreatePhysicalDeviceAdapterWithQuerying: Too many GPUs allocated based on CreateInfo"));
		return {};
	}

	uint32_t deviceExtNameCount = vkInstance->GetLogicalDeviceExtensionsCount(requestedDeviceFeatures);

	const char** deviceFeatureNames = (const char**)cacheAllocator->Allocate(sizeof(char*) * deviceExtNameCount);

	vkInstance->GetLogicalDeviceExtensions(requestedDeviceFeatures, deviceFeatureNames);

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
		
		if (vkInstance->QuerySpecificPhysicalDeviceFeatures(requestedPhysicalFeatures, &currentRequest, deviceFeatureNames, deviceExtNameCount, gpuIndex, &currentDeviceExtMask))
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

	EntryHandle physicalIndex = vkInstance->CreateGPUFromIndex(gpuIndex);

	if (EntryHandle() == physicalIndex)
	{
		GetLastInstanceDriverError(STRING_VIEW_FROM_LITERAL("CreatePhysicalDeviceAdapterWithQuerying: GPU creation by driver failed"));
		return {};
	}

	int physicalEntryIndex = physicalDeviceCounter++;

	RenderPhysicalDeviceContainer* container = &physicalDeviceIndices[physicalEntryIndex];

	CleanInitializePhysicalDeviceIndices(container);

	container->physicalDeviceIndex = physicalIndex;
	container->information.minUniformAlignment = vkInstance->GetMinimumUniformBufferAlignment(physicalIndex);
	container->information.minStorageAlignment = vkInstance->GetMinimumStorageBufferAlignment(physicalIndex);
	container->information.maxMSAALevels = findMSB(vkInstance->GetMaxMSAALevels(physicalIndex));
	container->information.deviceTimeStampPeriodNS = vkInstance->GetTimeStampPeriod(physicalIndex);
	container->information.optimalImageCopyOffsetAlignment = vkInstance->GetOptimalImageCopyOffsetAlignment(physicalIndex);
	container->internalDriverDeviceListIdentifier = gpuIndex;

	RenderPhysicalDeviceIndex indexRet{};

	indexRet = physicalEntryIndex;

	return indexRet;
}

int RenderInstance::CreatePerFrameStagingBuffers(RenderDeviceIndex deviceSelection, uint32_t bufferSize)
{
	RHIDevice* rhiDevice = GetDeviceHandle(deviceSelection);

	VKDevice* dev = rhiDevice->device;

	for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		EntryHandle handle = rhiDevice->container.stagingBuffers[i] = dev->CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			API::ConvertMemoryTypeToVkMemoryPropertyFlags(MemoryTypeBits::HOST_MEMORY_COHERENT_TYPE)
		);

		if (EntryHandle() == handle)
		{
			for (int j = 0; j < i; j++)
				dev->DestroyBuffer(rhiDevice->container.stagingBuffers[j]);

			GetLastDeviceDriverError(rhiDevice, STRING_VIEW_FROM_LITERAL("CreatePerFrameStagingBuffers failed:"));
			return -1;
		}

		rhiDevice->container.stagingBufferAllocators[i].dataSize = bufferSize;
		rhiDevice->container.stagingBufferAllocators[i].dataAllocator = 0;
	}

	return 0;
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

	uint32_t deviceExtNameCount = vkInstance->GetLogicalDeviceExtensionsCount(createInfo->requestedDeviceFeatures);

	const char** deviceFeatureNames = (const char**)cacheAllocator->Allocate(sizeof(char*) * deviceExtNameCount);

	vkInstance->GetLogicalDeviceExtensions(createInfo->requestedDeviceFeatures, deviceFeatureNames);

	VkPhysicalDeviceVulkan12Features features12{};
	VkPhysicalDeviceFeatures2 features2{};

	API::ConvertGPUFeatureRequestToVkPhysicalDeviceProperties(createInfo->requestedPhysicalFeatures, &features2, &features12);

	EntryHandle deviceIndex = vkInstance->CreateLogicalDevice(physicalIndex);

	if (EntryHandle() == deviceIndex)
	{
		GetLastInstanceDriverError(STRING_VIEW_FROM_LITERAL("CreateLogicalDevice: device creation failed from instance"));
		logicalDeviceCounter--;
		return ret;
	}

	rhiDevice->container.logicalDeviceIndex = deviceIndex;

	VKDevice* majorDevice = vkInstance->GetLogicalDevice(deviceIndex);

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

	queueSuccessful = majorDevice->GetPresentQueue(&queueIndices[1], &queueCounts[1], vkInstance->GetRenderSurface(windowsSurfaces[createInfo->surfaceIndexForPresent]()), famPropsContainer);

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

	int createRet =	majorDevice->CreateLogicalDevice(
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

		vkInstance->DestroyLogicalDevice(deviceIndex);
		
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

SwapChainIndex RenderInstance::CreateSwapChainHandle(RenderDeviceIndex deviceSelection, WindowIndex surfaceIndex, ImageFormat mainBackBufferColorFormat, uint32_t _width, uint32_t _height)
{
	SwapChainIndex swapChainInternalIndex{};

	RHIDevice* rhiDevice = GetDeviceHandle(deviceSelection);

	VKDevice* dev = rhiDevice->device;

	RenderWindowSpecificData* winData = windowsSurfaces.Get(surfaceIndex);

	if (!winData)
	{
		return {};
	}

	swapChainInternalIndex = swapChains.Allocate();

	if (SwapChainIndex() == swapChainInternalIndex)
	{
		return {};
	}

	EntryHandle swapChainIndex = dev->CreateSwapChain(MAX_FRAMES_IN_FLIGHT, MAX_FRAMES_IN_FLIGHT, API::ConvertImageFormatToVulkanFormat(mainBackBufferColorFormat), winData->vkRenderSurface);

	if (EntryHandle() == swapChainIndex)
	{
		swapChains.Free(swapChainInternalIndex);
		GetLastDeviceDriverError(rhiDevice, STRING_VIEW_FROM_LITERAL("CreateSwapChain: failed to create driver swapchain handle"));
		return {};
	}

	int createRet = CreateDriverSwapChainData(rhiDevice, swapChainIndex, _width, _height, false);

	if (createRet)
	{
		DestroyDriverSwapChain(rhiDevice, swapChainIndex);
		swapChains.Free(swapChainInternalIndex);
		return {};
	}

	VKSwapChain* vkSwcData = dev->GetSwapChain(swapChainIndex);

	RenderSwapchainData* swcData = swapChains.Get(swapChainInternalIndex);

	CleanInitializeSwapChain(swcData);

	swcData->swapChainIdx = swapChainIndex;
	swcData->height = _height;
	swcData->width = _width;

	uint32_t imageCount = swcData->imageCount = vkSwcData->imageCount;

	EntryHandle* renderWait = dev->CreateSemaphores(MAX_FRAMES_IN_FLIGHT);

	if (!renderWait)
	{
		DestroySwapChain(swapChainInternalIndex);
		return {};
	}

	EntryHandle* renderFinished = dev->CreateSemaphores(imageCount);

	if (!renderFinished)
	{
		for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
			dev->DestroySemaphore(renderWait[i]);

		DestroySwapChain(swapChainInternalIndex);
		
		return {};
	}


	for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		swcData->rendererWaitSemaphores[i] = renderWait[i];
	}

	for (uint32_t i = 0; i < imageCount; i++)
	{
		swcData->rendererFinishedSemaphores[i] = renderFinished[i];
	}

	swcData->deviceIndex = deviceSelection;

	bool texturesAvailable = textureResourceHandles.DoIHaveNFreeElements(imageCount);
	bool resourcesAvailable = resourceStatuses.DoIHaveNFreeElements(imageCount);
	bool viewsAvailable = textureViewsResourceHandles.DoIHaveNFreeElements(imageCount);

	int success = -1;

	if (texturesAvailable && resourcesAvailable && viewsAvailable)
	{
		success = 0;

		for (uint32_t i = 0; i < imageCount; i++)
		{
			TextureIndex textureID = textureResourceHandles.Allocate();

			RenderTextureDescription* desc = textureResourceHandles.Get(textureID);

			ResourceIndex resourceIndex = resourceStatuses.Allocate();

			ResourceStatus* status = resourceStatuses.Get(resourceIndex);

			CleanInitializeResourceStatus(status);

			CleanInitializeTextureResourceHandle(desc);

			status->resourceType = ResourceStatusType::MANAGED_IMAGE_RESOURCE;

			desc->resourceStatusIndex = resourceIndex;

			swcData->textureIds[i] = textureID;

			TextureViewIndex viewIndex = textureViewsResourceHandles.Allocate();

			RenderImageViewDescription* viewDesc = textureViewsResourceHandles.Get(viewIndex);

			CleanInitializeTextureViewsResourceHandle(viewDesc);

			desc->arrayLayers = 1;
			desc->mipLayers = 1;
			desc->imageHeight = _height;
			desc->imageWidth = _width;
			desc->format = mainBackBufferColorFormat;
			desc->viewCount = 1;
			desc->viewIndex[0] = viewIndex;
			desc->deviceIndex = deviceSelection;

			viewDesc->viewIndex = vkSwcData->imageViews[i];
			viewDesc->mask = COLOR_IMAGE_ASPECT;
			viewDesc->firstLayer = viewDesc->firstMipLevel = 0;
			viewDesc->layerCount = viewDesc->mipLevelCount = 1;
			viewDesc->desiredLayoutForView = ImageLayout::COLORATTACHMENT;
		}
	}

	if (success)
	{
		DestroySwapChain(swapChainInternalIndex);

		return {};
	}

	return  swapChainInternalIndex;
}

ImageFormat RenderInstance::FindSupportedBackBufferColorFormat(RenderPhysicalDeviceIndex physicalDeviceIndex, WindowIndex surfaceIndex, ImageFormat* requestedFormats, uint32_t requestSize)
{
	EntryHandle physicalIndex = physicalDeviceIndices[physicalDeviceIndex.index].physicalDeviceIndex;

	for (uint32_t i = 0; i < requestSize; i++)
	{
		bool ret = vkInstance->ValidateSwapChainFormatSupport(physicalIndex, API::ConvertImageFormatToVulkanFormat(requestedFormats[i]), windowsSurfaces[surfaceIndex]());

		if (ret)
		{
			return requestedFormats[i];
		}
	}

	GetLastInstanceDriverError(STRING_VIEW_FROM_LITERAL("FindSupportedBackBufferColorFormat: could not find supported swc format from request"));
	
	return ImageFormat::IMAGE_UNKNOWN;
}

ImageFormat RenderInstance::FindSupportedDepthFormat(RenderDeviceIndex deviceSelection, ImageFormat* requestedFormats, uint32_t requestSize)
{
	RHIDevice* rhiDevice = GetDeviceHandle(deviceSelection);

	VKDevice* dev = rhiDevice->device;

	VkFormat format;

	for (uint32_t i = 0; i < requestSize; i++)
	{
		format = API::ConvertImageFormatToVulkanFormat(requestedFormats[i]);

		format = VK::Utils::findSupportedFormat(dev->gpu,
			&format,
			1,
			VK_IMAGE_TILING_OPTIMAL,
			VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
		);

		if (format != VK_FORMAT_UNDEFINED)
		{
			return requestedFormats[i];
		}
	}

	internalRendererLogger->AddLogMessage(LOGERROR, STRING_VIEW_FROM_LITERAL("FindSupportedDepthFormat: could not find supported depth format from request"));

	return ImageFormat::IMAGE_UNKNOWN;
}

SamplerIndex RenderInstance::CreateSampler(RenderDeviceIndex deviceSelection, uint32_t baseLod, uint32_t maxLod, SamplerFilterMode minFilter, SamplerFilterMode magFilter, SamplerAddressMode addressMode, SamplerMipmapMode mipmapMode, CompareOp compareOp)
{
	RHIDevice* rhiDevice = GetDeviceHandle(deviceSelection);

	VKDevice* dev = rhiDevice->device;

	SamplerIndex samplerIndex = samplerResourceHandles.Allocate();

	if (SamplerIndex() == samplerIndex)
	{
		return {};
	}

	VkSamplerAddressMode mode = API::ConvertSamplerAddressModeToVulkanSamplerAddressMode(addressMode);

	EntryHandle samplerHandle = dev->CreateSampler(
		API::ConvertSamplerFilterModeToVulkanFilter(minFilter), 
		API::ConvertSamplerFilterModeToVulkanFilter(magFilter),
		mode,
		mode,
		mode,
		API::ConvertCompareOpToVulkanCompareOp(compareOp),
		API::ConvertSamplerMipmapModeToVulkanSamplerMipmapMode(mipmapMode),
		static_cast<float>(baseLod),
		static_cast<float>(maxLod)
	);

	if (EntryHandle() == samplerHandle)
	{
		GetLastDeviceDriverError(rhiDevice, STRING_VIEW_FROM_LITERAL("CreateSampler: file driver creation"));
		DestroySamplerResourceHandle(deviceSelection, samplerIndex);
		return -1;
	}
	
	samplerResourceHandles.pool[samplerIndex.index] = samplerHandle;

	return SamplerIndex(samplerIndex);
}

uint32_t RenderInstance::GetSwapChainHeight(SwapChainIndex swapChainIndex)
{
	RenderSwapchainData* data = swapChains.Get(swapChainIndex);

	if (!data) return ~0ul;

	return data->height;
}

uint32_t RenderInstance::GetSwapChainWidth(SwapChainIndex swapChainIndex)
{
	RenderSwapchainData* data = swapChains.Get(swapChainIndex);

	if (!data) return ~0ul;

	return data->width;
}

int RenderInstance::CreateShaderResourceSet(ShaderResourceManager* descriptorManager, int descriptorSet)
{
	if (descriptorSet < 0 || descriptorManager->descriptorSetHandles.maxCount <= descriptorSet)
	{
		return -1;
	}

	if (descriptorManager->descriptorSetHandles[descriptorSet] != EntryHandle())
	{
		return 0;
	}

	RHIDevice* rhiDevice = GetDeviceHandle(descriptorManager->deviceIndex);

	VKDevice* dev = rhiDevice->device;

	ShaderResourceSet* set = descriptorManager->descriptorSets[descriptorSet];

	int frames = set->setCount;	

	uint32_t varCountRequested = 0;

	int bindingCount = set->templateMetaData->bindingCount;

	ShaderResourceTemplateInstanceIndex layoutHandle = set->templateMetaData->vulkanDescLayout;

	int lastBinding = bindingCount - 1;

	ShaderResourceHeader* lastheader = (ShaderResourceHeader*)&set->resourceBindings[lastBinding];

	if (lastheader->arrayCount & UNBOUNDED_DESCRIPTOR_ARRAY)
	{
		varCountRequested = (lastheader->arrayCount & DESCRIPTOR_COUNT_MASK);
	}

	DescriptorSetBuilder* builder = dev->CreateDescriptorSetBuilder(descriptorManager->deviceResourceHeap, shaderResourceTemplates[layoutHandle].resourceTemplateInstanceHandle, frames, varCountRequested);

	int success = 0;

	for (int i = 0; i < bindingCount; i++)
	{
		ShaderResourceArray* header = (ShaderResourceArray*)&set->resourceBindings[i];

		switch (header->type)
		{
			case ShaderResourceType::SAMPLERSTATE:
			{
				ShaderResourceSampler* samplers = &header->resourceArray.samplers;
	
				for (int sampler = 0; sampler < samplers->samplerCount; sampler++)
				{
					EntryHandle samplerHandle = samplerResourceHandles[samplers->samplerHandles[sampler]];

					if (EntryHandle() == samplerHandle)
					{
						internalRendererLogger->AddLogMessage(LOGERROR, STRING_VIEW_FROM_LITERAL("CreateShaderResourceSet : Invalid sampler handle"));
						success = -1;
						continue;
					}

					builder->AddSamplerDescription(samplerHandle, sampler, i, 0, frames);
				}

				break;
			}
			case ShaderResourceType::IMAGE2D:
			{
				ShaderResourceImage* image = &header->resourceArray.images;
				
				for (int imageIndex = 0; imageIndex < image->textureCount; imageIndex++)
				{
					RenderTextureDescription* desc = textureResourceHandles.Get(image->textureDetails[imageIndex].textureHandle);

					if (!desc)
					{
						internalRendererLogger->AddLogMessage(LOGERROR, STRING_VIEW_FROM_LITERAL("CreateShaderResourceSet : Invalid texture handle"));
						success = -1;
						continue;
					}

					RenderImageViewDescription* imageViewDesc = textureViewsResourceHandles.Get(desc->viewIndex[image->textureDetails[imageIndex].viewIndex]);

					if (!imageViewDesc)
					{
						internalRendererLogger->AddLogMessage(LOGERROR, STRING_VIEW_FROM_LITERAL("CreateShaderResourceSet : Invalid image view handle"));
						success = -1;
						continue;
					}

					builder->AddImageResourceDescription(imageViewDesc->viewIndex, API::ConvertImageLayoutToVulkanImageLayout(imageViewDesc->desiredLayoutForView), imageIndex, i, 0, frames);
				}

				break;
			}
			case ShaderResourceType::IMAGESTORE2D:
			{
				ShaderResourceImage* image = &header->resourceArray.images;

				for (int imageIndex = 0; imageIndex < image->textureCount; imageIndex++)
				{
					RenderTextureDescription* desc = textureResourceHandles.Get(image->textureDetails[imageIndex].textureHandle);

					if (!desc)
					{
						internalRendererLogger->AddLogMessage(LOGERROR, STRING_VIEW_FROM_LITERAL("CreateShaderResourceSet : Invalid storage texture handle"));
						success = -1;
						continue;
					}

					RenderImageViewDescription* imageViewDesc = textureViewsResourceHandles.Get(desc->viewIndex[image->textureDetails[imageIndex].viewIndex]);

					if (!imageViewDesc)
					{
						internalRendererLogger->AddLogMessage(LOGERROR, STRING_VIEW_FROM_LITERAL("CreateShaderResourceSet : Invalid storage image view handle"));
						success = -1;
						continue;
					}

					builder->AddStorageImageDescription(imageViewDesc->viewIndex, API::ConvertImageLayoutToVulkanImageLayout(imageViewDesc->desiredLayoutForView), imageIndex, i, 0, frames);
				}
				break;
			}
			case ShaderResourceType::SAMPLER3D:
			case ShaderResourceType::SAMPLER2D:
			case ShaderResourceType::SAMPLERCUBE:
			{
				ShaderResourceCombinedImage* image = &header->resourceArray.combinedImages;
			
				for (int imageIndex = 0; imageIndex < image->textureCount; imageIndex++)
				{
					RenderTextureDescription* desc = textureResourceHandles.Get(image->textureDetails[imageIndex].textureHandle);

					if (!desc)
					{
						internalRendererLogger->AddLogMessage(LOGERROR, STRING_VIEW_FROM_LITERAL("CreateShaderResourceSet : Invalid combined image texture handle"));
						success = -1;
						continue;
					}

					RenderImageViewDescription* imageViewDesc = textureViewsResourceHandles.Get(desc->viewIndex[image->textureDetails[imageIndex].viewIndex]);

					if (!imageViewDesc)
					{
						internalRendererLogger->AddLogMessage(LOGERROR, STRING_VIEW_FROM_LITERAL("CreateShaderResourceSet : Invalid combined image view handle"));
						success = -1;
						continue;
					}

					EntryHandle samplerHandle = samplerResourceHandles[image->textureDetails[imageIndex].samplerHandle];

					if (EntryHandle() == samplerHandle)
					{
						internalRendererLogger->AddLogMessage(LOGERROR, STRING_VIEW_FROM_LITERAL("CreateShaderResourceSet : Invalid combined image sampler handle"));
						success = -1;
						continue;
					}

					builder->AddCombinedTextureArray(imageViewDesc->viewIndex, samplerHandle, API::ConvertImageLayoutToVulkanImageLayout(imageViewDesc->desiredLayoutForView), imageIndex, i, 0, frames);
				}
				
				break;
			}
			case ShaderResourceType::STORAGE_BUFFER:
			{
				ShaderResourceBuffer* buffer = &header->resourceArray.buffers;
				
				int arrayCount = buffer->bufferCount;

				for (int j = 0; j < arrayCount; j++)
				{
					RenderAllocation* alloc = allocations.Get(buffer->allocationIndex[j]);

					if (!alloc)
					{
						internalRendererLogger->AddLogMessage(LOGERROR, STRING_VIEW_FROM_LITERAL("CreateShaderResourceSet : Invalid storage allocation handle"));
						success = -1;
						continue;
					}

					if (alloc->allocType == AllocationType::PERFRAME)
						builder->AddStorageBuffer(dev->GetBufferHandle(bufferHandles[alloc->memIndex].bufferHandle), alloc->deviceAllocSize / frames, i, frames, alloc->offset, 0, j);
					else
						builder->AddStorageBufferDirect(dev->GetBufferHandle(bufferHandles[alloc->memIndex].bufferHandle), alloc->deviceAllocSize, i, frames, alloc->offset, 0, j);
				}
				break;
			}
			case ShaderResourceType::UNIFORM_BUFFER:
			{
				ShaderResourceBuffer* buffer = &header->resourceArray.buffers;

				int arrayCount = buffer->bufferCount;

				for (int j = 0; j < arrayCount; j++)
				{
					RenderAllocation* alloc = allocations.Get(buffer->allocationIndex[j]);

					if (!alloc)
					{
						internalRendererLogger->AddLogMessage(LOGERROR, STRING_VIEW_FROM_LITERAL("CreateShaderResourceSet : Invalid uniform allocation handle"));
						success = -1;
						continue;
					}

					if (alloc->allocType == AllocationType::PERFRAME)
						builder->AddUniformBuffer(dev->GetBufferHandle(bufferHandles[alloc->memIndex].bufferHandle), alloc->deviceAllocSize / frames, i, frames, alloc->offset, 0, j);
					else
						builder->AddUniformBufferDirect(dev->GetBufferHandle(bufferHandles[alloc->memIndex].bufferHandle), alloc->deviceAllocSize, i, frames, alloc->offset, 0, j);
				}
				break;
			}
			case ShaderResourceType::BUFFER_VIEW:
			{
				ShaderResourceBuffer* buffer = &header->resourceArray.views;

				int arrayCount = buffer->bufferCount;

				for (int j = 0; j < arrayCount; j++)
				{
					RenderAllocation* alloc = allocations.Get(buffer->allocationIndex[j]);

					if (!alloc)
					{
						internalRendererLogger->AddLogMessage(LOGERROR, STRING_VIEW_FROM_LITERAL("CreateShaderResourceSet : Invalid buffer view allocation handle"));
						success = -1;
						continue;
					}

					int frameCount = (alloc->allocType == AllocationType::PERFRAME) ? MAX_FRAMES_IN_FLIGHT : 1;

					for (int g = 0; g < frameCount; g++)
					{
						VkBufferView handle = dev->GetBufferView(alloc->viewIndex, g);

						if (VK_NULL_HANDLE == handle)
						{
							internalRendererLogger->AddLogMessage(LOGERROR, STRING_VIEW_FROM_LITERAL("CreateShaderResourceSet : Invalid driver buffer view handle"));
							success = -1;
							break;
						}

						if (header->action == ShaderResourceAction::SHADERREAD)
						{
							builder->AddUniformBufferView(handle, i, g, 1, j);
						}
						else if (header->action == ShaderResourceAction::SHADERWRITE)
						{
							builder->AddStorageBufferView(handle, i, g, 1, j);
						}
					}
				}
				break;
			}
		}
	}

	if (!success)
	{
		EntryHandle handle = builder->AddDescriptorsToCache();

		descriptorManager->descriptorSetHandles.pool[descriptorSet] = handle;
	}

	return success;
}

PipelineHandleIndex RenderInstance::CreateGraphicsPipelineObject(GraphicsIntermediaryPipelineInfo* info)
{
	GraphPipelineDescription* graph = graphPipelineDescriptions.Get(info->pipelinename);

	PipelineHandleIndex pipelineInstHandle = -1;

	if (!graph)
	{
		return pipelineInstHandle;
	}

	pipelineInstHandle = pipelineHandles.Allocate();

	if (PipelineHandleIndex() == pipelineInstHandle)
	{
		return pipelineInstHandle;
	}

	PipelineInstanceData* pid = &graph->instanceData;

	PipelineHandle* posStruct = pipelineHandles.Get(pipelineInstHandle);

	CleanInitializePipelineHandle(posStruct);
	
	posStruct->group = GRAPHICSO;
	posStruct->pipelineIdentifierGroup = info->pipelinename;
	posStruct->resourceSetCount = info->descCount;

	uint32_t pushRangeCount = 0;

	int success = 0;

	for (uint32_t i = 0; i < info->descCount; i++)
	{
		ShaderResourceManager* descriptorManager = descriptorManagers.Get(info->descriptorsetid[i].descriptorManagerIndex);

		success = CreateShaderResourceSet(descriptorManager, info->descriptorsetid[i].descriptorSetIndex);

		if (success)
			break;

		posStruct->resourceSets[i] = info->descriptorsetid[i];
		pushRangeCount += descriptorManager->GetConstantBufferCount(info->descriptorsetid[i].descriptorSetIndex);
	}

	if (!success)
	{
		posStruct->pushRangeCount = pushRangeCount;
		posStruct->indexBufferHandle = info->indexBufferHandle;
		posStruct->indexCount = info->indexCount;
		posStruct->vertexBufferHandle = info->vertexBufferHandle;
		posStruct->vertexCount = info->vertexCount;
		posStruct->indirectBufferHandle = info->indirectAllocation;
		posStruct->indirectCountBufferHandle = info->indirectCountAllocation;
		posStruct->instanceCount = info->instanceCount;
		posStruct->indexSize = info->indexSize;
		posStruct->indirectDrawCount = info->indirectDrawCount;

		int renderStateCount = 0;

		for (uint32_t a = 0; a < pid->frameGraphCount; a++)
		{
			AttachmentGraphInstance* graphInstance = attachmentGraphsInstances.Get(pid->frameGraphIndices[a]);

			AttachmentRenderPassInstance* renderPassInstance = &graphInstance->passes[pid->frameGraphRenderPasses[a]];

			renderStateCount += renderPassInstance->maxSampleCount;
		}

		posStruct->numHandles = renderStateCount;
	}
	else
	{
		DestroyPipelineHandle(pipelineInstHandle);
		pipelineInstHandle = -1;
	}

	return pipelineInstHandle;
}

ShaderComputeLayout* RenderInstance::GetComputeLayout(RenderShaderGraphIndex& shaderGraphIndex)
{
	ShaderGraph* graph = shaderGraphs.shaderGraphPtrs.Get(shaderGraphIndex);

	if (!graph)
	{
		return nullptr;
	}

	ShaderMap* map = &graph->shaderMaps[0];

	ShaderDetails* details = shaderGraphs.shaderDetails.Get(map->shaderReference);

	return &details->computeLayout;
}

PipelineHandleIndex RenderInstance::CreateComputePipelineObject(ComputeIntermediaryPipelineInfo* info)
{
	GraphPipelineDescription* graph = graphPipelineDescriptions.Get(info->pipelinename);

	PipelineHandleIndex pipelineInstHandle;

	if (!graph)
	{
		return pipelineInstHandle;
	}

	pipelineInstHandle = pipelineHandles.Allocate();

	if (PipelineHandleIndex() == pipelineInstHandle)
	{
		return pipelineInstHandle;
	}

	PipelineHandle* posStruct = pipelineHandles.Get(pipelineInstHandle);

	CleanInitializePipelineHandle(posStruct);
	
	posStruct->numHandles = 1;
	posStruct->group = COMPUTESO;
	posStruct->pipelineIdentifierGroup = info->pipelinename;
	posStruct->resourceSetCount = info->descCount;
	posStruct->x = info->x;
	posStruct->y = info->y;
	posStruct->z = info->z;
	posStruct->indirectDispatchCommandHandle = info->indirectDispatchAllocation;
	
	uint32_t pushRangeCount = 0;

	int success = 0;

	for (uint32_t i = 0; i < info->descCount; i++)
	{
		ShaderResourceManager* descriptorManager = descriptorManagers.Get(info->descriptorsetid[i].descriptorManagerIndex);
		success = CreateShaderResourceSet(descriptorManager, info->descriptorsetid[i].descriptorSetIndex);

		if (success)
			break;

		posStruct->resourceSets[i] = info->descriptorsetid[i];
		pushRangeCount += descriptorManager->GetConstantBufferCount(info->descriptorsetid[i].descriptorSetIndex);
	}

	posStruct->pushRangeCount = pushRangeCount;

	if (success)
	{
		DestroyPipelineHandle(pipelineInstHandle);
		pipelineInstHandle = -1;
	}

	return pipelineInstHandle;
}

void RenderInstance::DrawScene(RenderDeviceIndex deviceSelection, GPUCommandStreamIndex& commandStreamIndex, uint32_t imageIndex)
{
	RHIDevice* rhiDevice = GetDeviceHandle(deviceSelection);

	VKDevice* dev = rhiDevice->device;

	rhiDevice->container.stagingBufferAllocators[currentFrame].dataAllocator = 0;

	EntryHandle cbindex = rhiDevice->container.currentCommandBufferIndex[currentFrame];

	CommandRecorder recorder{};
	
	RecordingBufferObject rcb = dev->GetRecordingBufferObject(cbindex);

	uint32_t accumulatorIndex = PopBarrierAccumulator();

	BarrierAccumulator* accumulator = &barrierAccumulators[accumulatorIndex];

	recorder.rbo = &rcb;
	recorder.accumulator = accumulator;
	recorder.barrierAccumulatorIndex = accumulatorIndex;
	recorder.device = rhiDevice;

	ResetCommandPool(&recorder);

	SwapUpdateCommands();

	UploadHostTransfers(&recorder);

	UploadDescriptorsUpdates(&recorder);

	BeginCommandRecording(&recorder);

	if (rhiDevice->container.queriesAreActive)
	{
		ResetDeviceQueries(&recorder, rhiDevice->container.queryPoolIndex, rhiDevice->container.maxQueryResults * currentFrame, rhiDevice->container.maxQueryResults);
	}

	UploadDeviceLocalTransfers(&recorder);

	InvokeTransferCommands(&recorder);

	UploadImageMemoryTransfers(&recorder);

	int commandCountIter = 0;

	GPUCommandStreamAllocator* stream = gpuCommandStreams.Get(commandStreamIndex);

	while (commandCountIter < stream->commandCount)
	{
		GPUCommand* command = &stream->commands[commandCountIter];

		if (command->streamType == GPUCommandStreamType::COMPUTE_QUEUE_COMMANDS)
		{
			WriteDeviceQuery(&recorder, StageBits::COMPUTE_BARRIER);
			
			ComputeQueue* queue = computeQueues.Get(command->commandIndex.indexForComputeQueue);

			for (uint32_t pipeInst = 0; pipeInst < queue->queueCount; pipeInst++)
			{
				PipelineHandleIndex pipelineIndex = queue->pipelines[pipeInst];

				PipelineHandle* handle = pipelineHandles.Get(pipelineIndex);

				GeneratePipelineDescriptorBarriers(&recorder, handle->resourceSets, handle->resourceSetCount, pipelineIndex);

				GenerateComputeDispatchBindingsBarriers(&recorder, handle, pipelineIndex);
			}

			InsertAccumulatedBarriers(&recorder);

			for (uint32_t pipeInst = 0; pipeInst < queue->queueCount; pipeInst++)
			{
				PipelineHandleIndex pipelineIndex = queue->pipelines[pipeInst];

				PipelineHandle* handle = pipelineHandles.Get(pipelineIndex);

				EntryHandle pipelineTemp = graphPipelineDescriptions.Get(handle->pipelineIdentifierGroup)->pipelineIndices[0];

				BindComputePipelineCmd(&recorder, pipelineTemp);

				for (uint32_t ii = 0; ii < handle->resourceSetCount; ii++)
				{
					ShaderResourceManager* descriptorManager = descriptorManagers.Get(handle->resourceSets[ii].descriptorManagerIndex);

					BindComputeDescriptorSetsCmd(&recorder, descriptorManager->descriptorSetHandles[handle->resourceSets[ii].descriptorSetIndex], currentFrame, 1, ii, 0, nullptr);
				}

				for (uint32_t ii = 0, jj = 0, constantBufferPerSet = 0; ii < handle->pushRangeCount && jj < handle->resourceSetCount;)
				{
					ShaderResourceManager* descriptorManager = descriptorManagers.Get(handle->resourceSets[jj].descriptorManagerIndex);

					ShaderResourceConstantBuffer* pushArgs = (ShaderResourceConstantBuffer*)descriptorManager->GetConstantBuffer(handle->resourceSets[jj].descriptorSetIndex, constantBufferPerSet++);
					
					if (!pushArgs)
					{
						jj++;
						constantBufferPerSet = 0;
						continue;
					}

					PushConstantsCmd(&recorder, pushArgs->offset, pushArgs->size, pushArgs->stage, pushArgs->data);

					ii++;
				}

				InsertIntraPassBarrier(&recorder, pipelineIndex);

				if (AllocationInstanceIndex() != handle->indirectDispatchCommandHandle)
				{
					RenderAllocation* indirectBufferAlloc = allocations.Get(handle->indirectDispatchCommandHandle);

					size_t indirectBufferBaseOffset = 0;

					GetAllocationDetails(indirectBufferAlloc, nullptr, &indirectBufferBaseOffset, nullptr, currentFrame);

					DispatchIndirectCmd(&recorder, bufferHandles[indirectBufferAlloc->memIndex].bufferHandle, indirectBufferBaseOffset);
				}
				else
				{
					DispatchCmd(&recorder, handle->x, handle->y, handle->z);
				}
			}

			ResetIntraBarrierAccumulator(accumulator);
			
			WriteDeviceQuery(&recorder, StageBits::COMPUTE_BARRIER);
		}
		else if (command->streamType == GPUCommandStreamType::ATTACHMENT_COMMANDS)
		{
			AttachmentGraphInstance* currentGraphInstance = attachmentGraphsInstances.Get(command->commandIndex.attachmentGraphIndex);

			for (int i = 0; i < currentGraphInstance->graphLayout->passesCount; i++)
			{
				WriteDeviceQuery(&recorder, StageBits::BEGINNING_OF_PIPE);

				AttachmentRenderPassInstance* rpInst = &currentGraphInstance->passes[i];

				int SubRenderTargetSelection = rpInst->rpType == RenderPassType::SWAPCHAIN_IMAGE_COUNT ? imageIndex : currentFrame;

				int sampleLevelForRenderPass = rpInst->currentSampleCount;

				PipelineQueueIndex possibleQueueIndex = rpInst->graphicsOTQIndex;

				RenderTargetInfo* rtInfo = mainRenderTargets.Get(rpInst->baseRenderTarget[sampleLevelForRenderPass]);

				RenderTarget* renderTarget = dev->GetRenderTarget(rtInfo->driverRenderTargetInfo);

				VkClearValue* clears = (VkClearValue*)cacheAllocator->Allocate(sizeof(VkClearValue) * rpInst->attachInstCount);

				AttachmentInstance* instances = rpInst->attachInst;

				uint32_t clearCount = rpInst->attachInstCount;

				for (int g = 0; g < rpInst->attachInstCount; g++)
				{
					VkClearValue* currClear = &clears[g];
					switch (instances[g].clear.type)
					{
					case NOCLEAR:
						break;
					case CLEARCOLOR:
						currClear->color.float32[0] = instances[g].clear.val.cdata[0];
						currClear->color.float32[1] = instances[g].clear.val.cdata[1];
						currClear->color.float32[2] = instances[g].clear.val.cdata[2];
						currClear->color.float32[3] = instances[g].clear.val.cdata[3];
						break;
					case CLEARDEPTH:
						currClear->depthStencil.depth = instances[g].clear.val.ddata;
						currClear->depthStencil.stencil = instances[g].clear.val.sdata;
						break;
					}
				}

				if (PipelineQueueIndex() != possibleQueueIndex)
				{
					RenderQueue* queue = renderTargetQueues.Get(possibleQueueIndex);

					for (uint32_t pipeInst = 0; pipeInst < queue->queueCount; pipeInst++)
					{
						PipelineHandleIndex pipelineIndex = queue->pipelines[pipeInst];

						PipelineHandle* handle = pipelineHandles.Get(pipelineIndex);

						GeneratePipelineDescriptorBarriers(&recorder, handle->resourceSets, handle->resourceSetCount, pipelineIndex);

						GenerateDrawBindingsBarriers(&recorder, handle);
					}

					InsertAccumulatedBarriers(&recorder);
				}

				BeginRenderPassCmd(&recorder, rtInfo->driverRenderTargetInfo, SubRenderTargetSelection, VK_SUBPASS_CONTENTS_INLINE, { {0, 0}, {renderTarget->width, renderTarget->height} }, clears, clearCount);

				float x = static_cast<float>(renderTarget->width), y = static_cast<float>(renderTarget->height);

				float xOff = static_cast<float>(renderTarget->wOffset), yOff = static_cast<float>(renderTarget->hOffset);

				SetViewportCmd(&recorder, xOff, yOff, x, y, 0.0f, 1.0f);

				SetScissorCmd(&recorder, renderTarget->wOffset, renderTarget->hOffset, renderTarget->width, renderTarget->height);

				if (PipelineQueueIndex() != possibleQueueIndex)
				{
					RenderQueue* queue = renderTargetQueues.Get(possibleQueueIndex);

					for (uint32_t pipeInst = 0; pipeInst < queue->queueCount; pipeInst++)
					{
						PipelineHandleIndex pipelineIndex = queue->pipelines[pipeInst];

						PipelineHandle* handle = pipelineHandles.Get(pipelineIndex);

						GraphPipelineDescription* pipeDesc = graphPipelineDescriptions.Get(handle->pipelineIdentifierGroup);

						PipelineInstanceData* pid = &pipeDesc->instanceData;

						uint32_t pipelineOffset = 0;

						for (int i = 0; i < pid->frameGraphCount; i++)
						{
							if (command->commandIndex.attachmentGraphIndex == pid->frameGraphIndices[i].index)
							{
								pipelineOffset = pid->frameGraphPipelineIndices[i];
								break;
							}
						}

						EntryHandle pipelineTemp = pipeDesc->pipelineIndices[pipelineOffset + sampleLevelForRenderPass];

						BindGraphicsPipelineCmd(&recorder, pipelineTemp);

						for (uint32_t ii = 0; ii < handle->resourceSetCount; ii++)
						{
							ShaderResourceManager* descriptorManager = descriptorManagers.Get(handle->resourceSets[ii].descriptorManagerIndex);

							BindGraphicsDescriptorSetsCmd(&recorder, descriptorManager->descriptorSetHandles[handle->resourceSets[ii].descriptorSetIndex], currentFrame, 1, ii, 0, nullptr);
						}

						uint32_t vertexCount = handle->vertexCount;

						uint32_t indexCount = handle->indexCount;

						size_t vertexOffset = -1, indexOffset = -1, indirectBufferBaseOffset = -1, indirectCountBufferBaseOffset = -1;

						BufferMemoryIndex vertexMemIndex{}, indexMemIndex{}, indirectBufferIndex{}, indirectCountBufferIndex{};

						if (AllocationInstanceIndex() != handle->vertexBufferHandle)
						{		
							RenderAllocation* vertexAlloc = allocations.Get(handle->vertexBufferHandle);

							vertexMemIndex = vertexAlloc->memIndex;

							GetAllocationDetails(vertexAlloc, nullptr, &vertexOffset, nullptr, currentFrame);

							BindVertexBufferCmd(&recorder, bufferHandles[vertexMemIndex].bufferHandle, 0, 1, &vertexOffset);
						}

						if (AllocationInstanceIndex() != handle->indexBufferHandle)
						{			
							RenderAllocation* indexAlloc = allocations.Get(handle->indexBufferHandle);

							indexMemIndex = indexAlloc->memIndex;

							GetAllocationDetails(indexAlloc, nullptr, &indexOffset, nullptr, currentFrame);

							BindIndexBufferCmd(&recorder, bufferHandles[indexMemIndex].bufferHandle, indexOffset, handle->indexSize);
						}

						if (AllocationInstanceIndex() != handle->indirectBufferHandle)
						{
							RenderAllocation* indirectBufferAlloc = allocations.Get(handle->indirectBufferHandle);

							GetAllocationDetails(indirectBufferAlloc, nullptr, &indirectBufferBaseOffset, nullptr, currentFrame);

							indirectBufferIndex = indirectBufferAlloc->memIndex;
						}

						if (AllocationInstanceIndex() != handle->indirectCountBufferHandle)
						{
							RenderAllocation* indirectCountBufferAlloc = allocations.Get(handle->indirectCountBufferHandle);

							GetAllocationDetails(indirectCountBufferAlloc, nullptr, &indirectCountBufferBaseOffset, nullptr, currentFrame);

							indirectCountBufferIndex = indirectCountBufferAlloc->memIndex;
						}

						for (uint32_t ii = 0, jj = 0, constantBufferPerSet = 0; ii < handle->pushRangeCount && jj < handle->resourceSetCount;)
						{
							ShaderResourceManager* descriptorManager = descriptorManagers.Get(handle->resourceSets[ii].descriptorManagerIndex);

							ShaderResourceConstantBuffer* pushArgs = (ShaderResourceConstantBuffer*)descriptorManager->GetConstantBuffer(handle->resourceSets[jj].descriptorSetIndex, constantBufferPerSet++);
							
							if (!pushArgs)
							{
								jj++;
								constantBufferPerSet = 0;
								continue;
							}
					
							PushConstantsCmd(&recorder, pushArgs->offset, pushArgs->size, pushArgs->stage, pushArgs->data);

							ii++;
						}

						if (AllocationInstanceIndex() != handle->indirectBufferHandle)
						{
							if (BufferMemoryIndex() != indexMemIndex)
							{
								if (AllocationInstanceIndex() == handle->indirectCountBufferHandle)
								{
									DrawIndexedIndirectCountCmd(&recorder,
										bufferHandles[indirectBufferIndex].bufferHandle, 
										bufferHandles[indirectCountBufferIndex].bufferHandle, 
										indirectBufferBaseOffset, 
										indirectCountBufferBaseOffset, 
										handle->indirectDrawCount);
								}
								else
								{
									DrawIndexedIndirectCmd(&recorder, bufferHandles[indirectBufferIndex].bufferHandle, handle->indirectDrawCount, indirectBufferBaseOffset);
								}
							}
							else
							{
								if (AllocationInstanceIndex() != handle->indirectCountBufferHandle)
								{
									DrawIndirectCountCmd(&recorder,
										bufferHandles[indirectBufferIndex].bufferHandle,
										bufferHandles[indirectCountBufferIndex].bufferHandle,
										indirectBufferBaseOffset,
										indirectCountBufferBaseOffset,
										handle->indirectDrawCount);
								}
								else
								{
									DrawIndirectCmd(&recorder, bufferHandles[indirectBufferIndex].bufferHandle, handle->indirectDrawCount, indirectBufferBaseOffset);
								}
							}
						}
						else
						{
							if (BufferMemoryIndex() != indexMemIndex)
							{
								DrawIndexedCmd(&recorder, indexCount, handle->instanceCount, 0, 0, 0);
							}
							else
							{
								DrawCmd(&recorder, 0, vertexCount, 0, handle->instanceCount);
							}
						}
					}
				}

				EndRenderPassCmd(&recorder);

				WriteDeviceQuery(&recorder, StageBits::END_OF_PIPE);
			}
		}

		commandCountIter++;
	}

	ReturnBarrierAccumulator(accumulatorIndex);

	EndCommandRecording(&recorder);
}

void RenderInstance::IncreaseMSAA(AttachmentGraphInstanceIndex& frameGraph, int renderPassIndex)
{
	AttachmentGraphInstance* graphInstance = attachmentGraphsInstances.Get(frameGraph);

	if (graphInstance)
	{
		if (renderPassIndex < graphInstance->graphLayout->passesCount)
		{
			AttachmentRenderPassInstance* passInstance = &graphInstance->passes[renderPassIndex];

			int next = passInstance->currentSampleCount + 1;

			if (next < passInstance->maxSampleCount)
				passInstance->currentSampleCount = next;
		}
	}
}

void RenderInstance::DecreaseMSAA(AttachmentGraphInstanceIndex& frameGraph, int renderPassIndex)
{
	AttachmentGraphInstance* graphInstance = attachmentGraphsInstances.Get(frameGraph);

	if (graphInstance)
	{
		if (renderPassIndex < graphInstance->graphLayout->passesCount)
		{
			AttachmentRenderPassInstance* passInstance = &graphInstance->passes[renderPassIndex];

			int next = passInstance->currentSampleCount - 1;

			if (next >= 0)
				passInstance->currentSampleCount = next;
		}
	}
}

void RenderInstance::ResetCommandList(GPUCommandStreamIndex& commandStreamIndex)
{
	GPUCommandStreamAllocator* stream = gpuCommandStreams.Get(commandStreamIndex);

	if (stream)
	{
		stream->commandCount = 0;
		return;
	}

	internalRendererLogger->AddLogMessage(LOGERROR, STRING_VIEW_FROM_LITERAL("ResetCommandList: invalid command stream index"));
}

void RenderInstance::CreateGraphicsQueueForAttachments(AttachmentGraphInstanceIndex& frameGraphIndex, int renderPassIndex)
{
	AttachmentGraphInstance* graphInstance = attachmentGraphsInstances.Get(frameGraphIndex);

	AttachmentRenderPassInstance* passInstance = &graphInstance->passes[renderPassIndex];

	passInstance->graphicsOTQIndex = renderTargetQueues.Allocate();

	if (PipelineQueueIndex() != passInstance->graphicsOTQIndex)
	{
		return;
	}

	CleanInitializeRenderTargetQueue(renderTargetQueues.Get(passInstance->graphicsOTQIndex));
}

PipelineQueueIndex RenderInstance::CreateComputeQueue()
{
	PipelineQueueIndex computeQueue = computeQueues.Allocate();

	if (PipelineQueueIndex() != computeQueue)
	{
		CleanInitializeComputeQueue(computeQueues.Get(computeQueue));
	}

	return computeQueue;
}

void RenderInstance::AddComputeCommandQueue(GPUCommandStreamIndex& commandStreamIndex, PipelineQueueIndex& handleIndex)
{
	GPUCommandStreamAllocator* stream = gpuCommandStreams.Get(commandStreamIndex);

	if (stream)
	{
		if (stream->commandCount == stream->maxCommandCount)
		{
			internalRendererLogger->AddLogMessage(LOGERROR, STRING_VIEW_FROM_LITERAL("AddCommandQueue: current count is at max"));
			return;
		}

		GPUCommand* command = &stream->commands[stream->commandCount++];

		command->commandIndex.indexForComputeQueue = handleIndex;
		command->streamType = GPUCommandStreamType::COMPUTE_QUEUE_COMMANDS;

		return;
	}

	internalRendererLogger->AddLogMessage(LOGERROR, STRING_VIEW_FROM_LITERAL("AddCommandQueue: invalid command stream index"));
}

void RenderInstance::AddAttachmentCommandQueue(GPUCommandStreamIndex& commandStreamIndex, AttachmentGraphInstanceIndex& handleIndex)
{
	GPUCommandStreamAllocator* stream = gpuCommandStreams.Get(commandStreamIndex);

	if (stream)
	{
		if (stream->commandCount == stream->maxCommandCount)
		{
			internalRendererLogger->AddLogMessage(LOGERROR, STRING_VIEW_FROM_LITERAL("AddCommandQueue: current count is at max"));
			return;
		}

		GPUCommand* command = &stream->commands[stream->commandCount++];

		command->commandIndex.attachmentGraphIndex = handleIndex;
		command->streamType = GPUCommandStreamType::ATTACHMENT_COMMANDS;

		return;
	}

	internalRendererLogger->AddLogMessage(LOGERROR, STRING_VIEW_FROM_LITERAL("AddCommandQueue: invalid command stream index"));
}

GPUCommandStreamIndex RenderInstance::CreateGPUCommandStream(int maxGPUCommandCount)
{
	GPUCommandStreamIndex gpuCommandsIndex = gpuCommandStreams.Allocate();

	if (GPUCommandStreamIndex() == gpuCommandsIndex)
	{
		return gpuCommandsIndex;
	}

	GPUCommandStreamAllocator* stream = gpuCommandStreams.Get(gpuCommandsIndex);

	CleanInitializeGpuCommandStream(stream);

	stream->maxCommandCount = maxGPUCommandCount;
	stream->commands = (GPUCommand*)AllocateFromStorageAllocator(sizeof(GPUCommand) * maxGPUCommandCount);

	if (!stream->commands)
	{
		DestroyGpuCommandStream(gpuCommandsIndex);
		return {};
	}

	return gpuCommandsIndex;
}

void RenderInstance::EndFrame(RenderDeviceIndex deviceSelection, GPUCommandStreamIndex& commandStreamIndex)
{
	char StringBuffer[512];

	RHIDevice* rhiDevice = GetDeviceHandle(deviceSelection);

	VKDevice* dev = rhiDevice->device;

	int commandCountIter = 0;

	GPUCommandStreamAllocator* stream = gpuCommandStreams.Get(commandStreamIndex);

	if (rhiDevice->container.queriesAreActive)
	{
		int queryOffset = 0;

		while (commandCountIter < stream->commandCount)
		{
			GPUCommand* command = &stream->commands[commandCountIter];

			const char* passDesc = "Undefined pass : ";

			int queryCount = 0;

			if (command->streamType == GPUCommandStreamType::COMPUTE_QUEUE_COMMANDS)
			{
				passDesc = "Compute Pass : ";

				queryCount = 2;
			}
			else if (command->streamType == GPUCommandStreamType::ATTACHMENT_COMMANDS)
			{
				AttachmentGraphInstance* currentGraphInstance = attachmentGraphsInstances.Get(command->commandIndex.attachmentGraphIndex);

				queryCount = (2 * currentGraphInstance->graphLayout->passesCount);

				passDesc = "Render Pass : ";
			}

			if (previousFrame < MAX_FRAMES_IN_FLIGHT && (rhiDevice->container.queryCounts[previousFrame] >= queryOffset + queryCount))
			{
				dev->ReadbackResultsFromQueries(
					rhiDevice->container.queryPoolIndex,
					(rhiDevice->container.maxQueryResults * previousFrame) + queryOffset,
					queryCount,
					rhiDevice->container.queryResults,
					sizeof(uint32_t) * rhiDevice->container.maxQueryResults,
					sizeof(uint32_t),
					VK_QUERY_RESULT_WAIT_BIT
				);

				for (uint32_t i = 0; i < queryCount; i += 2)
				{
					double timeNs = (rhiDevice->container.queryResults[i + 1] - rhiDevice->container.queryResults[i]) * rhiDevice->container.relatedPhysDeviceInfo->deviceTimeStampPeriodNS;

					double timeMs = timeNs / 1e6;

					int actualSize = snprintf(StringBuffer, 512, "%s Time to run pass: %lf", passDesc, timeMs);

					internalRendererLogger->AddLogMessage(LOGINFO, StringBuffer, actualSize);
				}
			}

			queryOffset += queryCount;

			commandCountIter++;
		}

		if (previousFrame < MAX_FRAMES_IN_FLIGHT)
		{
			rhiDevice->container.queryCounts[previousFrame] = 0;
		}

		commandCountIter = 0;
	}

	while (commandCountIter < stream->commandCount)
	{
		GPUCommand* command = &stream->commands[commandCountIter++];

		if (command->streamType == GPUCommandStreamType::COMPUTE_QUEUE_COMMANDS)
		{
			ComputeQueue* computeQueue = computeQueues.Get(command->commandIndex.indexForComputeQueue);

			computeQueue->queueCount = 0;
		}
		else if (command->streamType == GPUCommandStreamType::ATTACHMENT_COMMANDS)
		{
			AttachmentGraphInstance* currentGraphInstance = attachmentGraphsInstances.Get(command->commandIndex.attachmentGraphIndex);

			for (int i = 0; i < currentGraphInstance->graphLayout->passesCount; i++)
			{
				if (PipelineQueueIndex() != currentGraphInstance->passes[i].graphicsOTQIndex)
				{
					RenderQueue* queue = renderTargetQueues.Get(currentGraphInstance->passes[i].graphicsOTQIndex);

					queue->queueCount = 0;
				}
			}
		}
	}

	cacheAllocator->Reset();

	previousFrame = currentFrame;
	currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void RenderInstance::WriteDeviceQuery(CommandRecorder* recorder, PipelineStage stage)
{
	if (recorder->device->container.queriesAreActive)
	{
		int queryBase = recorder->device->container.maxQueryResults * currentFrame;

		WriteTimeStamp(recorder, recorder->device->container.queryPoolIndex, recorder->device->container.queryCounts[currentFrame] + queryBase, stage);

		recorder->device->container.queryCounts[currentFrame] += 1;
	}
}

void RenderInstance::ToggleDeviceQueries(RenderDeviceIndex mainDeviceSelection)
{
	RHIDevice* device = GetDeviceHandle(mainDeviceSelection);

	device->container.queriesAreActive ^= 1;
}

int RenderInstance::AddPipelineToRPGraphicsQueue(PipelineHandleIndex& psoIndex, AttachmentGraphInstanceIndex& frameGraphIndex, int renderPass)
{
	AttachmentGraphInstance* currentGraphInstance = attachmentGraphsInstances.Get(frameGraphIndex);

	if (currentGraphInstance)
	{
		if (renderPass < currentGraphInstance->graphLayout->passesCount)
		{
			AttachmentRenderPassInstance* rendPassInst = &currentGraphInstance->passes[renderPass];

			if (PipelineQueueIndex() != rendPassInst->graphicsOTQIndex)
			{
				RenderQueue* queue = renderTargetQueues.Get(rendPassInst->graphicsOTQIndex);

				if (queue->queueCount < MAX_QUEUE_ENTRIES)
				{
					queue->pipelines[queue->queueCount++] = psoIndex;

					return 0;
				}
			}
			else
			{
				internalRendererLogger->AddLogMessage(LOGERROR, STRING_VIEW_FROM_LITERAL("AddPipelineToRPGraphicsQueue : no graphics queue created for this render pass"));
			}
		}
	}

	internalRendererLogger->AddLogMessage(LOGERROR, STRING_VIEW_FROM_LITERAL("AddPipelineToRPGraphicsQueue : incorrect parameter passed to function"));

	return -1;
}

int RenderInstance::AddPipelineToComputeQueue(PipelineQueueIndex& queueIndex, PipelineHandleIndex& psoIndex)
{
	ComputeQueue* queue = computeQueues.Get(queueIndex);

	if (queue)
	{
		if (queue->queueCount < MAX_QUEUE_ENTRIES)
		{
			queue->pipelines[queue->queueCount++] = psoIndex;
			return 0;
		}
	}

	internalRendererLogger->AddLogMessage(LOGERROR, STRING_VIEW_FROM_LITERAL("AddPipelineToComputeQueue : incorrect parameter passed to function"));

	return -1;
}

int RenderInstance::ReadData(AllocationInstanceIndex& handle, void* dest, int size, int offset)
{
	RenderAllocation* allocation = allocations.Get(handle);

	if (!allocation)
	{
		return -1;
	}

	RHIDevice* rhiDevice = GetDeviceHandle(allocation->deviceIndex);

	VKDevice* dev = rhiDevice->device;

	size_t allocOffset = 0;

	MemoryType type = HOST_MEMORY_TYPE;

	type = bufferHandles[allocation->memIndex].type;

	if (!((type & MemoryTypeBits::HOST_MEMORY_TYPE) || (type & MemoryTypeBits::HOST_MEMORY_COHERENT_TYPE)))
	{
		internalRendererLogger->AddLogMessage(LOGERROR, STRING_VIEW_FROM_LITERAL("ReadData: Non readable memory from CPU"));
		return -1;
	}

	allocOffset = allocation->offset;

	EntryHandle index = bufferHandles[allocation->memIndex].bufferHandle;

	int readRet = dev->ReadHostBuffer(dest, index, size, allocOffset+offset);

	if (readRet)
	{
		GetLastDeviceDriverError(rhiDevice, STRING_VIEW_FROM_LITERAL("ReadData: driver read failed"));
	}

	return readRet;
}

int RenderInstance::UpdateDriverMemory(void* data, AllocationInstanceIndex& allocationIndex, int size, int allocOffset, TransferType transferType)
{
	RenderAllocation* alloc = allocations.Get(allocationIndex);

	if (!alloc)
	{
		return -1;
	}

	int oneStrideSize = alloc->requestedSize * alloc->structureCopies;

	if (oneStrideSize <= allocOffset  || oneStrideSize < size)
	{
		internalRendererLogger->AddLogMessage(LOGERROR, STRING_VIEW_FROM_LITERAL("UpdateDriverMemory : Bad arguments to function"));
		return -1;
	}

	void* outData = data;

	int copies = 1;

	RenderDriverUpdateCommandMemory* rducm = (RenderDriverUpdateCommandMemory*)updateCommandBuffers[currentUpdateCommandBuffer]->Allocate(sizeof(RenderDriverUpdateCommandMemory));

	if (!rducm)
	{
		return -1;
	}

	if (transferType == TransferType::CACHED)
	{
		outData = updateCommandsCache->Allocate(size, 16);
		memcpy(outData, data, size);
	}

	if (alloc->allocType == AllocationType::PERFRAME)
	{
		copies = MAX_FRAMES_IN_FLIGHT;
	}

	rducm->allocationIndex = allocationIndex;
	rducm->allocOffset = allocOffset;
	rducm->copiesWithin = copies;
	rducm->size = size;
	rducm->data = outData;
	rducm->updateType = DriverUpdateType::MEMORYUPDATE;

	return 0;
}

int RenderInstance::UpdateImageMemory(void* data, TextureIndex& textureIndex, size_t totalSize, int width, int height, int mipLevels, int mipStart, int layerCount, int layerStart, ImageViewAspectMask mask)
{
	if (!textureResourceHandles.Get(textureIndex))
	{
		return -1;
	}

	RenderDriverUpdateCommandImage* rduci = (RenderDriverUpdateCommandImage*)updateCommandBuffers[currentUpdateCommandBuffer]->Allocate(sizeof(RenderDriverUpdateCommandImage));

	if (!rduci)
	{
		return -1;
	}

	rduci->data = data;
	rduci->height = height;
	rduci->mipLevels = mipLevels;
	rduci->width = width;
	rduci->layersCount = layerCount;
	rduci->textureIndex = textureIndex;
	rduci->updateType = DriverUpdateType::IMAGEMEMORYUPDATE;
	rduci->totalSize = totalSize;
	rduci->mask = mask;
	rduci->mipStart = mipStart;
	rduci->layerStart = layerStart;

	return 0;
}

int RenderInstance::InsertTransferCommand(AllocationInstanceIndex& allocationIndex, int size, int allocOffset, uint32_t fillValue)
{
	RenderAllocation* alloc = allocations.Get(allocationIndex);

	if (!alloc)
	{
		return -1;
	}

	int oneStrideSize = alloc->requestedSize * alloc->structureCopies;

	if (oneStrideSize <= allocOffset || oneStrideSize < size)
	{
		internalRendererLogger->AddLogMessage(LOGERROR, STRING_VIEW_FROM_LITERAL("UpdateDriverMemory : Bad arguments to function"));
		return -1;
	}

	RenderDriverUpdateCommandFill* rducf = (RenderDriverUpdateCommandFill*)updateCommandBuffers[currentUpdateCommandBuffer]->Allocate(sizeof(RenderDriverUpdateCommandFill));

	if (!rducf)
	{
		return -1;
	}

	int copies = 1;

	if (alloc->allocType == AllocationType::PERFRAME)
	{
		copies = MAX_FRAMES_IN_FLIGHT;
	}

	rducf->allocationIndex = allocationIndex;
	rducf->allocOffset = allocOffset;
	rducf->fillValue = fillValue;
	rducf->size = size;
	rducf->updateType = DriverUpdateType::TRANSFERCOMMAND;
	rducf->copiesWithin = copies;

	return 0;
}

int RenderInstance::UpdateShaderResourceArray(ShaderResourceSetHandle handle, int bindingindex, ShaderResourceType type, DeviceHandleArrayUpdate* resourceArrayData)
{
	ShaderResourceManager* descriptorManager = descriptorManagers.Get(handle.descriptorManagerIndex);

	if (!descriptorManager)
	{
		return -1;
	}

	ShaderResourceSet* set = descriptorManager->descriptorSets[handle.descriptorSetIndex];

	int argSize = 0;

	void* argData = nullptr;

	int resCount = resourceArrayData->resourceCount;

	switch (type)
	{
	case ShaderResourceType::SAMPLER3D:
	case ShaderResourceType::SAMPLER2D:
	case ShaderResourceType::SAMPLERSTATE:
	case ShaderResourceType::IMAGE2D:
	case ShaderResourceType::SAMPLERCUBE:
	{
		DeviceHandleArrayUpdate* cachedUpdate = (DeviceHandleArrayUpdate*)(updateCommandsCache->Allocate(sizeof(DeviceHandleArrayUpdate)));

		int sizeOfUnderLyingStruct = sizeof(int);

		switch (resourceArrayData->updateType)
		{
		case DeviceHandleArrayUpdateType::TEXTURE_VIEW_UPDATE:
			sizeOfUnderLyingStruct = sizeof(DeviceHandleArrayUpdateTextureView);
			break;
		case DeviceHandleArrayUpdateType::TEXTURE_VIEW_SAMPLER_UPDATE:
			sizeOfUnderLyingStruct = sizeof(DeviceHandleArrayUpdateTextureViewSampler);
			break;
		case DeviceHandleArrayUpdateType::SAMPLER_UPDATE:
		default:
			break;
		}
		
		cachedUpdate->updateType = resourceArrayData->updateType;
		cachedUpdate->resourceDstBegin = resourceArrayData->resourceDstBegin;
		cachedUpdate->resourceCount = resCount;
		cachedUpdate->resourceHandles = updateCommandsCache->Allocate(sizeOfUnderLyingStruct * resCount);
		
		memcpy(cachedUpdate->resourceHandles, resourceArrayData->resourceHandles, sizeOfUnderLyingStruct * resCount);
		
		argData = cachedUpdate;
		argSize = (sizeOfUnderLyingStruct * resCount) + sizeof(DeviceHandleArrayUpdate);
		break;
	}
	default:
		internalRendererLogger->AddLogMessage(LOGERROR, STRING_VIEW_FROM_LITERAL("UpdateShaderResourceArray : Invalid Descriptor type passed"));
		return -1;
	}

	RenderDriverUpdateCommandResource* rducr = (RenderDriverUpdateCommandResource*)updateCommandBuffers[currentUpdateCommandBuffer]->Allocate(sizeof(RenderDriverUpdateCommandResource));

	if (!rducr)
	{
		return -1;
	}

	rducr->bindingindex = bindingindex;
	rducr->updateType = DriverUpdateType::RESOURCEUPDATE;
	rducr->descriptorIdManagerIndex = PACK_DESCRIPTOR_MANAGER_INDEX(handle.descriptorManagerIndex.index) | PACK_DESCRIPTOR_SET_INDEX(handle.descriptorSetIndex);
	rducr->type = type;
	rducr->cachedDataSize = argSize;
	rducr->data = argData;
	rducr->copies = set->setCount;

	return 0;
}


int RenderInstance::UpdateBufferResourceArray(ShaderResourceSetHandle handle, int bindingindex, ShaderResourceType type, BufferArrayUpdate* resourceArrayData)
{
	ShaderResourceManager* descriptorManager = descriptorManagers.Get(handle.descriptorManagerIndex);

	if (!descriptorManager)
	{
		return -1;
	}

	ShaderResourceSet* set = descriptorManager->descriptorSets[handle.descriptorSetIndex];

	int argSize = 0;

	void* argData = nullptr;

	int resCount = resourceArrayData->allocationCount;

	switch (type)
	{
	case ShaderResourceType::STORAGE_BUFFER:
	case ShaderResourceType::UNIFORM_BUFFER:
	case ShaderResourceType::BUFFER_VIEW:
	{
		BufferArrayUpdate* cachedUpdate = (BufferArrayUpdate*)(updateCommandsCache->Allocate(sizeof(BufferArrayUpdate)));

		cachedUpdate->resourceDstBegin = resourceArrayData->resourceDstBegin;
		cachedUpdate->allocationCount = resCount;
		cachedUpdate->allocationIndices = (AllocationInstanceIndex*)(updateCommandsCache->Allocate(sizeof(AllocationInstanceIndex) * resCount));

		memcpy(cachedUpdate->allocationIndices, resourceArrayData->allocationIndices, sizeof(int) * resCount);

		argData = cachedUpdate;
		argSize = (sizeof(int) * resCount) + sizeof(BufferArrayUpdate);
		break;
	}
	default:
		internalRendererLogger->AddLogMessage(LOGERROR, STRING_VIEW_FROM_LITERAL("UpdateBufferResourceArray : Invalid Descriptor type passed"));
		return -1;
	}

	RenderDriverUpdateCommandResource* rducr = (RenderDriverUpdateCommandResource*)updateCommandBuffers[currentUpdateCommandBuffer]->Allocate(sizeof(RenderDriverUpdateCommandResource));

	if (!rducr)
	{
		return -1;
	}

	rducr->bindingindex = bindingindex;
	rducr->updateType = DriverUpdateType::RESOURCEUPDATE;
	rducr->descriptorIdManagerIndex = PACK_DESCRIPTOR_MANAGER_INDEX(handle.descriptorManagerIndex.index) | PACK_DESCRIPTOR_SET_INDEX(handle.descriptorSetIndex);
	rducr->type = type;
	rducr->cachedDataSize = argSize;
	rducr->data = argData;
	rducr->copies = set->setCount;

	return 0;
}

void RenderInstance::SwapUpdateCommands()
{
	int drainBuffer = currentUpdateCommandBuffer;

	currentUpdateCommandBuffer = (currentUpdateCommandBuffer ^ 1);

	RenderDriverUpdateCommandHeader* header = (RenderDriverUpdateCommandHeader*)updateCommandBuffers[drainBuffer]->dataHead;

	size_t currentSize = updateCommandBuffers[drainBuffer]->dataAllocator.load();

	while (currentSize)
	{
		switch (header->updateType)
		{
		case DriverUpdateType::RESOURCEUPDATE:
		{
			RenderDriverUpdateCommandResource* rducr = (RenderDriverUpdateCommandResource*)header;

			int descriptorManager = UNPACK_DESCRIPTOR_MANAGER_INDEX(rducr->descriptorIdManagerIndex);

			int descriptorId = UNPACK_DESCRIPTOR_SET_INDEX(rducr->descriptorIdManagerIndex);

			descriptorUpdatePool.Create(descriptorManager, descriptorId, rducr->bindingindex, rducr->type, rducr->data, rducr->cachedDataSize, rducr->copies);
			header = rducr->GetNext();
			currentSize -= sizeof(RenderDriverUpdateCommandResource);
			break;
		}
		case DriverUpdateType::TRANSFERCOMMAND:
		{
			RenderDriverUpdateCommandFill* rducf = (RenderDriverUpdateCommandFill*)header;
			transferCommandPool.Create(rducf->allocationIndex,  rducf->size, rducf->allocOffset, rducf->fillValue, rducf->copiesWithin);
			header = rducf->GetNext();
			currentSize -= sizeof(RenderDriverUpdateCommandFill);
			break;
		}
		case DriverUpdateType::IMAGEMEMORYUPDATE:
		{
			RenderDriverUpdateCommandImage* rduci = (RenderDriverUpdateCommandImage*)header;
			imageMemoryUpdateManager.Create(rduci->data, rduci->textureIndex, rduci->totalSize, rduci->width, rduci->height, rduci->mipLevels, rduci->layersCount, rduci->mipStart, rduci->layerStart, rduci->mask);
			header = rduci->GetNext();
			currentSize -= sizeof(RenderDriverUpdateCommandImage);
			break;
		}
		case DriverUpdateType::MEMORYUPDATE:
		{
			RenderDriverUpdateCommandMemory* rducm = (RenderDriverUpdateCommandMemory*)header;

			RenderAllocation* alloc = allocations.Get(rducm->allocationIndex);

			MemoryType bufType = bufferHandles[alloc->memIndex].type;

			if ((bufType & MemoryTypeBits::HOST_MEMORY_TYPE) || (bufType & MemoryTypeBits::HOST_MEMORY_COHERENT_TYPE))
			{
				driverHostMemoryUpdater.Create(rducm->data, rducm->size, rducm->allocationIndex, rducm->allocOffset, rducm->copiesWithin);
			}
			else if (bufType & MemoryTypeBits::DEVICE_MEMORY_TYPE)
			{
				driverDeviceMemoryUpdater.Create(rducm->data, rducm->size, rducm->allocationIndex, rducm->allocOffset, rducm->copiesWithin);
			}

			header = rducm->GetNext();
			currentSize -= sizeof(RenderDriverUpdateCommandMemory);

			break;
		}
		}
	}

	updateCommandBuffers[drainBuffer]->Reset();
}


int RenderInstance::UploadFrameAttachmentResource(AttachmentGraphInstanceIndex& frameGraph, int resourceIndex, int perTextureViewIndex, ShaderResourceSetHandle handle, int bindingIndex, int textureStart)
{
	AttachmentGraphInstance* currentGraphInstance = attachmentGraphsInstances.Get(frameGraph);

	if (currentGraphInstance)
	{
		if (resourceIndex < currentGraphInstance->graphLayout->resourceCount)
		{
			int imageCount = currentGraphInstance->resources[resourceIndex].imageCount;

			DeviceHandleArrayUpdateTextureView* textureIds = (DeviceHandleArrayUpdateTextureView*)cacheAllocator->Allocate(sizeof(DeviceHandleArrayUpdateTextureView) * imageCount);

			for (int i = 0; i < imageCount; i++)
			{
				TextureIndex textureIndex = currentGraphInstance->resources[resourceIndex].textureIds[0][i];
				textureIds[i].imageHandle = textureIndex;
				textureIds[i].viewIndex = perTextureViewIndex;
			}

			DeviceHandleArrayUpdate update;

			update.updateType = DeviceHandleArrayUpdateType::TEXTURE_VIEW_UPDATE;
			update.resourceCount = imageCount;
			update.resourceDstBegin = textureStart;
			update.resourceHandles = textureIds;

			UpdateShaderResourceArray(handle, bindingIndex, ShaderResourceType::IMAGE2D, &update);

			return 0;
		}

		internalRendererLogger->AddLogMessage(LOGERROR, STRING_VIEW_FROM_LITERAL("UploadFrameAttachmentResource : resource index exceed graph resource count"));
	}

	return -1;
}

void RenderInstance::PipelineUpdateIndirectCommandBuffer(PipelineHandleIndex& pipelineIndex, AllocationInstanceIndex& allocationIndex)
{
	PipelineHandle* handle = pipelineHandles.Get(pipelineIndex);

	if (handle)
	{
		if (handle->group == GRAPHICSO)
		{
			handle->indirectBufferHandle = allocationIndex;
		}
	}
}

void RenderInstance::PipelineUpdateVertexBuffer(PipelineHandleIndex& pipelineIndex, AllocationInstanceIndex& allocationIndex, uint32_t vertexCount)
{
	PipelineHandle* handle = pipelineHandles.Get(pipelineIndex);

	if (handle)
	{
		if (handle->group == GRAPHICSO)
		{
			handle->vertexBufferHandle = allocationIndex;
			handle->vertexCount = vertexCount;
		}
	}
}

void RenderInstance::PipelineUpdateIndexBuffer(PipelineHandleIndex& pipelineIndex, AllocationInstanceIndex& allocationIndex, uint32_t indexCount, uint32_t indexStride)
{
	PipelineHandle* handle = pipelineHandles.Get(pipelineIndex);

	if (handle)
	{
		if (handle->group == GRAPHICSO)
		{
			handle->indexBufferHandle = allocationIndex;
			handle->indexSize = indexStride;
			handle->indexCount = indexCount;
		}
	}
}

void RenderInstance::PipelineUpdateIndirectCountBuffer(PipelineHandleIndex& pipelineIndex, AllocationInstanceIndex& allocationIndex)
{
	PipelineHandle* handle = pipelineHandles.Get(pipelineIndex);

	if (handle)
	{
		if (handle->group == GRAPHICSO)
		{
			handle->indirectCountBufferHandle = allocationIndex;
		}
	}
}

void RenderInstance::PipelineUpdateDispatchCommands(PipelineHandleIndex& pipelineIndex, uint32_t x, uint32_t y, uint32_t z)
{
	PipelineHandle* handle = pipelineHandles.Get(pipelineIndex);

	if (handle)
	{
		if (handle->group == COMPUTESO)
		{
			handle->x = x;
			handle->y = y;
			handle->z = z;
		}
	}
}

BufferMemoryIndex RenderInstance::CreateUniversalBuffer(RenderDeviceIndex deviceSelection, size_t size, MemoryType bufferMemoryType)
{
	RHIDevice* rhiDevice = GetDeviceHandle(deviceSelection);

	VKDevice* dev = rhiDevice->device;

	BufferMemoryIndex bufferIndex = bufferHandles.Allocate();

	if (BufferMemoryIndex() == bufferIndex)
	{
		return {};
	}

	EntryHandle bufferHandle = dev->CreateBuffer(
		size,
		VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT |
		VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT |
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT |
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
		VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
		VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT |
		VK_BUFFER_USAGE_TRANSFER_DST_BIT |
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
		API::ConvertMemoryTypeToVkMemoryPropertyFlags(bufferMemoryType));

	if (EntryHandle() == bufferHandle)
	{
		GetLastDeviceDriverError(rhiDevice, STRING_VIEW_FROM_LITERAL("CreateUniversalBuffer : Driver buffer creation error"));
		DestroyBufferHandle(bufferIndex);
		return {};
	}

	RenderBufferDescription* desc = bufferHandles.Get(bufferIndex);

	CleanInitializeBufferHandle(desc);

	desc->bufferHandle = bufferHandle;
	desc->type = bufferMemoryType;
	desc->deviceIndex = deviceSelection;

	return bufferIndex;
}

int RenderInstance::CreateHighLevelInstance(uint32_t vkDriverSpecificMemory, uint32_t vkDriverCacheSize, uint32_t instancePermanentSpecificMemory, uint32_t instanceCacheMemory)
{
	void* driverInstanceDataHead = AllocateFromStorageAllocator(vkDriverSpecificMemory + vkDriverCacheSize);
	void* instanceDataHead = AllocateFromStorageAllocator(instancePermanentSpecificMemory + instanceCacheMemory);

	vkInstance->SetInstanceDataAndSize(driverInstanceDataHead, vkDriverSpecificMemory, vkDriverCacheSize);

	VKInstanceDebugData vkDebugData{};

	vkDebugData.userCallback = vulkanDebugCallback;
	vkDebugData.userData = internalRendererLogger;
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
	instanceFeaturesRequest.windowManagementType = WindowManagementType::WINDOWS32;

	int ret = vkInstance->CreateRenderInstance(instanceDataHead, instancePermanentSpecificMemory, instanceCacheMemory, vkDebugDataTemp, &instanceFeaturesRequest);

	if (ret)
	{
		GetLastInstanceDriverError(STRING_VIEW_FROM_LITERAL("CreateHighLevelInstance: driver instance creation failed"));
	}

	return ret;
}

WindowIndex RenderInstance::CreateWindowedSurface(OSWindowInternalData* windowData)
{
	WindowIndex windowAllocIndex = windowsSurfaces.Allocate();

	if (WindowIndex() == windowAllocIndex)
	{
		return {};
	}

#if defined(_WIN32)
	EntryHandle renderSurfaceIndex = vkInstance->CreateWindowedSurface(windowData->inst, windowData->wnd);
#else
	EntryHandle renderSurfaceIndex = EntryHandle();
#endif

	if (EntryHandle() == renderSurfaceIndex)
	{
		windowsSurfaces.Free(windowAllocIndex);
		GetLastInstanceDriverError(STRING_VIEW_FROM_LITERAL("CreateWindowedSurface: driver window creation failed"));
		return {};
	}

	RenderWindowSpecificData* winData = windowsSurfaces.Get(windowAllocIndex);

	winData->vkRenderSurface = renderSurfaceIndex;

	return windowAllocIndex;
}

ShaderResourceManagerIndex RenderInstance::CreateDescriptorHeap(RenderDeviceIndex deviceSelection, DescriptorTypes* types, uint32_t* descriptorCountPerFrame, uint32_t numDescriptorTypesCount, uint32_t maxDescriptorSets, uint32_t maxShaderResourceSets)
{
	RHIDevice* rhiDevice = GetDeviceHandle(deviceSelection);

	VKDevice* dev = rhiDevice->device;

	ShaderResourceManagerIndex descriptorManagerIndex = descriptorManagers.Allocate();

	if (ShaderResourceManagerIndex() == descriptorManagerIndex)
	{
		return descriptorManagerIndex;
	}

	ShaderResourceManager* manager = descriptorManagers.Get(descriptorManagerIndex);

	CleanInitializeDescriptorManager(manager);

	manager->Create(storageAllocator, maxShaderResourceSets, STRING_VIEW_FROM_LITERAL("Descriptor Manager"), internalRendererLogger);

	DescriptorPoolBuilder builder = dev->CreateDescriptorPoolBuilder(numDescriptorTypesCount, VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT);

	for (uint32_t i = 0; i < numDescriptorTypesCount; i++)
	{
		uint32_t individualCount = descriptorCountPerFrame[i];

		switch (types[i])
		{
		case DescriptorTypes::UNIFORM_DESCRIPTOR:
		{
			builder.AddUniformPoolSize(MAX_FRAMES_IN_FLIGHT * individualCount);
			break;
		}
		case DescriptorTypes::UNORDERED_ACCESS_DESCRIPTOR:
		{
			builder.AddStoragePoolSize(MAX_FRAMES_IN_FLIGHT * individualCount);
			break;
		}
		case DescriptorTypes::SAMPLED_IMAGE_DESCRIPTOR:
		{
			builder.AddSampledImage(MAX_FRAMES_IN_FLIGHT * individualCount);
			break;
		}
		case DescriptorTypes::STORAGE_IMAGE_DESCRIPTOR:
		{
			builder.AddStorageImage(MAX_FRAMES_IN_FLIGHT * individualCount);
			break;
		}
		case DescriptorTypes::SAMPLER_DESCRIPTOR:
		{
			builder.AddSampler(MAX_FRAMES_IN_FLIGHT * individualCount);
			break;
		}
		case DescriptorTypes::COMBINED_IMAGE_SAMPLER_DESCRIPTOR:
		{
			builder.AddImageSamplerCombined(MAX_FRAMES_IN_FLIGHT * individualCount);
			break;
		}
		default:
		{
			break;
		}
		}
	};
	
	manager->deviceResourceHeap = dev->CreateDesciptorPool(&builder, MAX_FRAMES_IN_FLIGHT * maxDescriptorSets);
	manager->deviceIndex = deviceSelection;

	if (EntryHandle() == manager->deviceResourceHeap)
	{
		GetLastDeviceDriverError(rhiDevice, STRING_VIEW_FROM_LITERAL("CreateDescriptorHeap : driver of heap creation failed"));
		DestroyDescriptorManager(descriptorManagerIndex);
		return {};
	}

	return descriptorManagerIndex;
}

void RenderInstance::GeneratePipelineDescriptorBarriers(CommandRecorder* recorder, ShaderResourceSetHandle* descriptorid, int descriptorcount, PipelineHandleIndex& pipelineIndex)
{
	for (int i = 0; i < descriptorcount; i++)
	{
		ShaderResourceManager* manager = descriptorManagers.Get(descriptorid[i].descriptorManagerIndex);

		ShaderResourceSet* set = manager->descriptorSets[descriptorid[i].descriptorSetIndex];

		int counter = 0;

		int totalBindingCount = set->templateMetaData->bindingCount;

		while (counter < totalBindingCount)
		{
			ShaderResourceArray* header = (ShaderResourceArray*)&set->resourceBindings[counter++];
			switch (header->type)
			{
			case ShaderResourceType::SAMPLERCUBE:
			case ShaderResourceType::SAMPLER2D:
			case ShaderResourceType::SAMPLER3D:
			{
				ShaderResourceCombinedImage* imageBarrier = &header->resourceArray.combinedImages;

				int arrayCount = imageBarrier->textureCount;

				for (int imageIndex = 0; imageIndex < arrayCount; imageIndex++)
				{
					TextureIndex currImageIndex = imageBarrier->textureDetails[imageIndex].textureHandle;

					int viewIndex = imageBarrier->textureDetails[imageIndex].viewIndex;

					TransitionImageLayout(recorder->device->device, currImageIndex, viewIndex, ConvertShaderStageToBarrierStage(header->stage), READ_SHADER_RESOURCE, recorder->accumulator, pipelineIndex);
				}
				break;
			}
			case ShaderResourceType::IMAGE2D:
			{
				ShaderResourceImage* imageBarrier = &header->resourceArray.images;

				int arrayCount = imageBarrier->textureCount;

				for (int imageIndex = 0; imageIndex < arrayCount; imageIndex++)
				{
					TextureIndex currImageIndex = imageBarrier->textureDetails[imageIndex].textureHandle;

					int viewIndex = imageBarrier->textureDetails[imageIndex].viewIndex;

					TransitionImageLayout(recorder->device->device, currImageIndex, viewIndex, ConvertShaderStageToBarrierStage(header->stage), READ_SHADER_RESOURCE, recorder->accumulator, pipelineIndex);
				}
				break;
			}
			case ShaderResourceType::IMAGESTORE2D:
			{
				ShaderResourceImage* imageBarrier = &header->resourceArray.images;

				int arrayCount = imageBarrier->textureCount;

				for (int imageIndex = 0; imageIndex < arrayCount; imageIndex++)
				{
					TextureIndex currImageIndex = imageBarrier->textureDetails[imageIndex].textureHandle;

					int viewIndex = imageBarrier->textureDetails[imageIndex].viewIndex;

					TransitionImageLayout(recorder->device->device, currImageIndex, viewIndex, COMPUTE_BARRIER, WRITE_SHADER_RESOURCE, recorder->accumulator, pipelineIndex);
				}
				break;
			}
			case ShaderResourceType::BUFFER_VIEW:
			case ShaderResourceType::STORAGE_BUFFER:
			case ShaderResourceType::UNIFORM_BUFFER:
			{
				ShaderResourceBuffer* bufferBarrier = (header->type == ShaderResourceType::BUFFER_VIEW) ? (ShaderResourceBuffer*)&header->resourceArray.views : (ShaderResourceBuffer*)&header->resourceArray.buffers;

				int arrayCount = bufferBarrier->bufferCount;

				for (int g = 0; g < arrayCount; g++)
				{
					AllocationInstanceIndex& allocationIndex = bufferBarrier->allocationIndex[g];

					InsertBufferBarrier(recorder->device->device, allocationIndex, ConvertShaderStageToBarrierStage(header->stage), header, pipelineIndex, recorder->accumulator);
				}
				break;
			}
			}
		}
	}
}

void RenderInstance::InsertAccumulatedBarriers(CommandRecorder* recorder)
{
	RBOPipelineBarrierArgs args{};

	BarrierAccumulator* accumulator = recorder->accumulator;

	if (accumulator->accumulators[IMAGE_BARRIER_ACCUMULATOR].barrierCount)
	{
		args.srcStageMask |= API::ConvertBarrierStageToVulkanPipelineStage(accumulator->accumulators[IMAGE_BARRIER_ACCUMULATOR].srcStage);
		args.dstStageMask |= API::ConvertBarrierStageToVulkanPipelineStage(accumulator->accumulators[IMAGE_BARRIER_ACCUMULATOR].dstStage);

		args.imageMemoryBarrierCount = accumulator->accumulators[IMAGE_BARRIER_ACCUMULATOR].barrierCount;
		args.pImageMemoryBarriers = (VkImageMemoryBarrier*)accumulator->accumulators[IMAGE_BARRIER_ACCUMULATOR].allocator->dataHead;
	}

	if (accumulator->accumulators[BUFFER_BARRIER_ACCUMULATOR].barrierCount)
	{
		args.srcStageMask |= API::ConvertBarrierStageToVulkanPipelineStage(accumulator->accumulators[BUFFER_BARRIER_ACCUMULATOR].srcStage);
		args.dstStageMask |= API::ConvertBarrierStageToVulkanPipelineStage(accumulator->accumulators[BUFFER_BARRIER_ACCUMULATOR].dstStage);

		args.bufferMemoryBarrierCount = accumulator->accumulators[BUFFER_BARRIER_ACCUMULATOR].barrierCount;
		args.pBufferMemoryBarriers = (VkBufferMemoryBarrier*)accumulator->accumulators[BUFFER_BARRIER_ACCUMULATOR].allocator->dataHead;
	}

	if (args.imageMemoryBarrierCount || args.bufferMemoryBarrierCount)
	{
		recorder->rbo->BindPipelineBarrierCommand(&args);

		accumulator->accumulators[BUFFER_BARRIER_ACCUMULATOR].barrierCount = accumulator->accumulators[BUFFER_BARRIER_ACCUMULATOR].dstStage = accumulator->accumulators[BUFFER_BARRIER_ACCUMULATOR].srcStage = 0;

		accumulator->accumulators[BUFFER_BARRIER_ACCUMULATOR].allocator->Reset();

		accumulator->accumulators[IMAGE_BARRIER_ACCUMULATOR].barrierCount = accumulator->accumulators[IMAGE_BARRIER_ACCUMULATOR].srcStage = accumulator->accumulators[IMAGE_BARRIER_ACCUMULATOR].dstStage = 0;

		accumulator->accumulators[IMAGE_BARRIER_ACCUMULATOR].allocator->Reset();
	}
}

void RenderInstance::GenerateDrawBindingsBarriers(CommandRecorder* recorder, PipelineHandle* handle)
{
	if (AllocationInstanceIndex() != handle->vertexBufferHandle)
		InsertBufferBarrier(recorder->device->device, handle->vertexBufferHandle, StageBits::VERTEX_INPUT_BARRIER, BarrierActionBits::READ_VERTEX_INPUT, recorder->accumulator);

	if (AllocationInstanceIndex() != handle->indirectBufferHandle)
		InsertBufferBarrier(recorder->device->device, handle->indirectBufferHandle, StageBits::INDIRECT_DRAW_BARRIER, BarrierActionBits::READ_INDIRECT_COMMAND, recorder->accumulator);

	if (AllocationInstanceIndex() != handle->indirectCountBufferHandle)
		InsertBufferBarrier(recorder->device->device, handle->indirectCountBufferHandle, StageBits::INDIRECT_DRAW_BARRIER, BarrierActionBits::READ_INDIRECT_COMMAND, recorder->accumulator);
}

void RenderInstance::GenerateComputeDispatchBindingsBarriers(CommandRecorder* recorder, PipelineHandle* handle, PipelineHandleIndex& pipelineIndex)
{
	if (AllocationInstanceIndex() != handle->indirectDispatchCommandHandle)
	{
		size_t size = 0, offset = 0, align = 0;

		int bufferLastAccessFrame = 0;

		VkBufferMemoryBarrier* vkBarrier = nullptr;

		RenderAllocation* alloc = allocations.Get(handle->indirectDispatchCommandHandle);

		GetAllocationDetails(alloc, &size, &offset, &bufferLastAccessFrame, currentFrame);

		ResourceStatus* status = resourceStatuses.Get(alloc->resourceStatus);

		if (StageBits::INDIRECT_DRAW_BARRIER & status->currStage[bufferLastAccessFrame] && BarrierActionBits::READ_INDIRECT_COMMAND & status->currAction[bufferLastAccessFrame])
			return;

		vkBarrier = (VkBufferMemoryBarrier*)recorder->accumulator->intraPassBarrierAllocator.Allocate(sizeof(VkBufferMemoryBarrier));

		IntraPassBarrier* intraBarrier = GetIntraPassBarrier(recorder->accumulator, BarrierType::BUFFER_BARRIER, pipelineIndex, vkBarrier);

		intraBarrier->destStage |= StageBits::INDIRECT_DRAW_BARRIER;
		intraBarrier->srcStage |= status->currStage[bufferLastAccessFrame];
		intraBarrier->barrierCount++;
		
		VkBuffer buffer = recorder->device->device->GetBufferHandle(bufferHandles[alloc->memIndex].bufferHandle);

		BarrierAction newAction = BarrierActionBits::READ_INDIRECT_COMMAND;

		vkBarrier->sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
		vkBarrier->pNext = nullptr;
		vkBarrier->srcAccessMask = API::ConvertBarrierActionToVulkanAccessFlags(status->currAction[bufferLastAccessFrame]);
		vkBarrier->dstAccessMask = API::ConvertBarrierActionToVulkanAccessFlags(newAction);
		vkBarrier->srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		vkBarrier->dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		vkBarrier->buffer = buffer;
		vkBarrier->offset = offset;
		vkBarrier->size = size;

		status->currStage[bufferLastAccessFrame] = StageBits::INDIRECT_DRAW_BARRIER;
		status->currAction[bufferLastAccessFrame] = newAction;
	}
}

void RenderInstance::TransitionImageLayout(VKDevice* dev, TextureIndex& imageIndex, int perImageViewIndex, PipelineStage destBarrierStage, BarrierAction destBarrierAction, BarrierAccumulator* accumulator, PipelineHandleIndex& pipelineIndex)
{
	RenderTextureDescription* desc = textureResourceHandles.Get(imageIndex);

	ResourceStatus* status = resourceStatuses.Get(desc->resourceStatusIndex);

	RenderImageViewDescription* viewDesc = textureViewsResourceHandles.Get(desc->viewIndex[perImageViewIndex]);

	if (status->resourceType == ResourceStatusType::MANAGED_IMAGE_RESOURCE)
		return;

	int viewMipStart = viewDesc->firstMipLevel;
	int viewMipCount = viewDesc->mipLevelCount;
	int totalMipCount = desc->mipLayers;

	int viewLayerStart = viewDesc->firstLayer;
	int viewLayerCount = viewDesc->layerCount;
	int totalLayerCount = desc->arrayLayers;

	TransitionImageLayout(dev,  desc->textureIndex, viewMipStart, viewMipCount, totalMipCount, viewLayerStart, viewLayerCount, viewDesc->mask, viewDesc->desiredLayoutForView, status, destBarrierStage, destBarrierAction, accumulator, pipelineIndex);
}

void RenderInstance::TransitionImageLayout(VKDevice* dev, EntryHandle imageIndex, int mipStart, int mipCount, int totalMipCount, int layerStart, int layerCount,
	ImageViewAspectMask mask, ImageLayout requestedLayout, ResourceStatus* status,
	PipelineStage destBarrierStage, BarrierAction destBarrierAction, BarrierAccumulator* accumulator, PipelineHandleIndex& pipelineIndex)
{
	VkImageMemoryBarrier barrier{};

	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.subresourceRange.aspectMask = API::ConvertImageViewAspectMaskToVulkanImageAspectFlags(mask);

	barrier.dstAccessMask = API::ConvertBarrierActionToVulkanAccessFlags(destBarrierAction);
	barrier.newLayout = API::ConvertImageLayoutToVulkanImageLayout(requestedLayout);
	barrier.image = dev->GetImageByHandle(imageIndex);

	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

	struct MipArrayLevel
	{
		int trackedMipStart;
		int trackedMipCount;
		BarrierAction actions;
		PipelineStage stages;
		ImageLayout layout;
		int coalesced;
		struct MipArrayLevel* nextInLevel;
	};

	MipArrayLevel* arrayLevels = (MipArrayLevel*)cacheAllocator->CAllocate(sizeof(MipArrayLevel) * (mipCount * layerCount));

	MipArrayLevel** linkedListPtr = (MipArrayLevel**)cacheAllocator->CAllocate(sizeof(MipArrayLevel*) * layerCount);

	int nodeCount = 0;

	for (int j = layerStart; j < layerStart + layerCount; j++)
	{
		MipArrayLevel* curr = nullptr;

		MipArrayLevel** next = &linkedListPtr[j - layerStart];

		for (int i = mipStart; i < mipStart + mipCount; i++)
		{
			int currentMipArrayIndex = (j * totalMipCount) + i;

			BarrierAction currAction = status->currAction[currentMipArrayIndex];
			PipelineStage currStage = status->currStage[currentMipArrayIndex];
			ImageLayout currLayout = status->currentLayout[currentMipArrayIndex];;

			if ((currLayout != requestedLayout)
				|| (currStage != destBarrierStage)
				|| (currAction != destBarrierAction)
				)
			{
				if (!curr || currLayout != curr->layout)
				{
					if (curr && curr->trackedMipCount)
					{
						*next = curr;
						next = &curr->nextInLevel;
					}

					MipArrayLevel* newLevel = &arrayLevels[nodeCount++];

					newLevel->trackedMipStart = i;
					newLevel->layout = currLayout;
					newLevel->trackedMipCount = 1;
					newLevel->nextInLevel = nullptr;
					newLevel->actions = currAction;
					newLevel->stages = currStage;
					newLevel->coalesced = 0;

					curr = newLevel;
				}
				else
				{
					curr->actions |= currAction;
					curr->stages |= currStage;
					curr->trackedMipCount++;
				}

				status->currentLayout[currentMipArrayIndex] = requestedLayout;
				status->currAction[currentMipArrayIndex] = destBarrierAction;
				status->currStage[currentMipArrayIndex] = destBarrierStage;
			}
			else
			{
				if (curr && curr->trackedMipCount)
				{
					*next = curr;
					next = &curr->nextInLevel;
				}

				curr = nullptr;
			}
		}

		if (curr && curr->trackedMipCount)
		{
			*next = curr;
		}
	}

	//attempt to merge square rectangles with same layout and mip start/count, no splitting

	for (int i = 0; i < layerCount; i++)
	{
		MipArrayLevel* curr = linkedListPtr[i];

		while (curr)
		{
			if (curr->coalesced)
			{
				curr = curr->nextInLevel;
				continue;
			}

			int trackedMipStart = curr->trackedMipStart;
			int trackedMipCount = curr->trackedMipCount;
			int trackedLayerCount = 1;
			int trackedLayerStart = i + layerStart;

			for (int j = i + 1; j < layerCount; j++)
			{
				MipArrayLevel* candidate = linkedListPtr[j];
				int extended = 0;
				while (candidate)
				{
					if (!candidate->coalesced)
					{
						if (candidate->trackedMipStart == trackedMipStart &&
							candidate->trackedMipCount == trackedMipCount &&
							candidate->layout == curr->layout)
						{
							trackedLayerCount++;
							extended = 1;
							candidate->coalesced = 1;

							curr->stages |= candidate->stages;
							curr->actions |= candidate->actions;

							break;
						}
					}
					candidate = candidate->nextInLevel;
				}
				if (!extended)
					break;
			}

			VkImageMemoryBarrier* currentBarrier = nullptr;
			
			if (destBarrierStage & curr->stages)
			{
				currentBarrier = (VkImageMemoryBarrier*)accumulator->intraPassBarrierAllocator.Allocate(sizeof(VkImageMemoryBarrier));
				
				IntraPassBarrier* intraBarrier = GetIntraPassBarrier(accumulator, BarrierType::IMAGE_BARRIER, pipelineIndex, currentBarrier);

				intraBarrier->destStage |= destBarrierStage;
				intraBarrier->srcStage |= curr->stages;
				intraBarrier->barrierCount++;
			}
			else
			{
				currentBarrier = (VkImageMemoryBarrier*)accumulator->accumulators[IMAGE_BARRIER_ACCUMULATOR].allocator->Allocate(sizeof(VkImageMemoryBarrier));

				accumulator->accumulators[IMAGE_BARRIER_ACCUMULATOR].srcStage |= curr->stages;
				accumulator->accumulators[IMAGE_BARRIER_ACCUMULATOR].dstStage |= destBarrierStage;
				accumulator->accumulators[IMAGE_BARRIER_ACCUMULATOR].barrierCount++;
			}

			*currentBarrier = barrier;

			currentBarrier->oldLayout = API::ConvertImageLayoutToVulkanImageLayout(curr->layout);
			currentBarrier->srcAccessMask = API::ConvertBarrierActionToVulkanAccessFlags(curr->actions);
			currentBarrier->subresourceRange.baseMipLevel = trackedMipStart;
			currentBarrier->subresourceRange.levelCount = trackedMipCount;
			currentBarrier->subresourceRange.baseArrayLayer = trackedLayerStart;
			currentBarrier->subresourceRange.layerCount = trackedLayerCount;

			curr = curr->nextInLevel;
		}
	}
}

IntraPassBarrier* RenderInstance::GetIntraPassBarrier(BarrierAccumulator* accum, BarrierType type, PipelineHandleIndex& pipelineIndex, void* driverBarrierData)
{
	int topIndex = RENDER_MAX(accum->intraPassCount - 1, 0);

	IntraPassBarrier* intraPassBarrier = &accum->intraPassBarriers[topIndex];

	if (intraPassBarrier->pipelineInst != pipelineIndex || intraPassBarrier->barrierType != type)
	{
		intraPassBarrier = &accum->intraPassBarriers[accum->intraPassCount++];
		intraPassBarrier->pipelineInst = pipelineIndex;
		intraPassBarrier->barrierType = type;
		intraPassBarrier->barrierCount = 0;
		intraPassBarrier->driverSpecificBarriers = driverBarrierData;
		intraPassBarrier->destStage = 0;
		intraPassBarrier->srcStage = 0;
	}

	return intraPassBarrier;
}

void RenderInstance::InsertBufferBarrier(VKDevice* dev, AllocationInstanceIndex& allocationIndex, PipelineStage destBarrierStage, ShaderResourceHeader* header, PipelineHandleIndex& pipelineIndex, BarrierAccumulator* accumulator)
{
	size_t size = 0, offset = 0, align = 0;

	int bufferLastAccessFrame = 0;

	AllocationType allocType;

	VkBufferMemoryBarrier* vkBarrier = nullptr;

	RenderAllocation* alloc = allocations.Get(allocationIndex);

	GetAllocationDetails(alloc, &size, &offset, &bufferLastAccessFrame, currentFrame);

	ResourceStatus* status = resourceStatuses.Get(alloc->resourceStatus);

	if 
	(
		(status->currAction[bufferLastAccessFrame] & (BarrierActionBits::READ_SHADER_RESOURCE | BarrierActionBits::READ_UNIFORM_BUFFER))
		&& !(status->currAction[bufferLastAccessFrame] & BarrierActionBits::WRITE_SHADER_RESOURCE)
		&& (header->action == ShaderResourceAction::SHADERREAD)
		&& (status->currStage[bufferLastAccessFrame] & destBarrierStage)
	)
	{
		return;
	}

	if (destBarrierStage & status->currStage[bufferLastAccessFrame])
	{
		vkBarrier = (VkBufferMemoryBarrier*)accumulator->intraPassBarrierAllocator.Allocate(sizeof(VkBufferMemoryBarrier));

		IntraPassBarrier* intraBarrier = GetIntraPassBarrier(accumulator, BarrierType::BUFFER_BARRIER, pipelineIndex, vkBarrier);

		intraBarrier->destStage |= destBarrierStage;
		intraBarrier->srcStage |= status->currStage[bufferLastAccessFrame];
		intraBarrier->barrierCount++;
	}
	else
	{
		vkBarrier = (VkBufferMemoryBarrier*)accumulator->accumulators[BUFFER_BARRIER_ACCUMULATOR].allocator->Allocate(sizeof(VkBufferMemoryBarrier));

		accumulator->accumulators[BUFFER_BARRIER_ACCUMULATOR].srcStage |= status->currStage[bufferLastAccessFrame];
		accumulator->accumulators[BUFFER_BARRIER_ACCUMULATOR].dstStage |= destBarrierStage;
		accumulator->accumulators[BUFFER_BARRIER_ACCUMULATOR].barrierCount++;
	}

	VkBuffer buffer = dev->GetBufferHandle(bufferHandles[alloc->memIndex].bufferHandle);

	BarrierAction newAction = 0;

	if (header->type == ShaderResourceType::UNIFORM_BUFFER)
		newAction = BarrierActionBits::READ_UNIFORM_BUFFER;
	else
	{
		if (header->action == ShaderResourceAction::SHADERWRITE)
			newAction = BarrierActionBits::WRITE_SHADER_RESOURCE;
		else if (header->action == ShaderResourceAction::SHADERREADWRITE)
			newAction = BarrierActionBits::READ_SHADER_RESOURCE | BarrierActionBits::WRITE_SHADER_RESOURCE;
		else
			newAction = BarrierActionBits::READ_SHADER_RESOURCE;
	}

	vkBarrier->sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
	vkBarrier->pNext = nullptr;
	vkBarrier->srcAccessMask = API::ConvertBarrierActionToVulkanAccessFlags(status->currAction[bufferLastAccessFrame]);
	vkBarrier->dstAccessMask = API::ConvertBarrierActionToVulkanAccessFlags(newAction);
	vkBarrier->srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	vkBarrier->dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	vkBarrier->buffer = buffer;
	vkBarrier->offset = offset;
	vkBarrier->size = size;

	status->currStage[bufferLastAccessFrame] = destBarrierStage;
	status->currAction[bufferLastAccessFrame] = newAction;
}

void RenderInstance::InsertBufferBarrier(VKDevice* dev, AllocationInstanceIndex& allocationIndex, PipelineStage destBarrierStage, BarrierAction destBarrierAction, BarrierAccumulator* accumulator)
{
	RenderAllocation* bufferAlloc = allocations.Get(allocationIndex);

	ResourceStatus* status = resourceStatuses.Get(bufferAlloc->resourceStatus);

	size_t bufferSize = 0, bufferBaseOffset = 0;

	int resourceIndexToUpdate = 0;

	GetAllocationDetails(bufferAlloc, &bufferSize, &bufferBaseOffset, &resourceIndexToUpdate, currentFrame);

	if (status->currStage[resourceIndexToUpdate] != destBarrierStage ||
		status->currAction[resourceIndexToUpdate] != destBarrierAction)
	{
		VkBufferMemoryBarrier* vkBarrier = (VkBufferMemoryBarrier*)accumulator->accumulators[BUFFER_BARRIER_ACCUMULATOR].allocator->Allocate(sizeof(VkBufferMemoryBarrier));

		VkBuffer buffer = dev->GetBufferHandle(bufferHandles[bufferAlloc->memIndex].bufferHandle);

		vkBarrier->sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
		vkBarrier->pNext = nullptr;
		vkBarrier->srcAccessMask = API::ConvertBarrierActionToVulkanAccessFlags(status->currAction[resourceIndexToUpdate]);
		vkBarrier->dstAccessMask = API::ConvertBarrierActionToVulkanAccessFlags(destBarrierAction);
		vkBarrier->srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		vkBarrier->dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		vkBarrier->buffer = buffer;
		vkBarrier->offset = bufferBaseOffset;
		vkBarrier->size = bufferSize;

		accumulator->accumulators[BUFFER_BARRIER_ACCUMULATOR].srcStage |= status->currStage[resourceIndexToUpdate];
		accumulator->accumulators[BUFFER_BARRIER_ACCUMULATOR].dstStage |= destBarrierStage;
		accumulator->accumulators[BUFFER_BARRIER_ACCUMULATOR].barrierCount++;

		status->currAction[resourceIndexToUpdate] = destBarrierAction;
		status->currStage[resourceIndexToUpdate] = destBarrierStage;
	}
}

void RenderInstance::InsertIntraPassBarrier(CommandRecorder* recorder, PipelineHandleIndex& pipelineIndex)
{
	if (recorder->accumulator->intraPassTop == recorder->accumulator->intraPassCount)
		return;

	IntraPassBarrier* ipb = &recorder->accumulator->intraPassBarriers[recorder->accumulator->intraPassTop];

	VkImageMemoryBarrier imageMemoryBarriers[32];
	VkBufferMemoryBarrier bufferMemoryBarriers[32];

	uint32_t imageCount = 0;
	uint32_t bufferCount = 0;

	RBOPipelineBarrierArgs args{};

	args.pBufferMemoryBarriers = bufferMemoryBarriers;
	args.pImageMemoryBarriers = imageMemoryBarriers;

	while(recorder->accumulator->intraPassTop < recorder->accumulator->intraPassCount && ipb->pipelineInst == pipelineIndex)
	{
		args.srcStageMask |= API::ConvertBarrierStageToVulkanPipelineStage(ipb->srcStage);
		args.dstStageMask |= API::ConvertBarrierStageToVulkanPipelineStage(ipb->destStage);

		if (ipb->barrierType == BarrierType::IMAGE_BARRIER)
		{
			VkImageMemoryBarrier* barriers = (VkImageMemoryBarrier*)ipb->driverSpecificBarriers;

			for (uint32_t i = 0; i < ipb->barrierCount; i++)
			{
				args.pImageMemoryBarriers[imageCount++] = barriers[i];
			}
		} 
		else if (ipb->barrierType == BarrierType::BUFFER_BARRIER)
		{
			VkBufferMemoryBarrier* barriers = (VkBufferMemoryBarrier*)ipb->driverSpecificBarriers;

			for (uint32_t i = 0; i < ipb->barrierCount; i++)
			{
				args.pBufferMemoryBarriers[bufferCount++] = barriers[i];
			}
		}

		recorder->accumulator->intraPassTop++;

		ipb = &recorder->accumulator->intraPassBarriers[recorder->accumulator->intraPassTop];
	}

	if (imageCount || bufferCount)
	{
		args.imageMemoryBarrierCount = imageCount;
		args.bufferMemoryBarrierCount = bufferCount;
		recorder->rbo->BindPipelineBarrierCommand(&args);
	}
}

void RenderInstance::DestroyPhysicalDeviceIndices(RenderPhysicalDeviceIndex handle)
{
	if (handle.index >= maxPhysicalDevices || handle.index >= physicalDeviceCounter)
	{
		return;
	}

	RenderPhysicalDeviceContainer* container = &physicalDeviceIndices[handle.index];

	DestroyDriverPhysicalDevice(vkInstance, container->physicalDeviceIndex);

	CleanInitializePhysicalDeviceIndices(container);

	physicalDeviceCounter--;
}

void RenderInstance::DestroyLogicalDeviceIndices(RenderDeviceIndex handle)
{
	if (handle.index >= maxLogicalDevices || handle.index >= logicalDeviceCounter)
	{
		return;
	}

	RHIDevice* container = GetDeviceHandle(handle);

	DestroyDriverLogicalDevice(vkInstance, container->container.logicalDeviceIndex);

	CleanInitializeRHIDevice(container);

	logicalDeviceCounter--;
}

void RenderInstance::DestroyWindowsSurfaces(WindowIndex& handle)
{
	RenderWindowSpecificData* data = windowsSurfaces.Get(handle);

	if (!data)
	{
		return;
	}

	DestroyDriverWindowsSurface(vkInstance, data->vkRenderSurface);

	CleanInitializeWindowsSurface(data);

	windowsSurfaces.Free(handle);
}

void RenderInstance::DestroySwapChain(SwapChainIndex& handle)
{
	RenderSwapchainData* swcData = swapChains.Get(handle);

	if (!swcData)
	{
		return;
	}

	if (swcData->deviceIndex.index >= 0)
	{
		RHIDevice* container = GetDeviceHandle(swcData->deviceIndex);

		for (int i = 0; i < swcData->imageCount; i++)
		{
			if (TextureIndex() != swcData->textureIds[i])
			{
				RenderTextureDescription* texDesc = textureResourceHandles.Get(swcData->textureIds[i]);

				for (int j = 0; j < texDesc->viewCount; j++)
				{
					TextureViewIndex viewHandle = texDesc->viewIndex[j];

					RenderImageViewDescription* viewDesc = textureViewsResourceHandles.Get(viewHandle);

					CleanInitializeTextureViewsResourceHandle(viewDesc);

					textureViewsResourceHandles.Free(viewHandle);
				}

				CleanInitializeResourceStatus(resourceStatuses.Get(texDesc->resourceStatusIndex));

				resourceStatuses.Free(texDesc->resourceStatusIndex);

				CleanInitializeTextureResourceHandle(texDesc);

				textureResourceHandles.Free(swcData->textureIds[i]);
			}

			DestroyDriverSemaphore(container, swcData->rendererFinishedSemaphores[i]);
		}

		for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			DestroyDriverSemaphore(container, swcData->rendererWaitSemaphores[i]);
		}

		DestroyDriverSwapChain(container, swcData->swapChainIdx);
	}

	CleanInitializeSwapChain(swcData);

	swapChains.Free(handle);
}

void RenderInstance::DestroyBufferHandle(BufferMemoryIndex& handle)
{
	RenderBufferDescription* bufferHandle = bufferHandles.Get(handle);

	if (!bufferHandle)
	{
		return;
	}

	if (bufferHandle->deviceIndex.index >= 0)
	{
		RHIDevice* container = GetDeviceHandle(bufferHandle->deviceIndex);

		DestroyDriverBufferHandle(container, bufferHandle->bufferHandle);
	}

	CleanInitializeBufferHandle(bufferHandle);

	bufferHandles.Free(handle);
}

void RenderInstance::DestroyImagePool(ImageMemoryIndex& handle)
{
	ImagePoolDescription* imagePool = imagePools.Get(handle);
	
	if (!imagePool)
	{
		return;
	}

	if (imagePool->deviceIndex.index >= 0)
	{
		RHIDevice* container = GetDeviceHandle(imagePool->deviceIndex);

		DestroyDriverImagePool(container, imagePool->imagePoolHandle);
	}
	
	CleanInitializeImagePool(imagePool);

	imagePools.Free(handle);
}

void RenderInstance::DestroyPipelineHandle(PipelineHandleIndex& handle)
{
	PipelineHandle* pipelineHandle = pipelineHandles.Get(handle);
	
	if (!pipelineHandle)
	{
		return;
	}

	CleanInitializePipelineHandle(pipelineHandle);

	pipelineHandles.Free(handle);
}

void RenderInstance::DestroyAttachmentGraph(int handle)
{
	AttachmentGraph* attachmentGraph = attachmentGraphs.Get(handle);
	
	if (!attachmentGraph)
	{
		return;
	}

	CleanInitializeAttachmentGraph(attachmentGraph);

	attachmentGraphs.Free(handle);
}

void RenderInstance::DestroyAttachmentGraphInstance(AttachmentGraphInstanceIndex& handle)
{

	AttachmentGraphInstance* attachmentGraphInstance = attachmentGraphsInstances.Get(handle);
	
	if (!attachmentGraphInstance)
	{
		return;
	}

	RHIDevice* container = GetDeviceHandle(attachmentGraphInstance->deviceIndex);

	int resourceCount = attachmentGraphInstance->graphLayout->resourceCount;

	int passesCount = attachmentGraphInstance->graphLayout->passesCount;

	for (int i = 0; i < resourceCount; i++)
	{
		int sampleCount = RENDER_MAX(findMSB(attachmentGraphInstance->resources[i].sampHi), 1);

		for (int j = 0; j < sampleCount; j++)
		{
			for (int g = 0; g < attachmentGraphInstance->resources[i].imageCount; g++)
			{
				if (TextureIndex() != attachmentGraphInstance->resources[i].textureIds[j][g])
				{
					DestroyTextureResourceHandle(attachmentGraphInstance->resources[i].textureIds[j][g]);
				}
			}
		}
	}

	for (int i = 0; i < passesCount; i++)
	{
		int sampleCount = attachmentGraphInstance->passes[i].maxSampleCount;

		for (int h = 0; h < sampleCount; h++)
		{
			if (OldStyleRenderPassIndex() != attachmentGraphInstance->passes[i].baseRenderPass[h])
			{
				DestroyRenderPass(attachmentGraphInstance->passes[i].baseRenderPass[h]);
			}
		}
		
		for (int h = 0; h < sampleCount; h++)
		{
			if (DriverRenderTargetIndex() != attachmentGraphInstance->passes[i].baseRenderTarget[h])
			{
				DestroyRenderTarget(attachmentGraphInstance->passes[i].baseRenderTarget[h]);
			}
		}
	}

	CleanInitializeAttachmentGraphsInstance(attachmentGraphInstance);
	
	attachmentGraphsInstances.Free(handle);
}

void RenderInstance::DestroyRenderTargetQueue(PipelineQueueIndex& handle)
{
	RenderQueue* renderTargetQueue = renderTargetQueues.Get(handle);
	
	if (!renderTargetQueue)
	{
		return;
	}

	CleanInitializeRenderTargetQueue(renderTargetQueue);

	renderTargetQueues.Free(handle);
}

void RenderInstance::DestroyComputeQueue(PipelineQueueIndex& handle)
{
	ComputeQueue* computeQueue = computeQueues.Get(handle);
	
	if (!computeQueue)
	{
		return;
	}

	CleanInitializeComputeQueue(computeQueue);

	computeQueues.Free(handle);
}

void RenderInstance::DestroyTextureResourceHandle(TextureIndex& handle)
{
	RenderTextureDescription* textureResourceHandle = textureResourceHandles.Get(handle);
	
	if (!textureResourceHandle)
	{
		return;
	}

	if (textureResourceHandle->deviceIndex.index >= 0)
	{
		RHIDevice* container = GetDeviceHandle(textureResourceHandle->deviceIndex);

		for (int i = 0; i < textureResourceHandle->viewCount; i++)
		{
			DestroyTextureViewsResourceHandle(textureResourceHandle->deviceIndex, textureResourceHandle->viewIndex[i]);
		}

		DestroyResourceStatus(textureResourceHandle->resourceStatusIndex);

		DestroyDriverImage(container, textureResourceHandle->textureIndex);
	}

	CleanInitializeTextureResourceHandle(textureResourceHandle);

	textureResourceHandles.Free(handle);
}

void RenderInstance::DestroyTextureViewsResourceHandle(RenderDeviceIndex mainLogicalDevice, TextureViewIndex& handle)
{
	RHIDevice* container = GetDeviceHandle(mainLogicalDevice);
	
	RenderImageViewDescription* textureViewsResourceHandle = textureViewsResourceHandles.Get(handle);
	
	if (!textureViewsResourceHandle)
	{
		return;
	}

	DestroyDriverImageView(container, textureViewsResourceHandle->viewIndex);

	CleanInitializeTextureViewsResourceHandle(textureViewsResourceHandle);

	textureViewsResourceHandles.Free(handle);
}

void RenderInstance::DestroySamplerResourceHandle(RenderDeviceIndex mainLogicalDevice, SamplerIndex handle)
{
	RHIDevice* container = GetDeviceHandle(mainLogicalDevice);
	
	EntryHandle samplerResourceHandle = samplerResourceHandles[handle];
	
	if (EntryHandle() == samplerResourceHandle)
	{
		return;
	}

	DestroyDriverSamplerResourceHandle(container, samplerResourceHandle);

	samplerResourceHandles.pool[handle.index] = EntryHandle();
	samplerResourceHandles.Free(handle);
}

void RenderInstance::DestroyResourceStatus(ResourceIndex& handle)
{	
	ResourceStatus* resourceStatus = resourceStatuses.Get(handle);
	
	if (!resourceStatus)
	{
		return;
	}

	storageAllocator->Free(resourceStatus->currAction);
	storageAllocator->Free(resourceStatus->currentLayout);
	storageAllocator->Free(resourceStatus->currStage);

	CleanInitializeResourceStatus(resourceStatus);
	
	resourceStatuses.Free(handle);
}

void RenderInstance::DestroyPipelineInfo(GenericRenderPipelineInfoIndex& handle)
{	
	GenericPipelineStateInfo* pipelineInfo = pipelineInfos.Get(handle);
	
	if (!pipelineInfo)
	{
		return;
	}

	CleanInitializePipelineInfo(pipelineInfo);

	pipelineInfos.Free(handle);
}

void RenderInstance::DestroyRenderPass(OldStyleRenderPassIndex& handle)
{
	RenderOldStyleVulkanRenderPassInfo* info = renderPasses.Get(handle);
	
	if (!info || EntryHandle() == info->renderPassHandle)
	{
		return;
	}

	RHIDevice* container = GetDeviceHandle(info->deviceIndex);

	DestroyOldStyleRenderPass(container, info->renderPassHandle);
	
	info->renderPassHandle = EntryHandle();
	info->deviceIndex = {};

	renderPasses.Free(handle);
}

void RenderInstance::DestroyRenderTarget(DriverRenderTargetIndex& handle)
{
	RenderTargetInfo* info = mainRenderTargets.Get(handle);
	
	if (!info || EntryHandle() == info->driverRenderTargetInfo)
	{
		return;
	}

	RHIDevice* container = GetDeviceHandle(info->deviceIndex);

	DestroyDriverMainRenderTarget(container, info->driverRenderTargetInfo);

	info->driverRenderTargetInfo = EntryHandle();
	info->deviceIndex = {};

	mainRenderTargets.Free(handle);
}

void RenderInstance::DestroyShaderResourceTemplate(ShaderResourceTemplateInstanceIndex& handle)
{
	RenderShaderResourceTemplateInfo* shaderResourceTemplate = shaderResourceTemplates.Get(handle);
	
	if (!shaderResourceTemplate)
	{
		return;
	}

	RHIDevice* container = GetDeviceHandle(shaderResourceTemplate->deviceIndex);

	DestroyDriverShaderResourceLayout(container, shaderResourceTemplate->resourceTemplateInstanceHandle);
	shaderResourceTemplate->resourceTemplateInstanceHandle = EntryHandle();
	shaderResourceTemplate->deviceIndex = {};
	shaderResourceTemplates.Free(handle);
}

void RenderInstance::DestroyAllocation(AllocationInstanceIndex& handle)
{
	RenderAllocation* allocation = allocations.Get(handle);
	
	if (!allocation)
	{
		return;
	}

	RHIDevice* container = GetDeviceHandle(allocation->deviceIndex);

	DestroyResourceStatus(allocation->resourceStatus);

	DestoryDriverBufferView(container, allocation->viewIndex);

	CleanInitializeAllocation(allocation);

	allocations.Free(handle);
}

void RenderInstance::DestroyDescriptorManager(ShaderResourceManagerIndex& handle)
{
	ShaderResourceManager* descriptorManager = descriptorManagers.Get(handle);

	if (!descriptorManager)
	{
		return;
	}

	storageAllocator->Free(descriptorManager->descriptorSetHandles.freeList);
	storageAllocator->Free(descriptorManager->descriptorSetHandles.pool);
	storageAllocator->Free(descriptorManager->descriptorSets);

	if (descriptorManager->deviceIndex.index >= 0)
	{
		RHIDevice* container = GetDeviceHandle(descriptorManager->deviceIndex);

		DestroyDriverDescriptorHeap(container, descriptorManager->deviceResourceHeap);
	}

	CleanInitializeDescriptorManager(descriptorManager);

	descriptorManagers.Free(handle);
}

void RenderInstance::DestroyGpuCommandStream(GPUCommandStreamIndex& handle)
{	
	GPUCommandStreamAllocator* gpuCommandStream = gpuCommandStreams.Get(handle);

	if (!gpuCommandStream)
	{
		return;
	}

	storageAllocator->Free(gpuCommandStream->commands);

	CleanInitializeGpuCommandStream(gpuCommandStream);

	gpuCommandStreams.Free(handle);
}

void RenderInstance::DestroyShaderGraph(RenderShaderGraphIndex& handle)
{	
	ShaderGraph* graph = shaderGraphs.shaderGraphPtrs.Get(handle);
	
	if (!graph)
	{
		return;
	}

	RHIDevice* container = GetDeviceHandle(graph->deviceIndex);

	int shaderCount = graph->shaderMapCount;

	for (int i = 0; i < shaderCount; i++)
	{
		int index = -1;

		if ((index = graph->shaderMaps[i].shaderReference) >= 0)
		{
			EntryHandle handle = shaderGraphs.shaderDetails.Get(index)->shaderHandle;

			if (EntryHandle() != handle)
			{
				DestroyDriverShader(container, handle);
			}

			shaderGraphs.shaderDetails.Free(index);
		}
	}

	int resourceCount = graph->resourceSetCount;

	for (int i = 0; i < resourceCount; i++)
	{
		ShaderResourceSetTemplate* set = &graph->shaderResourceSetTemplates[i];

		if (ShaderResourceTemplateInstanceIndex() != set->vulkanDescLayout)
		{
			DestroyShaderResourceTemplate(set->vulkanDescLayout);
		}
	}

	CleanInitializeShaderGraph(graph);

	shaderGraphs.shaderGraphPtrs.Free(handle);
}

void RenderInstance::DestroyGraphPipelineDescription(GeneratedPipelineInstanceIndex& handle)
{
	GraphPipelineDescription* desc = graphPipelineDescriptions.Get(handle);

	if (!desc)
	{
		return;
	}

	if (RenderDeviceIndex() != desc->instanceData.deviceIndex)
	{
		RHIDevice* container = GetDeviceHandle(desc->instanceData.deviceIndex);

		for (int i = 0; i < desc->instanceData.pipelineCount; i++)
		{
			DestroyDriverPipelineHandle(container, desc->pipelineIndices[i]);
		}
	}

	CleanInitializeGraphPipeline(desc);

	graphPipelineDescriptions.Free(handle);
}

void RenderInstance::CleanInitializePhysicalDeviceIndices(RenderPhysicalDeviceContainer* physicalDevice)
{
	physicalDevice->physicalDeviceIndex = EntryHandle();
	physicalDevice->internalDriverDeviceListIdentifier = -1;
	physicalDevice->information = {};
}

void RenderInstance::CleanInitializeRHIDevice(RHIDevice* logicalDevice)
{
	logicalDevice->device = nullptr;

	for (int i = 0; i < MAX_INSTANCE_FRAME_IN_FLIGHT; i++)
	{
		logicalDevice->container.currentCommandBufferIndex[i] = EntryHandle();
		logicalDevice->container.stagingBuffers[i] = EntryHandle();
		logicalDevice->container.stagingBufferAllocators[i].dataSize = 0;
		logicalDevice->container.stagingBufferAllocators[i].dataAllocator = 0;
	}

	logicalDevice->container.logicalDeviceIndex = EntryHandle();
	logicalDevice->container.presentQueue = EntryHandle();
	logicalDevice->container.queryPoolIndex = EntryHandle();
	logicalDevice->container.graphicsComputeTransfer = EntryHandle();
	logicalDevice->container.deviceTimelineSyncObject.currentValue = 0;
	logicalDevice->container.deviceTimelineSyncObject.driverTimelineObject = EntryHandle();
	logicalDevice->container.relatedPhysDeviceInfo = nullptr;
	logicalDevice->container.maxQueryResults = 0;
	logicalDevice->container.queriesAreActive = 0;
}

void RenderInstance::CleanInitializeWindowsSurface(RenderWindowSpecificData* windowSurface)
{
	*windowSurface = {};
}

void RenderInstance::CleanInitializeSwapChain(RenderSwapchainData* swapChain)
{
	swapChain->swapChainIdx = EntryHandle();
	swapChain->width = 0;
	swapChain->height = 0;
	swapChain->imageCount = 0;

	for (int i = 0; i < MAX_INSTANCE_FRAME_IN_FLIGHT; i++)
	{
		swapChain->rendererFinishedSemaphores[i] = EntryHandle();
	}

	for (int i = 0; i < MAX_SWC_IMAGE_COUNT; i++)
	{
		swapChain->rendererWaitSemaphores[i] = EntryHandle();
		swapChain->textureIds[i] = -1;
	}
}

void RenderInstance::CleanInitializeBufferHandle(RenderBufferDescription* bufferHandle)
{
	bufferHandle->bufferHandle = EntryHandle();
	bufferHandle->type = 0;
	bufferHandle->deviceIndex = -1;
}

void RenderInstance::CleanInitializeImagePool(ImagePoolDescription* imagePool)
{
	imagePool->deviceIndex = -1;
	imagePool->imagePoolType = 0;
	imagePool->imagePoolHandle = EntryHandle();
	imagePool->imagePoolSize = 0;
}

void RenderInstance::CleanInitializePipelineHandle(PipelineHandle* pipelineHandle)
{
	*pipelineHandle = {};
	pipelineHandle->pipelineIdentifierGroup = -1;
	pipelineHandle->vertexBufferHandle = -1;
	pipelineHandle->indexBufferHandle = -1;
	pipelineHandle->indirectBufferHandle = -1;
	pipelineHandle->indirectCountBufferHandle = -1;
	pipelineHandle->indirectDispatchCommandHandle = -1;
	pipelineHandle->indirectBufferHandle = -1;

	for (int i = 0; i < 16; i++)
	{
		pipelineHandle->resourceSets[i].descriptorManagerIndex = -1;
		pipelineHandle->resourceSets[i].descriptorSetIndex = -1;
	}
}

void RenderInstance::CleanInitializeAttachmentGraph(AttachmentGraph* attachmentGraph)
{
	attachmentGraph->passesCount = 0;
	attachmentGraph->resourceCount = 0;

	for (int i = 0; i < MAX_GRAPH_RESOURCES; i++)
	{
		attachmentGraph->resources[i].format = ImageFormat::IMAGE_UNKNOWN;
		attachmentGraph->resources[i].msaa = 0;
		attachmentGraph->resources[i].viewType = AttachmentViewType::STATIC;
	}

	for (int i = 0; i < MAX_GRAPH_RENDER_PASSES; i++)
	{
		attachmentGraph->holders[i].attachmentCount = 0;
		attachmentGraph->holders[i].colorCount = 0;
		attachmentGraph->holders[i].depthStencilCount = 0;
		attachmentGraph->holders[i].resolveCount = 0;

		for (int j = 0; j < MAX_RENDER_PASS_DESCRIPTIONS; j++)
		{
			attachmentGraph->holders[i].descs[j].attachType = 0;
			attachmentGraph->holders[i].descs[j].dstLayout = ImageLayout::UNDEFINED;
			attachmentGraph->holders[i].descs[j].srcLayout = ImageLayout::UNDEFINED;
			attachmentGraph->holders[i].descs[j].resourceIndex = -1;
			attachmentGraph->holders[i].descs[j].loadOp = AttachmentLoadUsage::ATTACHNOCARE;
			attachmentGraph->holders[i].descs[j].storeOp = AttachmentStoreUsage::ATTACHDISCARD;
		}
	}
}

void RenderInstance::CleanInitializeAttachmentGraphsInstance(AttachmentGraphInstance* attachmentGraphInstance)
{
	attachmentGraphInstance->graphLayout = nullptr;

	for (int i = 0; i < MAX_GRAPH_RESOURCES; i++)
	{
		for (int g = 0; g < MAX_SAMPLE_COUNT_LEVEL; g++)
		{
			for (int k = 0; k < MAX_RESOURCE_IMAGES; k++)
			{
				attachmentGraphInstance->resources[i].textureIds[g][k] = -1;
			}
		}

		attachmentGraphInstance->resources[i].imageCount = 0;
		attachmentGraphInstance->resources[i].sampHi = 0;
		attachmentGraphInstance->resources[i].sampLo = 0;
		attachmentGraphInstance->resources[i].usage = 0;
		
	}

	for (int i = 0; i < MAX_GRAPH_RENDER_PASSES; i++)
	{
		attachmentGraphInstance->passes[i].attachInstCount = 0;
		for (int j = 0; j < MAX_SAMPLE_COUNT_LEVEL; j++)
		{
			attachmentGraphInstance->passes[i].baseRenderPass[j] = -1;
			attachmentGraphInstance->passes[i].baseRenderTarget[j] = -1;
		}
		attachmentGraphInstance->passes[i].maxSampleCount = 0;
		attachmentGraphInstance->passes[i].graphicsOTQIndex = -1;
		attachmentGraphInstance->passes[i].rpType = RenderPassType::SWAPCHAIN_IMAGE_COUNT;
		attachmentGraphInstance->passes[i].currentSampleCount = 0;

		for (int j = 0; j < MAX_RENDER_PASS_DESCRIPTIONS; j++)
		{
			attachmentGraphInstance->passes[i].attachInst[j].attachmentResource = -1;
			attachmentGraphInstance->passes[i].attachInst[j].clear = {};
			attachmentGraphInstance->passes[i].attachInst[j].descLayout = nullptr;
		}
	}
}

void RenderInstance::CleanInitializeRenderTargetQueue(RenderQueue* renderTargetQueue)
{
	renderTargetQueue->queueCount = 0;
	memset(renderTargetQueue->pipelines, -1, sizeof(int) * 63);
}

void RenderInstance::CleanInitializeComputeQueue(ComputeQueue* computeQueue)
{
	computeQueue->queueCount = 0;
	memset(computeQueue->pipelines, -1, sizeof(int) * 63);
}

void RenderInstance::CleanInitializeTextureResourceHandle(RenderTextureDescription* textureResourceHandle)
{
	*textureResourceHandle = {};
	
	textureResourceHandle->textureIndex = EntryHandle();
	
	textureResourceHandle->resourceStatusIndex = -1;

	textureResourceHandle->deviceIndex = -1;

	textureResourceHandle->format = ImageFormat::IMAGE_UNKNOWN;
	
	for (int i = 0; i<MAX_VIEWS_ATTACHED_TO_TEXTURE; i++)
		textureResourceHandle->viewIndex[i] = -1;
}

void RenderInstance::CleanInitializeTextureViewsResourceHandle(RenderImageViewDescription* textureViewsResourceHandle)
{
	*textureViewsResourceHandle = {};
	textureViewsResourceHandle->desiredLayoutForView = ImageLayout::UNDEFINED;
	textureViewsResourceHandle->viewIndex = EntryHandle();
}

void RenderInstance::CleanInitializeResourceStatus(ResourceStatus* resourceStatus)
{
	*resourceStatus = {};
}

void RenderInstance::CleanInitializePipelineInfo(GenericPipelineStateInfo* pipelineInfo)
{
	*pipelineInfo = {};
	pipelineInfo->depthFormat = ImageFormat::IMAGE_UNKNOWN;
	pipelineInfo->colorFormat = ImageFormat::IMAGE_UNKNOWN;
}

void RenderInstance::CleanInitializeAllocation(RenderAllocation* allocation)
{
	*allocation = {};
	allocation->viewIndex = EntryHandle();
	allocation->memIndex = -1;
	allocation->parentAllocation = -1;
	allocation->resourceStatus = -1;
	allocation->formatType = ComponentFormatType::NO_BUFFER_FORMAT;
}

void RenderInstance::CleanInitializeDescriptorManager(ShaderResourceManager* descriptorManager)
{
	*descriptorManager = {};
	descriptorManager->deviceResourceHeap = EntryHandle();
}

void RenderInstance::CleanInitializeGpuCommandStream(GPUCommandStreamAllocator* gpuCommandStream)
{
	*gpuCommandStream = {};
}

void RenderInstance::CleanInitializeShaderGraph(ShaderGraph* shaderGraph)
{
	*shaderGraph = {};

	for (int i = 0; i < MAX_SHADER_MAPS; ++i)
	{
		shaderGraph->shaderMaps[i].shaderReference = -1;
	}

	for (int i = 0; i < MAX_SHADER_RESOURCES; ++i)
	{
		shaderGraph->shaderResources[i].rangeIndex = -1;
	}

	for (int i = 0; i < MAX_SHADER_RESOURCE_SET_TEMPLATES; ++i)
	{
		shaderGraph->shaderResourceSetTemplates[i].vulkanDescLayout = -1;
		shaderGraph->shaderResourceSetTemplates[i].dx12DescriptorTable = -1;
		shaderGraph->shaderResourceSetTemplates[i].resourceStart = -1;
	}
}

void  RenderInstance::CleanInitializeGraphPipeline(GraphPipelineDescription* desc)
{
	desc->instanceData.frameGraphCount = 0;
	desc->instanceData.pipelineCount = 0;

	memset(desc->instanceData.frameGraphIndices, -1, sizeof(int) * MAX_FRAME_GRAPHS_RENDER_PASS_COMBOS);
	memset(desc->instanceData.frameGraphRenderPasses, -1, sizeof(int) * MAX_FRAME_GRAPHS_RENDER_PASS_COMBOS);
	memset(desc->instanceData.frameGraphPipelineIndices, -1, sizeof(int) * MAX_FRAME_GRAPHS_RENDER_PASS_COMBOS);
}