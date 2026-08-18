#pragma once

struct RenderPhysicalDeviceIndex
{
	int index = -1;

	RenderPhysicalDeviceIndex operator=(const int val)
	{
		this->index = val;
		return *this;
	}

	bool operator==(const RenderPhysicalDeviceIndex& val) const
	{
		return val.index == this->index;
	}
};

struct RenderDeviceIndex
{
	int index = -1;

	RenderDeviceIndex operator=(const int val)
	{
		this->index = val;
		return *this;
	}

	bool operator==(const RenderDeviceIndex& val) const
	{
		return val.index == this->index;
	}
};

struct WindowIndex
{
	int index = -1;

	WindowIndex operator=(const int val)
	{
		this->index = val;
		return *this;
	}

	bool operator==(const WindowIndex& val) const
	{
		return val.index == this->index;
	}
};

struct ImageMemoryIndex
{
	int index = -1;

	ImageMemoryIndex() = default;
	ImageMemoryIndex(int val)
		: index(val)
	{

	}

	ImageMemoryIndex operator=(const int val)
	{
		this->index = val;
		return *this;
	}

	constexpr bool operator==(const ImageMemoryIndex& val) const
	{
		return val.index == this->index;
	}
};

struct AttachmentGraphInstanceIndex
{
	int index = -1;

	AttachmentGraphInstanceIndex() = default;
	AttachmentGraphInstanceIndex(int val)
		: index(val)
	{

	}

	AttachmentGraphInstanceIndex operator=(const int val)
	{
		this->index = val;
		return *this;
	}

	constexpr bool operator==(const AttachmentGraphInstanceIndex& val) const
	{
		return val.index == this->index;
	}
};

struct SwapChainIndex
{
	int index = -1;

	SwapChainIndex() = default;
	SwapChainIndex(int val)
		: index(val)
	{

	}

	SwapChainIndex operator=(const int val)
	{
		this->index = val;
		return *this;
	}

	constexpr bool operator==(const SwapChainIndex& val) const
	{
		return val.index == this->index;
	}
};

struct BufferMemoryIndex
{
	int index = -1;

	BufferMemoryIndex() = default;
	BufferMemoryIndex(int val)
		: index(val)
	{

	}

	BufferMemoryIndex operator=(const int val)
	{
		this->index = val;
		return *this;
	}

	constexpr bool operator==(const BufferMemoryIndex& val) const
	{
		return val.index == this->index;
	}
};

struct ShaderResourceManagerIndex
{
	int index = -1;

	ShaderResourceManagerIndex() = default;
	ShaderResourceManagerIndex(int val)
		: index(val)
	{

	}

	ShaderResourceManagerIndex operator=(const int val)
	{
		this->index = val;
		return *this;
	}

	constexpr bool operator==(const ShaderResourceManagerIndex& val) const
	{
		return val.index == this->index;
	}
};

struct SamplerIndex
{
	int index = -1;

	SamplerIndex() = default;
	SamplerIndex(int val)
		: index(val)
	{

	}

	SamplerIndex operator=(const int val)
	{
		this->index = val;
		return *this;
	}

	constexpr bool operator==(const SamplerIndex& val) const
	{
		return val.index == this->index;
	}
};

struct TextureIndex
{
	int index = -1;

	TextureIndex() = default;
	TextureIndex(int val)
		: index(val)
	{

	}

	TextureIndex operator=(const int val)
	{
		this->index = val;
		return *this;
	}

	constexpr bool operator==(const TextureIndex& val) const
	{
		return val.index == this->index;
	}
};

struct ResourceIndex
{
	int index = -1;

	ResourceIndex() = default;
	ResourceIndex(int val)
		: index(val)
	{

	}

	ResourceIndex operator=(const int val)
	{
		this->index = val;
		return *this;
	}

	constexpr bool operator==(const ResourceIndex& val) const
	{
		return val.index == this->index;
	}
};