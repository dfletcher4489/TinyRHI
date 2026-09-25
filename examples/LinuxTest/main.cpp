#include "OSFile.h"
#include "OSThread.h"
#include "OSWindow.h"
#include "OSMemory.h"
#include "StringUtils.h"
#include <stdio.h>
#include <string.h>

#include "allocator/AppAllocator.h"

#if defined(_WIN32)
#include "WinOSFile.h"
#endif

static volatile bool done = false;
static char MainWindowEventBuffer[512];
static char OSMemoryBuffer[4096*2];

SlabAllocator osMemoryAllocator { OSMemoryBuffer, sizeof(OSMemoryBuffer), STRING_VIEW_FROM_LITERAL("Global OS Memory Storage Buffer"), nullptr };

void ScanSTDIN(void* argument)
{
    uint64_t readSize;

    char inputBuffer[512];

    OSFileHandle stdInHandle;

    OSGetSTDInput(&stdInHandle);

    int pollCommand = OS_FILE_POLL_TIMEOUT;

    while(!done)
    {
        pollCommand = OSPollFile(&stdInHandle, 500);

        if (!pollCommand)
        {
            int inputCharMakeUp = 1;
#ifdef WIN32
            int success = OSPollWindowsCommandLine();

            if (success <= 0) continue;

            inputCharMakeUp = 2;
#endif
            readSize = OSReadFile(&stdInHandle, sizeof(inputBuffer), inputBuffer);

            if (readSize <= inputCharMakeUp)
                continue;

            inputBuffer[readSize - inputCharMakeUp] = '\0';

            if (!strcmp(inputBuffer, "end"))
            {
                break;
            }
            else
            {
                printf("%s %d\n", inputBuffer, readSize);
            }
        }
    }

    done = true;
}

int TestOSFiles()
{
    StringView testFile = STRING_VIEW_FROM_LITERAL("thisistestfile.txt");

    StringView testDirectory = STRING_VIEW_FROM_LITERAL("testdirectory");

    OSFileHandle handle{}; 

    int retCode = OSCreateDirectory(testDirectory.stringData, testDirectory.charCount, PUBLIC_DIR);

    if (retCode)
    {
        return -1;
    }

    int currDirectoryLength = OSGetCurrentDirectorySize();

    char currDirectoryBuffer[4096];

    retCode = OSGetCurrentDirectory(sizeof(currDirectoryBuffer), currDirectoryBuffer);

    if (retCode)
    {
        return -1;
    }

    currDirectoryBuffer[(currDirectoryLength)] = OSGetSystemFileTerminator();

    memcpy(currDirectoryBuffer + (currDirectoryLength)+1, testDirectory.stringData, testDirectory.charCount+1);

    retCode = OSSetCurrentDirectory(currDirectoryBuffer, (currDirectoryLength+1)+(testDirectory.charCount+1));

    if (retCode)
    {
        return -1;
    }

    retCode = OSCreateFile(testFile.stringData, testFile.charCount, READ | WRITE | CREATE_IF_NOT_EXIST, &handle);

    if (retCode)
    {
        return -1;
    }

    const char* outputString = "HELLO";

    size_t size = strlen(outputString);

    int64_t dataRet = OSWriteFile(&handle, size, outputString);

    if (dataRet != size)
    {
        return -1;
    }

    retCode = OSSeekFile(&handle, 0, BEGIN);

    if (retCode)
    {
        OSCloseFile(&handle);
        return -1;
    }

    char dataOut[64];

    dataRet = OSReadFile(&handle, size, dataOut);

    if (dataRet != size || strcmp(outputString, dataOut) != 0)
    {
        OSCloseFile(&handle);
        return -1;
    }

    if (OSFileExist(testFile.stringData, testFile.charCount, READ | WRITE))
    {
        OSCloseFile(&handle);
        return -1;
    }

    OSCloseFile(&handle);

    return 0;
}

int TestOSSync()
{
    OSSemaphore sema;

    int retCode = CreateOSSemaphore(&sema, 2);

    if (retCode)
    {
        return -1;
    }
    
    retCode = WaitOSSemaphore(&sema, 500);

    if (retCode)
    {
        DeleteOSSemaphore(&sema);
        return -1;
    }

    retCode = WaitOSSemaphore(&sema, 500);

    if (retCode)
    {
        DeleteOSSemaphore(&sema);
        return -1;
    }

    retCode = WaitOSSemaphore(&sema, 3000);

    if (retCode != OS_SEMAPHORE_TIMEOUT_FAILED)
    {
        DeleteOSSemaphore(&sema);
        return -1;
    }

    printf("waited\n");

    retCode = NotifyOSSemaphore(&sema);

    retCode |= NotifyOSSemaphore(&sema);

    retCode |= DeleteOSSemaphore(&sema);

    OSSharedExclusive sharedExclusive{};

    retCode = CreateOSSharedExclusive(&sharedExclusive);

    if (retCode)
    {
        return -1;
    }

    retCode = ExclusiveAcquireOSSharedExclusive(&sharedExclusive);

    if (retCode)
    {
        DeleteOSSharedExclusive(&sharedExclusive);
        return -1;
    }

    retCode = SharedAcquireOSSharedExclusive(&sharedExclusive);

    if (retCode != OSSE_SHARED_LOCK_ACQUIRE_FAILED)
    {
        DeleteOSSharedExclusive(&sharedExclusive);
        return -1;
    }

    retCode = ExclusiveReleaseOSSharedExclusive(&sharedExclusive);

    if (retCode)
    {
        DeleteOSSharedExclusive(&sharedExclusive);
        return -1;
    }

    retCode = TryExclusiveAcquireOSSharedExclusive(&sharedExclusive);

    if (retCode)
    {
        DeleteOSSharedExclusive(&sharedExclusive);
        return -1;
    }

    retCode = TrySharedAcquireOSSharedExclusive(&sharedExclusive);

    if (retCode != OSSE_SHARED_LOCK_ACQUIRE_FAILED)
    {
        DeleteOSSharedExclusive(&sharedExclusive);
        return -1;
    }

    retCode = ExclusiveReleaseOSSharedExclusive(&sharedExclusive);

    retCode = TrySharedAcquireOSSharedExclusive(&sharedExclusive);

    if (retCode)
    {
        DeleteOSSharedExclusive(&sharedExclusive);
        return -1;
    }

    retCode = TrySharedAcquireOSSharedExclusive(&sharedExclusive);

    if (retCode)
    {
        DeleteOSSharedExclusive(&sharedExclusive);
        return -1;
    }

    SharedReleaseOSSharedExclusive(&sharedExclusive);

    DeleteOSSharedExclusive(&sharedExclusive);

    return retCode;
}

int main(int argc, const char** argv)
{
    StringView mainInputFile = STRING_VIEW_FROM_LITERAL("test.txt");

    StringView mainOutputFile = STRING_VIEW_FROM_LITERAL("test3.txt");

    OSFileHandle inputFileHandle, outputFileHandle, stdOutHandle, stdInHandle;

    int retCode = 0;

    OSFileMemoryRequirements fileMemReq = OSGetFileMemoryRequirements(5);

    OSThreadMemoryRequirements threadMemRequirements = OSGetThreadMemoryRequirements(5);

    OSWindowMemoryRequirements winMemReq = OSGetWindowMemoryRequirements(1);

    OSMemoryRequirements osMemRequiresMents = OSGetMemoryRequirements(1);

    OSSyncMemoryRequirements osSyncMemRequirements = OSGetSyncMemoryRequirements(5);

    OSSeedMemory(
        osMemoryAllocator.Allocate(osMemRequiresMents.dataSize, osMemRequiresMents.alignment),
        osMemRequiresMents.dataSize,
        1
    );

    OSSeedFileMemory(
		osMemoryAllocator.Allocate(fileMemReq.dataSize, fileMemReq.alignment),
		fileMemReq.dataSize,
		5
	);

    OSSeedThreadMemory(
        osMemoryAllocator.Allocate(threadMemRequirements.dataSize, threadMemRequirements.alignment), 
        threadMemRequirements.dataSize, 
        5
    );

    OSSeedWindowMemory(
        osMemoryAllocator.Allocate(winMemReq.dataSize, winMemReq.alignment),
        winMemReq.dataSize,
        1
    );

    OSSeedSyncMemory(
        osMemoryAllocator.Allocate(osSyncMemRequirements.dataSize, osSyncMemRequirements.alignment),
        osSyncMemRequirements.dataSize,
        5
    );

    OSThreadHandle handle{};

    OSWindow window{};

    GenericWindowInfo info{};

    int closeWindow = 0;

    int testSync = 0;

    int testFiles = TestOSFiles();

    if (testFiles)
    {
        goto end;
    }

    testSync = TestOSSync();

    if (testSync)
    {
        goto end;
    }

    OSCreateThread(&handle, nullptr, ScanSTDIN, OS_THREAD_NONE);

    OSCreateWindow("Multi Platform Test", 320, 240, &window);

    OSWindowSeedEventBuffer(&window, MainWindowEventBuffer, sizeof(MainWindowEventBuffer));

    OSWindowShow(&window);

    closeWindow = 1;

    while(!done && !info.shouldBeClosed)
    {
        int ret = OSWindowPollEvents(&window, &info);

        if (ret)
        {
            done = true;
        }
    }

end:

    CloseAllThreads();

    if (closeWindow)
    {
        CloseAllWindows();
    }

    return retCode;
}

/*
    int retCode = OSOpenFile(mainInputFile.stringData, mainInputFile.charCount, READ, &inputFileHandle);

    if (retCode < 0)
    {
        StringView errorText = STRING_VIEW_FROM_LITERAL("Could not open input file");
        OSWriteFile(&stdOutHandle, errorText.charCount, errorText.stringData);
        return -1;
    }

    char* inputBuffer = (char*)malloc(inputFileHandle.fileLength);

    int readCount = OSReadFile(&inputFileHandle, inputFileHandle.fileLength, inputBuffer);

*/
/*
        int retCode = OSCreateFile(mainOutputFile.stringData, mainOutputFile.charCount, WRITE|CREATE_IF_NOT_EXIST, &outputFileHandle);

        uint64_t writeOutSize = 0;

        if (retCode < 0)
        {
            StringView errorText = STRING_VIEW_FROM_LITERAL("Could not open output file\n");
            OSWriteFile(&stdOutHandle, errorText.charCount, errorText.stringData, &writeOutSize);
            return -1;
        }

        char inputBuffer[1024];

        int pollCommand = OS_FILE_POLL_TIMEOUT;

        while(pollCommand == OS_FILE_POLL_TIMEOUT)
        {
            pollCommand = OSPollFile(&stdInHandle, 500);
        }

        retCode = 0;

        if (pollCommand > 0)
        {
            uint64_t readCount = 0;

            OSReadFile(&stdInHandle, 1024, inputBuffer, &readCount);

            OSWriteFile(&outputFileHandle, readCount, inputBuffer, &writeOutSize);
        }
        else
        {
            StringView errorText = STRING_VIEW_FROM_LITERAL("Could not poll std in file\n");

            OSWriteFile(&stdOutHandle, errorText.charCount, errorText.stringData, &writeOutSize);

            retCode = -1;
        }

        OSCloseFile(&outputFileHandle);

*/