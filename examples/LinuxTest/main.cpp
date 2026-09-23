#include "OSFile.h"
#include "OSThread.h"
#include "OSWindow.h"
#include "OSMemory.h"
#include "StringUtils.h"
#include <stdio.h>
#include <string.h>

#if defined(_WIN32)
#include "WinOSFile.h"
#endif

static volatile bool done = false;
static char MainWindowEventBuffer[512];
static char OSMemoryBuffer[512];

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

int main(int argc, const char** argv)
{
    StringView mainInputFile = STRING_VIEW_FROM_LITERAL("test.txt");

    StringView mainOutputFile = STRING_VIEW_FROM_LITERAL("test3.txt");

    OSFileHandle inputFileHandle, outputFileHandle, stdOutHandle, stdInHandle;

    int retCode = 0;

    OSThreadMemoryRequirements threadMemRequirements = OSGetThreadMemoryRequirements(5);

    OSWindowMemoryRequirements winMemReq = OSGetWindowMemoryRequirements(1);

    OSMemoryRequirements osMemRequiresMents = OSGetMemoryRequirements(1);

    OSSeedMemory(
        OSMemoryBuffer,
        osMemRequiresMents.dataSize,
        1
    );

    void* data = OSMemoryAllocate(nullptr, threadMemRequirements.dataSize + winMemReq.dataSize, OSMemoryAllocationTypes::COMMIT | OSMemoryAllocationTypes::RESERVE, OSMemoryAllocationProtections::READWRITE);

    OSSeedThreadMemory(
        data, 
        threadMemRequirements.dataSize, 
        5
    );

    OSSeedWindowMemory(
        (void*)((uintptr_t)data + threadMemRequirements.dataSize),
        winMemReq.dataSize,
        1
    );

    OSThreadHandle handle{};

    OSWindow window{};

    OSCreateThread(&handle, nullptr, ScanSTDIN, OS_THREAD_NONE);

    OSCreateWindow("Multi Platform Test", 320, 240, &window);

    OSWindowSeedEventBuffer(&window, MainWindowEventBuffer, sizeof(MainWindowEventBuffer));

    OSWindowShow(&window);

    GenericWindowInfo info{};

    while(!done && !info.shouldBeClosed)
    {
        int ret = OSWindowPollEvents(&window, &info);

        if (ret)
        {
            done = true;
        }
    }

    CloseAllThreads();

    CloseAllWindows();

    OSMemoryRelease(data, 0, OSMemoryReleaseTypes::RELEASE);

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