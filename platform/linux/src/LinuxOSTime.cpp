#include "OSTime.h"

#include <time.h>

bool initialize = false;

void OSTimeInitialize()
{
	initialize = true;
}

int64_t OSGetSystemTicks()
{
	if (!initialize) return 0;

	struct timespec currentTime;

	clock_gettime(CLOCK_MONOTONIC, &currentTime);

	return (int64_t)currentTime.tv_sec * 1000000000LL +
	       (int64_t)currentTime.tv_nsec;
}

double OSGetTimeSeconds(int64_t tickCounter)
{
	if (!initialize) return 0.0;

	return (double)tickCounter * 1e-9;
}

double OSGetCurrentTimeSeconds()
{
	if (!initialize) return 0.0;

	struct timespec currentTime;

	clock_gettime(CLOCK_MONOTONIC, &currentTime);

	int64_t ticks =
		(int64_t)currentTime.tv_sec * 1000000000LL +
		(int64_t)currentTime.tv_nsec;

	return (double)ticks * 1e-9;
}