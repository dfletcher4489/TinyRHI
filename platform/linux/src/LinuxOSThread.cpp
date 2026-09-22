#include "OSThread.h"
#include <atomic>

#include <pthread.h>

struct ThreadData
{
    ThreadPointer routine;
    void* argumentToThread;
    OSThreadFlags flags;
    int index;
};

static MPMCQueueData* freeList;
static pthread_t* handles;
static int* handleDefined;
static ThreadData* dataThreads;
static int maxFreeListEntry = 0;

ALIGNAS(128) static std::atomic<int> boundedLinearAllocator{ 0 };
ALIGNAS(128) static std::atomic<size_t> enqueuePos{ 0 };
ALIGNAS(128) static std::atomic<size_t> dequeuePos{ 0 };

static int InternalOSCloseThread(int index);

static void* ThreadFunction(void *arg);

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

    handleDefined[index] = 0;

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

OSThreadMemoryRequirements OSGetThreadMemoryRequirements(int maxNumberOfOpenThreads)
{
    int handlesSize = (maxNumberOfOpenThreads) * sizeof(pthread_t);
    int threadDataSize = (maxNumberOfOpenThreads) * sizeof(ThreadData);
    int freeListSize = (maxNumberOfOpenThreads) * sizeof(MPMCQueueData);
    int definedSize = (maxNumberOfOpenThreads) * sizeof(int);

    OSThreadMemoryRequirements memReqs{ handlesSize + threadDataSize + freeListSize + definedSize, alignof(pthread_t) };

    return memReqs;
}

int OSSeedThreadMemory(void* dataSource, int dataSize, int numberOfOpenThreads)
{
    uintptr_t dataHead = (uintptr_t)dataSource;

    int handleSize = numberOfOpenThreads;

    handles = (pthread_t*)dataSource;

    dataHead += sizeof(pthread_t) * handleSize;

    dataThreads = (ThreadData*)dataHead;

    dataHead += sizeof(ThreadData) * handleSize;

    freeList = (MPMCQueueData*)dataHead;

    dataHead += sizeof(MPMCQueueData) * handleSize;

    handleDefined = (int*)dataHead;

    for (int i = 0; i < handleSize; i++)
    {
        freeList[i].currentSequence.store(i, std::memory_order_relaxed);
        handleDefined[i] = 0;
    }

    maxFreeListEntry = handleSize;

    return OS_THREAD_SUCCESS;
}

int OSCreateThread(OSThreadHandle* handle, void* argumentToThread, ThreadPointer routine, OSThreadFlags flags)
{
    int index = FindFreeIndex();

    if (index < 0)
    {
        return OS_THREAD_HANDLE_EXHAUSTED;
    }

    dataThreads[index].argumentToThread = argumentToThread;
    dataThreads[index].flags = flags;
    dataThreads[index].index = index;
    dataThreads[index].routine = routine;

    pthread_attr_t attributes;

    pthread_attr_init(&attributes);

    if (flags == OS_THREAD_ASYNC)
    {
        pthread_attr_setdetachstate(&attributes, PTHREAD_CREATE_DETACHED);
    }

    void* argumentPassed = &dataThreads[index];

    int threadCreateRet = pthread_create(&handles[index], &attributes, ThreadFunction, argumentPassed);

    pthread_attr_destroy(&attributes);

    if (threadCreateRet)
    {
        return OS_THREAD_FAILED_CREATE;
    }

    handleDefined[index] = 1;

    handle->osDataHandle = index;

    return OS_THREAD_SUCCESS;
}

void CloseAllThreads()
{
    for (int i = 0; i < maxFreeListEntry; i++)
    {
        if (handleDefined[i])
        {
            if (dataThreads[i].flags != OS_THREAD_ASYNC)
            {
                pthread_join(handles[i], NULL);
            }
        }

        handleDefined[i] = 0;

        freeList[i].currentSequence.store(i, std::memory_order_relaxed);
    }

    enqueuePos.store(0, std::memory_order_relaxed);
    dequeuePos.store(0, std::memory_order_relaxed);
    boundedLinearAllocator.store(0, std::memory_order_relaxed);
}

int OSCloseThread(OSThreadHandle* handle)
{
    int index = handle->osDataHandle;

    if (index < 0 || index >= maxFreeListEntry)
    {
        return OS_THREAD_FAILED_CLOSE;
    }

    if (dataThreads[index].flags == OS_THREAD_ASYNC)
    {
        return OS_THREAD_FAILED_CLOSE;
    }

    pthread_join(handles[index], NULL);

    InternalOSCloseThread(index);

    handle->osDataHandle = -1;

    return OS_THREAD_SUCCESS;
}

int InternalOSCloseThread(int index)
{
    ReturnIndex(index);

    return OS_THREAD_SUCCESS;
}

int OSWaitThread(OSThreadHandle* handle, int timeout)
{
    int index = handle->osDataHandle;

    if (index < 0 || index >= maxFreeListEntry)
    {
        return OS_THREAD_FAILED_JOIN;
    }

    if (dataThreads[index].flags == OS_THREAD_ASYNC)
    {
        return OS_THREAD_FAILED_JOIN;
    }

    int ret = pthread_join(handles[index], NULL);

    return (ret ? OS_THREAD_FAILED_JOIN : OS_THREAD_SUCCESS);
}

void* ThreadFunction(void *arg) 
{
    ThreadData* data = (ThreadData*)arg;

    data->routine(data->argumentToThread);
    
    if (data->flags == OS_THREAD_ASYNC)
    {
        InternalOSCloseThread(data->index);
    }

    pthread_exit(NULL);

    return NULL; 
}