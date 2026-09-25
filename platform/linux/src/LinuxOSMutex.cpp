#include "OSMutex.h"

#include <errno.h>
#include <pthread.h>
#include <semaphore.h>

#include <atomic>
#include <cstring>

enum LinuxHandleType
{
	SEMAPHORE_HANDLE = 1,
	SRWLOCK_HANDLE = 2,
};

union LinuxHandleUnion
{
	pthread_rwlock_t lock;
	sem_t semaphoreHandle;
};

static LinuxHandleUnion* handles;
static int* handleTypes;
static MPMCQueueData* freeList;
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

	switch (handleTypes[index])
	{
	case SEMAPHORE_HANDLE:
		//handles[index].semaphoreHandle = INVALID_HANDLE_VALUE;
		break;
	//case SRWLOCK_HANDLE:
		//handles[index].lock = SRWLOCK_INIT;
		//break;
	}

	handleTypes[index] = 0;

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

void CloseAllSyncObject()
{
	for (int idx = 0; idx < maxFreeListEntry; idx++)
	{
		if (handleTypes[idx] != 0)
		{
			switch (handleTypes[idx])
			{
			case SEMAPHORE_HANDLE:
				sem_destroy(&handles[idx].semaphoreHandle);
				break;
			case SRWLOCK_HANDLE:
				break;
			}

			handleTypes[idx] = 0;
		}

		freeList[idx].currentSequence.store(idx, std::memory_order_relaxed);
	}

	enqueuePos.store(0, std::memory_order_relaxed);
	dequeuePos.store(0, std::memory_order_relaxed);
	boundedLinearAllocator.store(0, std::memory_order_relaxed);
}

OSSyncMemoryRequirements OSGetSyncMemoryRequirements(int maxNumberOfOpenSyncObjects)
{
	int handlesSize = (maxNumberOfOpenSyncObjects) * sizeof(LinuxHandleUnion);
	int handlesTypeSize = (maxNumberOfOpenSyncObjects) * sizeof(int);
	int freeListSize = (maxNumberOfOpenSyncObjects) * sizeof(MPMCQueueData);

	OSSyncMemoryRequirements memReqs{ handlesSize + handlesTypeSize + freeListSize, alignof(MPMCQueueData) };
	
	return memReqs;
}

int OSSeedSyncMemory(void* dataSource, int dataSize, int maxNumberSyncObjects)
{
	uintptr_t dataHead = (uintptr_t)dataSource;
	uintptr_t dataStart = dataHead;

	int handleSize = maxNumberSyncObjects;

	freeList = (MPMCQueueData*)dataHead;

	dataHead += sizeof(MPMCQueueData) * handleSize;

	handles = (LinuxHandleUnion*)dataHead;

	dataHead += handleSize * sizeof(LinuxHandleUnion);

	handleTypes = (int*)dataHead;

	dataHead += sizeof(int) * handleSize;

	for (int i = 0; i < handleSize; i++)
	{
		freeList[i].currentSequence.store(i, std::memory_order_relaxed);
		handleTypes[i] = 0;
	}

	maxFreeListEntry = handleSize;

	return OS_SEMAPHORE_SUCCESS;
}

int CreateOSSemaphore(OSSemaphore* semaphore, int count)
{
	int osIndex = FindFreeIndex();

	if (osIndex < 0 || osIndex >= maxFreeListEntry)
	{
		return OS_SEMAPHORE_HANDLE_EXHAUSTED;
	}

	sem_t* semaphoreImpl = &handles[osIndex].semaphoreHandle;

	int retCode = sem_init(semaphoreImpl, 0, count);
	
	if (retCode)
	{
		ReturnIndex(osIndex);
		return OS_SEMAPHORE_CREATE_FAILED;
	}

	handleTypes[osIndex] = SEMAPHORE_HANDLE;

	semaphore->maxCount = count;
	semaphore->osDataHandle = osIndex;

	return OS_SEMAPHORE_SUCCESS;
}

int WaitOSSemaphore(OSSemaphore* semaphore, unsigned int waitMS)
{
	int osIndex = semaphore->osDataHandle;

	if (osIndex < 0 || osIndex >= maxFreeListEntry)
	{
		return OS_SEMAPHORE_HANDLE_OUT_OF_BOUNDS;
	}

	if (handleTypes[osIndex] != SEMAPHORE_HANDLE)
	{
		return OS_SEMAPHORE_WRONG_TYPE;
	}

	struct timespec timeoutSpec{};

	clock_gettime(CLOCK_REALTIME, &timeoutSpec);

    timeoutSpec.tv_sec += waitMS / 1000;
    timeoutSpec.tv_nsec += (waitMS % 1000) * 1000000;

    if (timeoutSpec.tv_nsec >= 1000000000)
    {
        timeoutSpec.tv_sec++;
        timeoutSpec.tv_nsec -= 1000000000;
    }

	sem_t* semaphoreImpl = &handles[osIndex].semaphoreHandle;

	int semRetCode = sem_timedwait(semaphoreImpl, &timeoutSpec);

	int ret = OS_SEMAPHORE_SUCCESS;

	if (semRetCode)
	{
		int waitResult = errno;

		switch (waitResult)
		{
		case ETIMEDOUT:
			ret = OS_SEMAPHORE_TIMEOUT_FAILED;
			break;
		default:
			ret = OS_SEMAPHORE_WAIT_FAILED;
			break;
		}
	}

	return ret;
}

int NotifyOSSemaphore(OSSemaphore* semaphore)
{
	int osIndex = semaphore->osDataHandle;

	if (osIndex < 0 || osIndex >= maxFreeListEntry)
	{
		return OS_SEMAPHORE_HANDLE_OUT_OF_BOUNDS;
	}

	if (handleTypes[osIndex] != SEMAPHORE_HANDLE)
	{
		return OS_SEMAPHORE_WRONG_TYPE;
	}

	sem_t* semaphoreImpl = &handles[osIndex].semaphoreHandle;

	if (sem_post(semaphoreImpl))       
	{
		return OS_SEMAPHORE_RELEASE_FAILED;
	}

	return OS_SEMAPHORE_SUCCESS;
}

int DeleteOSSemaphore(OSSemaphore* semaphore)
{
	int osIndex = semaphore->osDataHandle;

	if (osIndex < 0 || osIndex >= maxFreeListEntry)
	{
		return OS_SEMAPHORE_HANDLE_OUT_OF_BOUNDS;
	}

	if (handleTypes[osIndex] != SEMAPHORE_HANDLE)
	{
		return OS_SEMAPHORE_WRONG_TYPE;
	}

	sem_destroy(&handles[osIndex].semaphoreHandle);

	ReturnIndex(osIndex);

	memset(semaphore, -1, sizeof(OSSemaphore));

	return OS_SEMAPHORE_SUCCESS;
}

int CreateOSSharedExclusive(OSSharedExclusive* osse)
{
	int osIndex = FindFreeIndex();

	if (osIndex < 0)
	{
		return OS_SEMAPHORE_HANDLE_EXHAUSTED;
	}

	int retCode = pthread_rwlock_init(&handles[osIndex].lock, NULL);

	if (retCode)
	{
		ReturnIndex(osIndex);
		return OSSE_CREATE_FAILED;
	}

	osse->internalOSHandle = osIndex;
	handleTypes[osIndex] = SRWLOCK_HANDLE;

	return OSSE_SUCCESS;
}

int ExclusiveAcquireOSSharedExclusive(OSSharedExclusive* osse)
{
	int osIndex = osse->internalOSHandle;

	if (osIndex < 0 || osIndex >= maxFreeListEntry)
	{
		return OS_SEMAPHORE_HANDLE_OUT_OF_BOUNDS;
	}

	if (handleTypes[osIndex] != SRWLOCK_HANDLE)
	{
		return OSSE_WRONG_TYPE;
	}

	int retCode = pthread_rwlock_wrlock(&handles[osIndex].lock);

	if (retCode)
	{
		return OSSE_EXCLUSIVE_LOCK_ACQUIRE_FAILED;
	}

	return OSSE_SUCCESS;
}

int SharedAcquireOSSharedExclusive(OSSharedExclusive* osse)
{
	int osIndex = osse->internalOSHandle;

	if (osIndex < 0 || osIndex >= maxFreeListEntry)
	{
		return OS_SEMAPHORE_HANDLE_OUT_OF_BOUNDS;
	}

	if (handleTypes[osIndex] != SRWLOCK_HANDLE)
	{
		return OSSE_WRONG_TYPE;
	}

	int retCode = pthread_rwlock_rdlock(&handles[osIndex].lock);

	if (retCode)
	{
		return OSSE_SHARED_LOCK_ACQUIRE_FAILED;
	}

	return OSSE_SUCCESS;
}

int ExclusiveReleaseOSSharedExclusive(OSSharedExclusive* osse)
{
	int osIndex = osse->internalOSHandle;

	if (osIndex < 0 || osIndex >= maxFreeListEntry)
	{
		return OS_SEMAPHORE_HANDLE_OUT_OF_BOUNDS;
	}

	if (handleTypes[osIndex] != SRWLOCK_HANDLE)
	{
		return OSSE_WRONG_TYPE;
	}

	pthread_rwlock_unlock(&handles[osIndex].lock);

	return OSSE_SUCCESS;
}

int SharedReleaseOSSharedExclusive(OSSharedExclusive* osse)
{
	int osIndex = osse->internalOSHandle;

	if (osIndex < 0 || osIndex >= maxFreeListEntry)
	{
		return OS_SEMAPHORE_HANDLE_OUT_OF_BOUNDS;
	}

	if (handleTypes[osIndex] != SRWLOCK_HANDLE)
	{
		return OSSE_WRONG_TYPE;
	}

	pthread_rwlock_unlock(&handles[osIndex].lock);

	return OSSE_SUCCESS;
}

int TryExclusiveAcquireOSSharedExclusive(OSSharedExclusive* osse)
{
	int osIndex = osse->internalOSHandle;

	if (osIndex < 0 || osIndex >= maxFreeListEntry)
	{
		return OS_SEMAPHORE_HANDLE_OUT_OF_BOUNDS;
	}

	if (handleTypes[osIndex] != SRWLOCK_HANDLE)
	{
		return OSSE_WRONG_TYPE;
	}

	if (pthread_rwlock_trywrlock(&handles[osIndex].lock))
	{
		return OSSE_EXCLUSIVE_LOCK_ACQUIRE_FAILED;
	}

	return OSSE_SUCCESS;
}

int TrySharedAcquireOSSharedExclusive(OSSharedExclusive* osse)
{
	int osIndex = osse->internalOSHandle;

	if (osIndex < 0 || osIndex >= maxFreeListEntry)
	{
		return OS_SEMAPHORE_HANDLE_OUT_OF_BOUNDS;
	}

	if (handleTypes[osIndex] != SRWLOCK_HANDLE)
	{
		return OSSE_WRONG_TYPE;
	}

	if (pthread_rwlock_tryrdlock(&handles[osIndex].lock))
	{
		return OSSE_SHARED_LOCK_ACQUIRE_FAILED;
	}

	return OSSE_SUCCESS;
}

void DeleteOSSharedExclusive(OSSharedExclusive* osse)
{
	int osIndex = osse->internalOSHandle;

	if (osIndex < 0 || osIndex >= maxFreeListEntry)
	{
		return;
	}

	if (handleTypes[osIndex] != SRWLOCK_HANDLE)
	{
		return;
	}

	pthread_rwlock_destroy(&handles[osIndex].lock);

	ReturnIndex(osIndex);
}
