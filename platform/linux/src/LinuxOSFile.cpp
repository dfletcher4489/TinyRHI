#include "OSFile.h"

#include <string.h>
#include <stdlib.h>

#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define MAX_PATH 4096

#define FILE_DESCRIPTOR_STD_IN 0
#define FILE_DESCRIPTOR_STD_OUT 1
#define FILE_DESCRIPTOR_STD_ERR 2

static int* intFileHandles;
static MPMCQueueData* freeList;
static int maxFreeListEntry = 0;

ALIGNAS(128) static std::atomic<int> boundedLinearAllocator{ 0 };
ALIGNAS(128) static std::atomic<size_t> enqueuePos{ 0 };
ALIGNAS(128) static std::atomic<size_t> dequeuePos{ 0 };

static int ConvertOSFileFlags(OSFileFlags flags)
{
    int osFlags = 0;

    if ((flags & READ) && (flags & WRITE))
    {   
        osFlags |= O_RDWR;
    } 
    else
    {
        if (flags & READ)
            osFlags |= O_RDONLY;
        if (flags & WRITE)
            osFlags |= O_WRONLY;
    }
    
    if (flags & CREATE)
    {
        osFlags |= (O_CREAT | O_TRUNC);
    }
    else if (flags & CREATE_IF_NOT_EXIST)
    {
        osFlags |= O_CREAT;
    }

    return osFlags;
}

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

    intFileHandles[index] = -1;
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

OSFileMemoryRequirements OSGetFileMemoryRequirements(int maxNumberOfOpenFiles)
{
    int handlesSize = (maxNumberOfOpenFiles) * sizeof(int);
    int freeListSize = (maxNumberOfOpenFiles) * sizeof(MPMCQueueData);

    OSFileMemoryRequirements memReqs{ handlesSize + freeListSize, alignof(MPMCQueueData) };

    return memReqs;
}

void CloseAllFiles()
{
    for (int i = 0; i < maxFreeListEntry; i++)
    {
        int idx = i;

        if (intFileHandles[idx] != -1)
        {
            close(intFileHandles[idx]);
            intFileHandles[idx] = -1;  
        }

        freeList[i].currentSequence.store(i, std::memory_order_relaxed);
    }

    enqueuePos.store(0, std::memory_order_relaxed);
    dequeuePos.store(0, std::memory_order_relaxed);
    boundedLinearAllocator.store(0, std::memory_order_relaxed);
}

int OSSeedFileMemory(void* dataSource, int dataSize, int numberOfOpenFiles)
{
    uintptr_t dataHead = (uintptr_t)dataSource;
    uintptr_t dataStart = dataHead;

    int handleSize = numberOfOpenFiles;

    freeList = (MPMCQueueData*)dataHead;

    dataHead += handleSize * sizeof(MPMCQueueData);

    intFileHandles = (int*)dataHead;

    for (int i = 0; i < handleSize; i++)
    {
        intFileHandles[i] = -1;
        freeList[i].currentSequence.store(i, std::memory_order_relaxed);
    }

    maxFreeListEntry = handleSize;

    return OS_FILE_SUCCESS;
}

int OSCreateFile(const char* filename, int nameLength, OSFileFlags flags, OSFileHandle* fileHandle)
{
    char namescratch[MAX_PATH];

    if (nameLength <= 0 || nameLength >= MAX_PATH || !filename || !fileHandle)
    {
        return OS_FILE_INVALID_ARGUMENT;
    }

    int internalHandlePtr = FindFreeIndex();

    if (internalHandlePtr < 0)
    {
        return OS_FILE_HANDLE_EXHASUTED;
    }

    memcpy(namescratch, filename, nameLength);

    namescratch[nameLength] = '\0';

    int fcntlFlags = ConvertOSFileFlags(flags);

    int fd = open(namescratch, fcntlFlags, S_IRUSR | S_IWUSR | S_IXUSR /*| S_IRGRP | S_IWGRP | S_IXGRP*/);

    if (fd < 0)
    {
        ReturnIndex(internalHandlePtr);
        return OS_FILE_FAILED_CREATE;
    }

    intFileHandles[internalHandlePtr] = fd;

    fileHandle->fileLength = 0;
    fileHandle->filePointer = 0;
    fileHandle->osDataHandle = internalHandlePtr;

    return OS_FILE_SUCCESS;
}

int OSOpenFile(const char* filename, int nameLength, OSFileFlags flags, OSFileHandle* fileHandle)
{
    char namescratch[MAX_PATH];

    if (nameLength <= 0 || nameLength >= MAX_PATH || !filename || !fileHandle)
    {
        return OS_FILE_INVALID_ARGUMENT;
    }

    int internalHandlePtr = FindFreeIndex();

    if (internalHandlePtr < 0)
    {
        return OS_FILE_HANDLE_EXHASUTED;
    }

    memcpy(namescratch, filename, nameLength);

    namescratch[nameLength] = '\0';

    int fcntlFlags = ConvertOSFileFlags(flags);

    int fd = open(namescratch, fcntlFlags);

    if (fd < 0)
    {
        ReturnIndex(internalHandlePtr);
        return OS_FILE_FAILED_CREATE;
    }

    struct stat statBuffer{};

    int fstatRet = fstat(fd, &statBuffer);

    if (fstatRet < 0)
    {
        close(fd);
        ReturnIndex(internalHandlePtr);
        return OS_FILE_FAILED_SIZE;
    }

    intFileHandles[internalHandlePtr] = fd;

    fileHandle->fileLength = statBuffer.st_size;
    fileHandle->filePointer = 0;
    fileHandle->osDataHandle = internalHandlePtr;

    return OS_FILE_SUCCESS;
}

int OSCloseFile(OSFileHandle* fileHandle)
{
    if (!fileHandle)
    {
        return OS_FILE_INVALID_ARGUMENT;
    }

    if (fileHandle->osDataHandle >= maxFreeListEntry || fileHandle->osDataHandle < 0)
    {
        return OS_FILE_INVALID_ARGUMENT;
    }

    int fd = intFileHandles[fileHandle->osDataHandle];

    int retCode = close(fd);

    if (retCode < 0)
    {
        retCode = OS_FILE_CLOSED_FAILED;
    }

    ReturnIndex(fileHandle->osDataHandle);

    fileHandle->osDataHandle = -1;
    fileHandle->fileLength = 0;
    fileHandle->filePointer = 0;

    return retCode;
}

int64_t OSReadFile(OSFileHandle* fileHandle, int size, char* buffer)
{
     if (!fileHandle || !buffer)
    {
        return OS_FILE_INVALID_ARGUMENT;
    }

    if (fileHandle->osDataHandle >= maxFreeListEntry+1 || fileHandle->osDataHandle < 0)
    {
        return OS_FILE_INVALID_ARGUMENT;
    }

    int fd = -1;
    
    if (fileHandle->osDataHandle < maxFreeListEntry) 
    {
        fd = intFileHandles[fileHandle->osDataHandle];
    }
    else
    {
        fd = FILE_DESCRIPTOR_STD_IN;
    }

    ssize_t readCount = read(fd, buffer, size);

    if (readCount < 0)
    {
        return OS_FILE_FAILED_READ;
    }

    fileHandle->filePointer += readCount;

    return readCount;
}

int64_t OSWriteFile(OSFileHandle* fileHandle, int size, const char* buffer)
{
    if (!fileHandle || !buffer)
    {
        return OS_FILE_INVALID_ARGUMENT;
    }

    if (fileHandle->osDataHandle == maxFreeListEntry || fileHandle->osDataHandle < 0)
    {
        return OS_FILE_INVALID_ARGUMENT;
    }

    int fd = -1;
    
    if (fileHandle->osDataHandle < maxFreeListEntry) 
    {
        fd = intFileHandles[fileHandle->osDataHandle];
    }
    else if (fileHandle->osDataHandle == maxFreeListEntry+1)
    {
        fd = FILE_DESCRIPTOR_STD_OUT;
    }
    else
    {
        fd = FILE_DESCRIPTOR_STD_ERR;
    }

    ssize_t writeCount = write(fd, buffer, size);

    if (writeCount < 0)
    {
        return OS_FILE_FAILED_WRITE;
    }

    fileHandle->filePointer += writeCount;

    return writeCount;
}

int OSSeekFile(OSFileHandle* fileHandle, size_t pointer, OSRelativeFlags flags)
{
    if (!fileHandle)
    {
        return OS_FILE_INVALID_ARGUMENT;
    }

    if (fileHandle->osDataHandle >= maxFreeListEntry || fileHandle->osDataHandle < 0)
    {
        return OS_FILE_INVALID_ARGUMENT;
    }

    int whence = SEEK_SET;

    switch (flags)
    {
    case BEGIN:
        break;
    case CURRENT:
        whence = SEEK_CUR;
        break;
    case END:
        whence = SEEK_END;
        break;
    default:
        return OS_FILE_INVALID_ARGUMENT;
    }

    int fd = intFileHandles[fileHandle->osDataHandle];

    off64_t newOffset = lseek64(fd, pointer, whence);

    if (newOffset < 0)
    {
        return OS_FILE_FAILED_SEEK;
    }

    fileHandle->filePointer = newOffset;

    return OS_FILE_SUCCESS;
}

int OSCreateFileIterator(const char* searchString, int nameLength, OSFileIterator* iterator)
{
    return OS_FILE_FUNCTION_NOT_IMPLEMENTED;
}

int OSNextFile(OSFileIterator* iterator)
{
    return OS_FILE_FUNCTION_NOT_IMPLEMENTED;
}

void OSGetSTDInput(OSFileHandle* fileHandle)
{
    fileHandle->osDataHandle = maxFreeListEntry;
    fileHandle->fileLength = 0;
    fileHandle->filePointer = 0;
}

void OSGetSTDOutput(OSFileHandle* fileHandle)
{
    fileHandle->osDataHandle = maxFreeListEntry+1;
    fileHandle->fileLength = 0;
    fileHandle->filePointer = 0;
}

void OSGetSTDError(OSFileHandle* fileHandle)
{
    fileHandle->osDataHandle = maxFreeListEntry+2;
    fileHandle->fileLength = 0;
    fileHandle->filePointer = 0;
}

int OSPollFile(OSFileHandle* fileHandle, int millisecondTimeOut)
{
    if (fileHandle->osDataHandle < 0 || fileHandle->osDataHandle >= maxFreeListEntry+1)
    {
        return OS_FILE_FAILED_POLL;
    }

    struct pollfd fds{};

    fds.fd = fileHandle->osDataHandle;
    fds.events = POLLIN;
    fds.revents = 0;

    int pollRet = poll(&fds, 1, millisecondTimeOut);

    if (pollRet < 0)
    {
        return OS_FILE_FAILED_POLL;
    }

    if (!pollRet)
    {
        return OS_FILE_POLL_TIMEOUT;
    }

    return OS_FILE_SUCCESS;
}

int OSFileExist(const char* inputFile, int charCount, OSFileFlags flags)
{
    char namescratch[MAX_PATH];

    if (charCount <= 0 || charCount >= MAX_PATH || !inputFile)
    {
        return OS_FILE_INVALID_ARGUMENT;
    }

    memcpy(namescratch, inputFile, charCount);

    namescratch[charCount] = '\0';

    int osFlags = 0;

    if ((flags & READ) && (flags & WRITE))
    {   
        osFlags |= O_RDWR;
    } 
    else
    {
        if (flags & READ)
            osFlags |= O_RDONLY;
        if (flags & WRITE)
            osFlags |= O_WRONLY;
    }

    int fd = open(namescratch, osFlags);

    if (fd < 0)
    {
        return OS_FILE_FAILED_EXISTING;
    }

    close(fd);

    return OS_FILE_SUCCESS;
}

int OSExtractFileName(const char* inputFilePath, int inputFilePathCount, char* outputBuffer)
{
    if (inputFilePathCount <= 0 || inputFilePathCount >= MAX_PATH || !outputBuffer)
    {
        return OS_FILE_INVALID_ARGUMENT;
    }

    char filePathTerminator1 = '\\';
    char filePathTerminator2 = '/';

    int searchForFileTerminatorIter = inputFilePathCount-1;

    while (searchForFileTerminatorIter >= 0)
    {
        if (inputFilePath[searchForFileTerminatorIter] == filePathTerminator2 || inputFilePath[searchForFileTerminatorIter] == filePathTerminator1)
            break;
     
        searchForFileTerminatorIter--;
    }

    int searchForFileExtensionBeginIter = inputFilePathCount;

    while (searchForFileExtensionBeginIter >= 0)
    {
        if (inputFilePath[searchForFileExtensionBeginIter] == '.')
            break;

        searchForFileExtensionBeginIter--;
    }

    if (searchForFileExtensionBeginIter < 0)
        return -1;

    int fileNameSize = (searchForFileExtensionBeginIter - searchForFileTerminatorIter) - 1;

    memcpy(outputBuffer, inputFilePath + searchForFileTerminatorIter + 1, fileNameSize);
    
    return fileNameSize;
}

int OSGetSystemFileTerminator()
{
    return '/';
}

int OSCreateDirectory(const char* directoryPath, int charCount, OSDirectoryFlag directoryFlag)
{
    if (charCount <= 0 || charCount >= MAX_PATH)
    {
        return OS_FILE_INVALID_ARGUMENT;
    }

    char pathscratch[MAX_PATH];

    memcpy(pathscratch, directoryPath, charCount);

    pathscratch[charCount] = '\0';

    int accessFlags = S_IRWXU;

    if (directoryFlag & PUBLIC_DIR)
    {
        accessFlags |= (S_IRWXG | S_IRWXO);
    }

    int retCode = mkdir(pathscratch, accessFlags);

    if (retCode < 0)
    {
        if (errno != EEXIST)
        {
            return OS_FILE_FAILED_CREATE_DIRECTORY;
        }
    }

    return OS_FILE_SUCCESS;
}

int OSGetCurrentDirectorySize()
{
    char *cwd = getcwd(NULL, 0);

    if (cwd == NULL)
    {
        return OS_FILE_FAILED_GET_CURRENT_DIRECTORY;;
    }

    int size = strlen(cwd);

    free(cwd);

    return size;
}

int OSGetCurrentDirectory(int bufferSize, char* outputBuffer)
{
    char* cwd = getcwd(outputBuffer, bufferSize);

    if (cwd == NULL)
    {
        return OS_FILE_FAILED_GET_CURRENT_DIRECTORY;
    }

    return OS_FILE_SUCCESS;
}

int OSSetCurrentDirectory(const char* inputPath, int charCount)
{
    if (charCount <= 0 || charCount >= MAX_PATH || !inputPath)
    {
        return OS_FILE_INVALID_ARGUMENT;
    }

    char pathscratch[MAX_PATH];

    memcpy(pathscratch, inputPath, charCount);

    pathscratch[charCount] = '\0';

    int retCode = chdir(pathscratch);

    if (retCode < 0)
    {
        return OS_FILE_FAILED_SET_CURRENT_DIRECTORY;
    }

    return OS_FILE_SUCCESS;
}