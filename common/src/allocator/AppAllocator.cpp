#include "allocator/AppAllocator.h"

#include <string.h>

void* Allocator::Head()
{
	return nullptr;
}

void Allocator::Free(void* memaddress)
{
	return;
}

void Allocator::Free(int allocSize)
{
	return;
}

void* RingAllocator::Allocate(int _allocSize, int alignment)
{
	char* head = (char*)dataHead;

	int out = UpdateAtomic(dataAllocator, _allocSize, dataSize, alignment);

	return (head + out);
}

void* RingAllocator::Allocate(int _allocSize)
{
	char* head = (char*)dataHead;

	int out = UpdateAtomic(dataAllocator, _allocSize, dataSize);

	return (head + out);
}

void* RingAllocator::CAllocate(int _allocSize, int alignment)
{
	char* head = (char*)dataHead;

	int out = UpdateAtomic(dataAllocator, _allocSize, dataSize, alignment);

	memset((head + out), 0, _allocSize);

	return (head + out);
}

void* RingAllocator::CAllocate(int _allocSize)
{
	char* head = (char*)dataHead;

	int out = UpdateAtomic(dataAllocator, _allocSize, dataSize);

	memset((head + out), 0, _allocSize);

	return (head + out);
}

void RingAllocator::Reset()
{
	dataAllocator = 0;
}

void* RingAllocator::Head()
{
	char* head = (char*)dataHead;
	return (void*)(head + dataAllocator);
}

StringView* RingAllocator::AllocateFromNullString(const char* name)
{
	StringView* view = (StringView*)Allocate(sizeof(StringView));

	int strLen = strnlen(name, 250);

	char* strBuf = (char*)Allocate(strLen);
	view->charCount = strLen;

	memcpy(strBuf, name, strLen);

	view->stringData = strBuf;

	return view;
}

StringView RingAllocator::AllocateFromNullStringCopy(const char* name)
{
	StringView view;

	int strLen = strnlen(name, 250);

	char* strBuf = (char*)Allocate(strLen);
	
	view.charCount = strLen;

	memcpy(strBuf, name, strLen);

	view.stringData = strBuf;

	return view;
}

void* RingAllocator::Realloc(void* memaddress, int allocSize)
{
	return Allocate(allocSize);
}

int RingAllocator::OffsetInAllocator(void* dataPtr)
{
	uintptr_t dataPtrInT = (uintptr_t)dataPtr;
	uintptr_t dataHeadInT = (uintptr_t)dataHead;

	if (dataPtrInT < dataHeadInT || dataPtrInT >= (dataHeadInT + dataSize))
	{
		logger->AddLogMessage(LOGERROR, allocatorName);
		logger->AddLogMessage(LOGERROR, STRING_VIEW_FROM_LITERAL("Ring Allocator: Offset into allocator overflow/underflow"));
		return -1;
	}

	
	return (int)(dataPtrInT - dataHeadInT);
}

std::pair<int, int> RingAllocator::GetUsageAndCapacity() const 
{
	return { dataAllocator.load(), dataSize };
}

void* SlabAllocator::Allocate(int _allocSize, int alignment)
{
	char* head = (char*)dataHead;

	if ((dataAllocator + _allocSize) > dataSize)
	{
		logger->AddLogMessage(LOGERROR, allocatorName);
		logger->AddLogMessage(LOGERROR, STRING_VIEW_FROM_LITERAL("Slab Allocator: Too big of allocation"));
		return nullptr;
	}

	int out = UpdateAtomic(dataAllocator, _allocSize, 0, alignment);

	return (head + out);
}

void* SlabAllocator::Allocate(int _allocSize)
{
	char* head = (char*)dataHead;

	if ((dataAllocator + _allocSize) > dataSize)
	{
		logger->AddLogMessage(LOGERROR, allocatorName);
		logger->AddLogMessage(LOGERROR, STRING_VIEW_FROM_LITERAL("Slab Allocator: Too big of allocation"));
		return nullptr;
	}

	int out = UpdateAtomic(dataAllocator, _allocSize, 0);

	return (head + out);
}

void* SlabAllocator::CAllocate(int _allocSize, int alignment)
{
	char* head = (char*)dataHead;

	if ((dataAllocator + _allocSize) > dataSize)
	{
		logger->AddLogMessage(LOGERROR, allocatorName);
		logger->AddLogMessage(LOGERROR, STRING_VIEW_FROM_LITERAL("Slab Allocator: Too big of allocation"));
		return nullptr;
	}

	int out = UpdateAtomic(dataAllocator, _allocSize, 0, alignment);

	memset((head + out), 0, _allocSize);

	return (head + out);
}

void* SlabAllocator::CAllocate(int _allocSize)
{
	char* head = (char*)dataHead;

	if ((dataAllocator + _allocSize) > dataSize)
	{
		logger->AddLogMessage(LOGERROR, allocatorName);
		logger->AddLogMessage(LOGERROR, STRING_VIEW_FROM_LITERAL("Slab Allocator: Too big of allocation"));
		return nullptr;
	}

	int out = UpdateAtomic(dataAllocator, _allocSize, 0);

	memset((head + out), 0, _allocSize);

	return (head + out);
}

void SlabAllocator::Reset()
{
	dataAllocator = 0;
}

void* SlabAllocator::Head()
{
	char* head = (char*)dataHead;
	return (void*)(head + dataAllocator);
}

StringView* SlabAllocator::AllocateFromNullString(const char* name)
{
	StringView* view = (StringView*)Allocate(sizeof(StringView));

	if (!view)
	{
		return view;
	}

	int strLen = strnlen(name, 250);
	
	char* strBuf = (char*)Allocate(strLen);

	if (!strBuf)
	{
		return nullptr;
	}

	view->charCount = strLen;

	memcpy(strBuf, name, strLen);

	view->stringData = strBuf;

	return view;
}

StringView SlabAllocator::AllocateFromNullStringCopy(const char* name)
{
	StringView view{};

	int strLen = strnlen(name, 250);

	char* strBuf = (char*)Allocate(strLen);

	if (!strBuf)
	{
		return view;
	}

	view.charCount = strLen;

	memcpy(strBuf, name, strLen);

	view.stringData = strBuf;

	return view;
}

int SlabAllocator::OffsetInAllocator(void* dataPtr)
{
	uintptr_t dataPtrInT = (uintptr_t)dataPtr;
	uintptr_t dataHeadInT = (uintptr_t)dataHead;

	if (dataPtrInT < dataHeadInT || dataPtrInT >= (dataHeadInT + dataSize))
	{
		logger->AddLogMessage(LOGERROR, allocatorName);
		logger->AddLogMessage(LOGERROR, STRING_VIEW_FROM_LITERAL("Slab Allocator: Offset in underflow/overflow"));
		return -1;
	}

	return (int)(dataPtrInT - dataHeadInT);
}

std::pair<int, int> SlabAllocator::GetUsageAndCapacity() const
{
	return { dataAllocator.load(), dataSize };
}

void* SlabAllocator::Realloc(void* memaddress, int allocSize)
{
	return Allocate(allocSize);
}

void SlabAllocator::Free(int _allocSize)
{
	char* head = (char*)dataHead;

	if ((dataAllocator - _allocSize) < 0)
	{
		_allocSize = dataAllocator.load();
	}

	int out = UpdateAtomic(dataAllocator, -_allocSize, 0);

	return;
}

int DeviceSlabAllocator::Allocate(int _allocSize, int alignment)
{
	int out = UpdateAtomic(dataAllocator, _allocSize, 0, alignment);

	if ((out + _allocSize) > dataSize)
	{
		logger->AddLogMessage(LOGERROR, allocatorName);
		logger->AddLogMessage(LOGERROR, STRING_VIEW_FROM_LITERAL("Device Slab Allocator: Too big of an allocation"));
		return -1;
	}

	return (out);
}

std::pair<int, int> DeviceSlabAllocator::GetUsageAndCapacity() const 
{
	return { dataAllocator.load(), dataSize };
}

void* TLSFAllocator::Allocate(int _allocSize, int alignment)
{
	void* ret = TLSFAllocate(&tlsf, _allocSize, alignment);

	if (!ret)
	{
		logger->AddLogMessage(LOGERROR, allocatorName);
		logger->AddLogMessage(LOGERROR, STRING_VIEW_FROM_LITERAL("TLSF Allocator: Allocation Failed"));
	}
	
	return ret;
}

void* TLSFAllocator::Allocate(int _allocSize)
{
	void* ret = TLSFAllocate(&tlsf, _allocSize);

	if (!ret)
	{
		logger->AddLogMessage(LOGERROR, allocatorName);
		logger->AddLogMessage(LOGERROR, STRING_VIEW_FROM_LITERAL("TLSF Allocator: Allocation Failed"));
	}

	return ret;
}

void* TLSFAllocator::CAllocate(int _allocSize, int alignment)
{
	void* alloc = TLSFAllocate(&tlsf, _allocSize, alignment);

	if (!alloc)
	{
		logger->AddLogMessage(LOGERROR, allocatorName);
		logger->AddLogMessage(LOGERROR, STRING_VIEW_FROM_LITERAL("TLSF Allocator: Allocation Failed"));
		return alloc;
	}

	memset(alloc, 0, _allocSize);

	return alloc;
}

void* TLSFAllocator::CAllocate(int _allocSize)
{
	void* alloc = TLSFAllocate(&tlsf, _allocSize);

	if (!alloc)
	{
		logger->AddLogMessage(LOGERROR, allocatorName);
		logger->AddLogMessage(LOGERROR, STRING_VIEW_FROM_LITERAL("TLSF Allocator: Allocation Failed"));
		return alloc;
	}

	memset(alloc, 0, _allocSize);

	return alloc;
}

void TLSFAllocator::Free(void* _allocPtr)
{
	TLSFFree(&tlsf, _allocPtr);
}

void* TLSFAllocator::Realloc(void* memaddress, int _allocSize)
{
	void* alloc = TLSFRealloc(&tlsf, memaddress, _allocSize);

	if (!alloc)
	{
		logger->AddLogMessage(LOGERROR, allocatorName);
		logger->AddLogMessage(LOGERROR, STRING_VIEW_FROM_LITERAL("TLSF Allocator: Reallocation Failed"));
		return alloc;
	}

	return alloc;
}

void TLSFAllocator::Reset()
{
	memset((void*)tlsf.memPool, 0, tlsf.totalMemPoolSize);
	TLSFInitialize(&tlsf, (void*)tlsf.memPool, tlsf.totalMemPoolSize);
}

StringView* TLSFAllocator::AllocateFromNullString(const char* name)
{
	StringView* view = (StringView*)Allocate(sizeof(StringView));

	if (!view)
	{
		return view;
	}

	int strLen = strnlen(name, 250);

	char* strBuf = (char*)Allocate(strLen);

	if (!strBuf)
	{
		return nullptr;
	}

	view->charCount = strLen;

	memcpy(strBuf, name, strLen);

	view->stringData = strBuf;

	return view;
}

StringView TLSFAllocator::AllocateFromNullStringCopy(const char* name)
{
	StringView view{};

	int strLen = strnlen(name, 250);

	char* strBuf = (char*)Allocate(strLen);

	if (!strBuf)
	{
		return view;
	}

	view.charCount = strLen;

	memcpy(strBuf, name, strLen);

	view.stringData = strBuf;

	return view;
}

int TLSFAllocator::OffsetInAllocator(void* dataPtr)
{
	uintptr_t dataPtrInT = (uintptr_t)dataPtr;
	uintptr_t dataHeadInT = (uintptr_t)tlsf.memPool;

	if (dataPtrInT < dataHeadInT || dataPtrInT >= (dataHeadInT + tlsf.totalMemPoolSize))
	{
		logger->AddLogMessage(LOGERROR, allocatorName);
		logger->AddLogMessage(LOGERROR, STRING_VIEW_FROM_LITERAL("TLSF Allocator: Offset into allocator failed"));
		return -1;
	}

	return (int)(dataPtrInT - dataHeadInT);
}

std::pair<int, int> TLSFAllocator::GetUsageAndCapacity() const
{
	return { 0, 0 };
}