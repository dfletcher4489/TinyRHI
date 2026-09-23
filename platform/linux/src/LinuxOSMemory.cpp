#include "OSMemory.h"

#include <linux/mman.h>
#include <sys/mman.h>
 
#include <unistd.h>

#define BLOCK_HEADER_SENTINEL_VALUE 0xbbadbeefbbadbeef

#define MAX_DETAILS_PROTECTION 255
#define MAX_DETAILS_ALLOC_TYPE 255
#define MAX_ALLOCATION_INDEX  65535

#define BLOCK_DETAILS_PROTECTION_OFFSET 0
#define BLOCK_DETAILS_ALLOC_TYPE_OFFSET 8
#define BLOCK_DETAILS_ALLOCATION_INDEX_OFFSET 16

#define MAKE_BLOCK_DETAILS(allocIndex, prot, allocType) ((allocIndex << BLOCK_DETAILS_ALLOCATION_INDEX_OFFSET) | (prot << BLOCK_DETAILS_PROTECTION_OFFSET) | (allocType << BLOCK_DETAILS_ALLOC_TYPE_OFFSET))

#define GET_BLOCK_DETAILS_ALLOCATION_INDEX(packed) ((packed >> BLOCK_DETAILS_ALLOCATION_INDEX_OFFSET) & MAX_ALLOCATION_INDEX)
#define GET_BLOCK_DETAILS_ALLOCATION_TYPE(packed) ((packed >> BLOCK_DETAILS_ALLOC_TYPE_OFFSET) & MAX_DETAILS_ALLOC_TYPE)
#define GET_BLOCK_DETAILS_PROTECTION(packed) ((packed >> BLOCK_DETAILS_PROTECTION_OFFSET) & MAX_DETAILS_PROTECTION)

struct MemBlockHeader
{
    size_t blockCommitSentinel;
	size_t blockDetails;
	size_t blockSize; 
    size_t blockCommitSize;
};

static MPMCQueueData* freeList;
static void** memoryLocations;
static int maxFreeListEntry = 0;

ALIGNAS(128) static std::atomic<int> boundedLinearAllocator{ 0 };
ALIGNAS(128) static std::atomic<size_t> enqueuePos{ 0 };
ALIGNAS(128) static std::atomic<size_t> dequeuePos{ 0 };

static int PopFromFreeList()
{
    MPMCQueueData* cell;

    size_t pos = dequeuePos.load(std::memory_order_relaxed);

    for (;;)
    {
        cell = &freeList[pos % maxFreeListEntry];
        size_t seq = cell->currentSequence.load(std::memory_order_acquire);
        intptr_t diff = (intptr_t)seq - (intptr_t)(pos + 1);
        if (diff == 0)
        {
            if (dequeuePos.compare_exchange_weak(pos, pos + 1, std::memory_order_relaxed, std::memory_order_relaxed))
                break;
        }
        else if (diff < 0)
            return -1;
        else
            pos = dequeuePos.load(std::memory_order_relaxed);
    }

    cell->currentSequence.store(pos + maxFreeListEntry, std::memory_order_release);

    int freeListIndex = cell->freeIndex;

    cell->freeIndex = -1;

    return freeListIndex;
}

static void ReturnIndex(int index)
{
    MPMCQueueData* cell;

    size_t pos = enqueuePos.load(std::memory_order_relaxed);

    for (;;)
    {
        cell = &freeList[pos % maxFreeListEntry];
        size_t seq = cell->currentSequence.load(std::memory_order_acquire);
        intptr_t diff = (intptr_t)seq - (intptr_t)pos;
        if (diff == 0)
        {
            if (enqueuePos.compare_exchange_weak(pos, pos + 1, std::memory_order_relaxed))
                break;
        }
        else if (diff < 0)
            return;
        else
            pos = enqueuePos.load(std::memory_order_relaxed);
    }

    memoryLocations[index] = nullptr;

    cell->freeIndex = index;
    cell->currentSequence.store(pos + 1, std::memory_order_release);
}

static int FindFreeIndex()
{
    int ret = PopFromFreeList();

    if (ret < 0)
    {
        int linearTop = boundedLinearAllocator.load(std::memory_order_acquire);

        while (linearTop < maxFreeListEntry)
        {
            if (boundedLinearAllocator.compare_exchange_weak(linearTop, linearTop + 1, std::memory_order_relaxed, std::memory_order_relaxed))
            {
                ret = linearTop;
                break;
            }
        }
    }

    return ret;
}

static int ConvertMemoryAllocationType(OSMemoryAllocationType allocType)
{
	int ret = 0;

	ret |= (MAP_HUGETLB | MAP_HUGE_2MB) * ((allocType & OSMemoryAllocationTypes::USE_LARGE_PAGES) != 0);

	return ret;
}

static int ConvertMemoryProtection(OSMemoryAllocationProtection protection)
{
    int ret = PROT_NONE;

    if (protection == OSMemoryAllocationProtections::READONLY)
    {
       ret = PROT_READ;
    }

    if (protection == OSMemoryAllocationProtections::READWRITE)
    {
        ret = PROT_READ | PROT_WRITE;
    }

    if (protection == OSMemoryAllocationProtections::EXECUTE)
    {
        ret = PROT_EXEC;
    }

    if (protection == (OSMemoryAllocationProtections::EXECUTE | OSMemoryAllocationProtections::READONLY))
    {
        ret = PROT_EXEC | PROT_READ;
    }

    if (protection == (OSMemoryAllocationProtections::EXECUTE | OSMemoryAllocationProtections::READWRITE))
    {
        ret = PROT_EXEC | PROT_READ | PROT_WRITE;
    }

    return ret;
}

OSMemoryRequirements OSGetMemoryRequirements(int maxNumberOfAllocations)
{
    int handlesSize = (maxNumberOfAllocations) * sizeof(void*);
    int freeListSize = (maxNumberOfAllocations) * sizeof(MPMCQueueData);

    OSMemoryRequirements memReqs{ handlesSize + freeListSize, alignof(void*) };

    return memReqs;
}

int OSSeedMemory(void* dataSource, int dataSize, int maxNumberOfAllocations)
{
    uintptr_t dataHead = (uintptr_t)dataSource;

    memoryLocations = (void**)dataSource;

    int handleSize = maxNumberOfAllocations;

    dataHead += sizeof(void*) * handleSize;

    freeList = (MPMCQueueData*)dataHead;

    for (int i = 0; i < handleSize; i++)
    {
        freeList[i].currentSequence.store(i, std::memory_order_relaxed);
        memoryLocations[i] = nullptr;
    }

    maxFreeListEntry = handleSize;

    return OS_MEMORY_SUCCESS;
}

void ReleaseAllMemoryAllocations()
{
    for (int i = 0; i < maxFreeListEntry; i++)
    {
        if (memoryLocations[i])
        {
            MemBlockHeader* header = ((MemBlockHeader*)memoryLocations[i]);

            uint64_t pageSize = OSGetStandardPageSize();

            if (GET_BLOCK_DETAILS_ALLOCATION_TYPE(header->blockDetails) & OSMemoryAllocationTypes::USE_LARGE_PAGES)
            {
                pageSize = OSGetLargePageSize();
            }

            uintptr_t absoluteMemAddr = ((uintptr_t)memoryLocations[i]) - (pageSize-sizeof(MemBlockHeader));

            munmap((void*)absoluteMemAddr, header->blockSize);

            memoryLocations[i] = nullptr;
        }

        freeList[i].currentSequence.store(i, std::memory_order_relaxed);
    }

    enqueuePos.store(0, std::memory_order_relaxed);
    dequeuePos.store(0, std::memory_order_relaxed);
    boundedLinearAllocator.store(0, std::memory_order_relaxed);
}

uint64_t OSGetStandardPageSize()
{
	return (uint64_t)sysconf(_SC_PAGESIZE);
}

uint64_t OSGetLargePageSize()
{
	return 2 * 1024 * 1024;
}

void* OSMemoryAllocate(void* startingAddress, uint64_t size, OSMemoryAllocationType allocType, OSMemoryAllocationProtection protection)
{
    uint64_t pageSize;

    if (allocType & OSMemoryAllocationTypes::USE_LARGE_PAGES)
    {
        if (!(allocType & OSMemoryAllocationTypes::COMMIT))
        {
            return nullptr;
        }

        pageSize = OSGetLargePageSize();
    }
    else
    {
        pageSize = OSGetStandardPageSize();
    }

    int index = -1;

    MemBlockHeader* blockHeader = nullptr;

    void* retAddr = nullptr;

    uint64_t adjustedSize = size;

    int toCommit = allocType & OSMemoryAllocationTypes::COMMIT;

    int toReserve = allocType & OSMemoryAllocationTypes::RESERVE;

    if (!startingAddress || toReserve)
    {
        adjustedSize = ((adjustedSize + sizeof(MemBlockHeader) + pageSize) + (pageSize - 1)) & ~(pageSize - 1);

        index = FindFreeIndex();

        if (index < 0)
        {
            return retAddr;
        }

        if (!toCommit)
        {
            retAddr = mmap(startingAddress, adjustedSize, PROT_NONE, MAP_ANONYMOUS | MAP_PRIVATE | ConvertMemoryAllocationType(allocType), -1, 0);
        }
        else
        {
            retAddr = mmap(startingAddress, adjustedSize, ConvertMemoryProtection(protection), MAP_ANONYMOUS | MAP_PRIVATE | ConvertMemoryAllocationType(allocType), -1, 0);
        }

        if (retAddr == MAP_FAILED)
        {
            ReturnIndex(index);
            return nullptr;
        }
    }
    else
    {
        MemBlockHeader* potentialBlockHeader = ((MemBlockHeader*)startingAddress) - 1;

        size_t currentCommitHeader = 0;

        if (potentialBlockHeader->blockCommitSentinel != BLOCK_HEADER_SENTINEL_VALUE)
        {
            return nullptr;    
        }
        
        currentCommitHeader = potentialBlockHeader->blockCommitSize;

        adjustedSize = (adjustedSize + (pageSize - 1)) & ~(pageSize - 1);

        retAddr = mmap((void*)(((uintptr_t)startingAddress - pageSize) + currentCommitHeader), adjustedSize, ConvertMemoryProtection(protection), MAP_ANONYMOUS | MAP_PRIVATE | MAP_FIXED | ConvertMemoryAllocationType(allocType), -1, 0);

        if (retAddr == MAP_FAILED)
        {
            return nullptr;
        }
  
        potentialBlockHeader->blockCommitSize += adjustedSize;

        return retAddr;
    }

    if (toReserve && !toCommit)
    {
        int retCode = mprotect((void*)retAddr, pageSize, ConvertMemoryProtection(protection));

        if (retCode)
        {
            munmap(retAddr, adjustedSize);
            ReturnIndex(index);
            return nullptr;
        }
    }

    blockHeader = (MemBlockHeader*)(((uintptr_t)retAddr) + pageSize-sizeof(MemBlockHeader));

    blockHeader->blockCommitSentinel = BLOCK_HEADER_SENTINEL_VALUE;
    blockHeader->blockDetails = MAKE_BLOCK_DETAILS(index, protection, allocType);
    blockHeader->blockSize = adjustedSize;
    blockHeader->blockCommitSize = toCommit ? adjustedSize : pageSize;

    memoryLocations[index] = blockHeader;

    return blockHeader + 1;
}

int OSMemoryRelease(void* memAddr, uint64_t size, OSMemoryReleaseTypes freeType)
{
    MemBlockHeader* header = ((MemBlockHeader*)memAddr) - 1;

    if (header->blockCommitSentinel != BLOCK_HEADER_SENTINEL_VALUE)
    {
        return OS_MEMORY_FREE_FAILURE;
    }

    uint64_t pageSize = OSGetStandardPageSize();

    if (GET_BLOCK_DETAILS_ALLOCATION_TYPE(header->blockDetails) & OSMemoryAllocationTypes::USE_LARGE_PAGES)
    {
        pageSize = OSGetLargePageSize();
    }

    int index = GET_BLOCK_DETAILS_ALLOCATION_INDEX(header->blockDetails);

    uintptr_t absoluteMemAddr = ((uintptr_t)memAddr) - pageSize;

    size_t headerCommitSize = header->blockCommitSize;

    if (freeType == OSMemoryReleaseTypes::RELEASE)
    {
        int retCode = munmap((void*)absoluteMemAddr, header->blockSize);

        if (retCode)
        {
            return OS_MEMORY_FREE_FAILURE;
        }

        ReturnIndex(index);

        return OS_MEMORY_SUCCESS;
    }

    size = (size + (pageSize - 1)) & ~(pageSize - 1);

    if ((headerCommitSize - pageSize) < size)
    {
        return OS_MEMORY_FREE_FAILURE;
    }

    absoluteMemAddr += (headerCommitSize - size);

    int retCode = madvise((void*)absoluteMemAddr, size, MADV_DONTNEED);

    if (retCode)
    {
        return OS_MEMORY_FREE_FAILURE;
    }

    retCode = mprotect((void*)absoluteMemAddr, size, PROT_NONE); //Make the decommitted range inaccessible. Physical backing has already been discarded with MADV_DONTNEED.

    if (retCode)
    {
        return OS_MEMORY_FREE_FAILURE;
    }

    header->blockCommitSize -= size;
    
	return OS_MEMORY_SUCCESS;
}