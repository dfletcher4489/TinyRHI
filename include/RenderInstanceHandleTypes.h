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

struct ImageMemoryPoolIndex
{
	int index = -1;

	ImageMemoryPoolIndex() = default;

	ImageMemoryPoolIndex operator=(const int val)
	{
		this->index = val;
		return *this;
	}

	constexpr bool operator==(const ImageMemoryPoolIndex& val) const
	{
		return val.index == this->index;
	}
};

struct AttachmentGraphInstanceIndex
{
	int index = -1;

	AttachmentGraphInstanceIndex() = default;

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