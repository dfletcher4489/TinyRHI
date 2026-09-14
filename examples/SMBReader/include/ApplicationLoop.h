#pragma once

#include "ProgramArgs.h"

struct ApplicationLoop
{
	ApplicationLoop(ProgramArgs& _args);
	~ApplicationLoop();

	void Execute();

	int InitializeRuntime();

	void CleanupRuntime();
	
	ProgramArgs& args;
	
	bool running, cleaned;
};
