#pragma once
#include <atomic>
#include <stdint.h>
#include <stddef.h>

#ifdef _MSC_VER
#define ALIGNAS(x) __declspec(align(x))
#elif defined(__GNUC__)
#define ALIGNAS(x) __attribute__((aligned(x)))
#endif

#define OS_PLATFORM_CACHE_SIZE 128 //potentially conservative

struct MPMCQueueData
{
    std::atomic<size_t> currentSequence;
    int freeIndex;
};