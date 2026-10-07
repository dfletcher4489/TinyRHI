#include "OSFile.h"
#include "OSThread.h"
#include "OSWindow.h"
#include "OSMemory.h"
#include "StringUtils.h"
#include <stdio.h>
#include <string.h>

#include "allocator/AppAllocator.h"
#include "Camera.h"
#include "RenderInstance.h"
#include "WindowManager.h"

#if defined(_WIN32)
#include "WinOSFile.h"
#endif

static volatile bool done = false;
static char MainWindowEventBuffer[512];
static char OSMemoryBuffer[4096*2];

static SlabAllocator osMemoryAllocator { OSMemoryBuffer, sizeof(OSMemoryBuffer), STRING_VIEW_FROM_LITERAL("Global OS Memory Storage Buffer"), nullptr };

static OSWindow window{};
static Logger mainAppLogger{};
static RenderPhysicalDeviceIndex mainGPU{};
static constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 1;
static RenderDeviceIndex mainLogicalDevice{};
static SwapChainIndex mainPresentationSwapChain{};
static WindowIndex mainPresentationWindow{};
static BufferMemoryIndex mainHostBuffer{};
static BufferMemoryIndex mainDeviceBuffer{};

static size_t mainHostSize = 512;
static size_t mainDeviceSize = 1 * KiB;

static DeviceSlabAllocator mainHostAllocator(mainHostSize, STRING_VIEW_FROM_LITERAL("Main GPU Host Driver Buffer"), &mainAppLogger);
static DeviceSlabAllocator mainDeviceAllocator(mainDeviceSize, STRING_VIEW_FROM_LITERAL("Main GPU Device Driver Buffer"), &mainAppLogger);

static ShaderResourceManagerIndex mainDescriptorManagerIndex{};
static GPUCommandStreamIndex mainCommandStreamIndex{};
static AttachmentGraphInstanceIndex basicGraphInstance{};
static GeneratedPipelineInstanceIndex pipelineHandle{};
static PipelineHandleIndex basicPipeline{};
static Camera mainCamera{};

static char RenderInstanceMemoryPool[64 * MiB];
static char RenderInstanceTemporaryPool[64 * KiB];
static char LoggerMessageMemory[64 * KiB];

static TLSFAllocator RenderInstanceMemoryAllocator{ RenderInstanceMemoryPool, sizeof(RenderInstanceMemoryPool), STRING_VIEW_FROM_LITERAL("Global Render Instance Storage Buffer"), &mainAppLogger };
static RingAllocator RenderInstanceTemporaryAllocator{ RenderInstanceTemporaryPool, sizeof(RenderInstanceTemporaryPool), STRING_VIEW_FROM_LITERAL("Global Render Instance Frame Buffer"), &mainAppLogger };

static int graphicsInit = 0;

static int windowWidth = 800;
static int windowHeight = 600;

static size_t mainDSVSize = 10 * MiB;
static ImageMemoryIndex mainDSVIndex{};
static DeviceSlabAllocator mainDSVAllocator(mainDSVSize, STRING_VIEW_FROM_LITERAL("Main DSV Allocator"), &mainAppLogger);

int InitGraphicsRuntime()
{
    mainAppLogger.InitLogger(LoggerMessageMemory, sizeof(LoggerMessageMemory));

    RenderInstanceCreateInfo riCreateInfo{};
    riCreateInfo.maxAttachmentGraphTemplates = 1;
    riCreateInfo.maxAttachmentGraphInstances = 1;
    riCreateInfo.maxImagePoolsCount = 1;
    riCreateInfo.maxBufferPoolsCount = 2;
    riCreateInfo.maxRenderTargets = 1;
    riCreateInfo.maxShaderGraphs = 1;
    riCreateInfo.maxShaderHandles = 2;
    riCreateInfo.maxDescriptorManagers = 1;
    riCreateInfo.maxShaderResourceTemplates = 1;
    riCreateInfo.maxComputeQueues = 1;
    riCreateInfo.maxRenderQueues = 1;
    riCreateInfo.maxPipelineTemplates = 5;
    riCreateInfo.maxPipelineInstances = 5;
    riCreateInfo.maxPipelineHandles = 5;
    riCreateInfo.maxAllocations = 10;
    riCreateInfo.maxGPUCommandsStreams = 1;
    riCreateInfo.maxTextureHandles = 15;
    riCreateInfo.maxSamplerHandles = 1;
    riCreateInfo.maxResourceStatuses = 15;
    riCreateInfo.commandBuffersSize = 32 * KiB;
    riCreateInfo.commandsCacheSize = 64 * KiB;
    riCreateInfo.internalLoggerRingSize = 8 * KiB;
    riCreateInfo.numberOfDriverHostAllocations = 10;
    riCreateInfo.numberOfTransferCommandAllocations = 10;
    riCreateInfo.numberOfResourceUpdateAllocations = 10;
    riCreateInfo.numberOfDriverDeviceAllocations = 10;
    riCreateInfo.numberOfImageMemoryAllocations = 10;
    riCreateInfo.maxWindows = 1;
    riCreateInfo.maxSwapChains = 1;
    riCreateInfo.maxGPUS = 1;
    riCreateInfo.maxLogicalDevices = 1;

    OSGetSTDOutput(&riCreateInfo.internalRendererHandle);

    GPUFeatureRequest request{};

    request.desiredMaxImageWidth = 4096;
    request.desiredMaxImageHeight = 4096;
    request.deviceType = DISCRETE | INTEGRATED;
    request.requireDescriptorBindingPartiallyBound = false;
    request.requireDescriptorBindingSampledImageUpdateAfterBind = false;
    request.requireDescriptorBindingUpdateUnusedWhilePending = false;
    request.requireDescriptorBindingVariableDescriptorCount = false;
    request.requireShaderSampledImageArrayNonUniformIndexing = false;
    request.requireStorageBuffer8BitAccess = false;
    request.requireDrawIndirectCount = false;
    request.requireRuntimeDescriptorArray = false;
    request.requireGeometryShader = false;
    request.requireTextureCompressionBC = false;
    request.requireTessellationShader = false;
    request.requireSamplerAnisotropy = false;
    request.requireMultiDrawIndirect = false;
    request.requireWideLines = false;
    request.requireTimelineSemaphores = false;

    LogicalDeviceFeatures deviceFeatures{};

    deviceFeatures.useSPVDebugInfo = false;
    deviceFeatures.useSPVDrawParameters = false;
    deviceFeatures.useSwapChain = true;
    deviceFeatures.useSwapChainMaintenance = false;

    GlobalRenderer::gRenderInstance.CreateRenderInstance(&riCreateInfo, &RenderInstanceMemoryAllocator, &RenderInstanceTemporaryAllocator);

    int rendererRetCode = GlobalRenderer::gRenderInstance.CreateHighLevelInstance(3 * MiB, 4 * MiB, 4 * KiB, 96 * KiB, false);

    if (rendererRetCode)
    {
        return -1;
    }

    OSWindowInternalData internalWindowData;

    OSWindowGetInternalData(&window, &internalWindowData);

    mainPresentationWindow = GlobalRenderer::gRenderInstance.CreateWindowedSurface(&internalWindowData);

    if (mainPresentationWindow == WindowIndex())
    {
        return -1;
    }

    int foundPhysicalDevice = GlobalRenderer::gRenderInstance.OpenPhysicalDevicePicker();

    if (foundPhysicalDevice)
    {
        return -1;
    }

    mainGPU = GlobalRenderer::gRenderInstance.CreatePhysicalDeviceAdapterWithQuerying(&request, &deviceFeatures);

    if (RenderPhysicalDeviceIndex() == mainGPU)
    {
        return -1;
    }

    GlobalRenderer::gRenderInstance.ClosePhysicalDevicePicker();

    LogicalDeviceCreateInfo lDeviceCreateInfo{};

    lDeviceCreateInfo.maxQueries = 0;
    lDeviceCreateInfo.physicalDeviceIndex = mainGPU;
    lDeviceCreateInfo.surfaceIndexForPresent = mainPresentationWindow;
    lDeviceCreateInfo.requestedDeviceFeatures = &deviceFeatures;
    lDeviceCreateInfo.requestedPhysicalFeatures = &request;
    lDeviceCreateInfo.deviceInstCacheSize = 96 * KiB;
    lDeviceCreateInfo.deviceInstHandleSize = 16 * KiB;
    lDeviceCreateInfo.deviceInstPermanentSize = 32 * KiB;
    lDeviceCreateInfo.driverCacheSize = 5 * MiB;
    lDeviceCreateInfo.driverPermanentSize = 5 * MiB;
    lDeviceCreateInfo.maxFramesInFlight = MAX_FRAMES_IN_FLIGHT;
    lDeviceCreateInfo.maxConcurrentRecordings = 1;
    lDeviceCreateInfo.maxAllocations = 10;
    lDeviceCreateInfo.maxTextureHandles = 15;

    mainLogicalDevice = GlobalRenderer::gRenderInstance.CreateLogicalDevice(&lDeviceCreateInfo);

    if (RenderDeviceIndex() == mainLogicalDevice)
    {
        return -1;
    }

    rendererRetCode = GlobalRenderer::gRenderInstance.CreatePerFrameStagingBuffers(mainLogicalDevice, 512 * KiB);

	if (rendererRetCode)
	{
		return -1;
	}

    ImageFormat requestedColorFormats = ImageFormat::R8G8B8A8;

    ImageFormat mainColorFormat = GlobalRenderer::gRenderInstance.FindSupportedBackBufferColorFormat(mainGPU, mainPresentationWindow, &requestedColorFormats, 1);

    if (ImageFormat::IMAGE_UNKNOWN == mainColorFormat)
    {
        return -1;
    }

    ImageFormat requestedDSVFormats = ImageFormat::D32FLOAT;

    ImageFormat mainDepthFormat = GlobalRenderer::gRenderInstance.FindSupportedDepthFormat(mainLogicalDevice, &requestedDSVFormats, 1);

    if (ImageFormat::IMAGE_UNKNOWN == mainDepthFormat)
    {
        return -1;
    }

    mainDSVIndex = GlobalRenderer::gRenderInstance.CreateImagePool(
		mainLogicalDevice, mainDSVSize, mainDepthFormat, 4096, 4096,
		ImageUsageFlagBits::TRANSFER_SRC | ImageUsageFlagBits::TRANSFER_DEST |
		ImageUsageFlagBits::SAMPLED | ImageUsageFlagBits::DEPTH_ATTACHMENT | 
		ImageUsageFlagBits::STENCIL_ATTACHMENT,
		MemoryTypeBits::DEVICE_MEMORY_TYPE);

	if (ImageMemoryIndex() == mainDSVIndex)
	{
		return -1;
	}

    mainPresentationSwapChain = GlobalRenderer::gRenderInstance.CreateSwapChainHandle(mainLogicalDevice, mainPresentationWindow, mainColorFormat, windowWidth, windowHeight);

    if (SwapChainIndex() == mainPresentationSwapChain)
    {
        return -1;
    }

    AttachmentGraphLayoutIndex basicLayout = GlobalRenderer::gRenderInstance.CreateAttachmentGraph(STRING_VIEW_FROM_LITERAL("BasicLayout.adf"));

    if (AttachmentGraphLayoutIndex() == basicLayout)
    {
        return -1;
    }

    basicGraphInstance = GlobalRenderer::gRenderInstance.CreateAttachmentGraphInstance(mainLogicalDevice, basicLayout);

    if (AttachmentGraphInstanceIndex() == basicGraphInstance)
    {
        return -1;
    }

    std::array<AttachmentClear, 2> clears {
        CLEARCOLOR, {0.0, 0.0, 0.0, 0.0},
        CLEARDEPTH, {1.0, 0}
    };

    rendererRetCode = GlobalRenderer::gRenderInstance.CreateSwapChainAttachment(basicGraphInstance, 0, mainPresentationSwapChain, clears.data(), nullptr, &mainDSVAllocator, {}, mainDSVIndex);

    if (rendererRetCode)
    {
        return -1;
    }

    GlobalRenderer::gRenderInstance.CreateGraphicsQueueForAttachments(basicGraphInstance, 0);

    mainCommandStreamIndex = GlobalRenderer::gRenderInstance.CreateGPUCommandStream(10);

    if (GPUCommandStreamIndex() == mainCommandStreamIndex)
    {
        return -1;
    }

    mainDeviceBuffer = GlobalRenderer::gRenderInstance.CreateUniversalBuffer(mainLogicalDevice, mainDeviceSize, MemoryTypeBits::DEVICE_MEMORY_TYPE);

	if (BufferMemoryIndex() == mainDeviceBuffer)
	{
		return -1;
	}

	mainHostBuffer = GlobalRenderer::gRenderInstance.CreateUniversalBuffer(mainLogicalDevice, mainHostSize, MemoryTypeBits::HOST_MEMORY_COHERENT_TYPE);

	if (BufferMemoryIndex() == mainHostBuffer)
	{
		return -1;
	}

    GenericRenderPipelineInfoIndex pdsHandle = GlobalRenderer::gRenderInstance.CreateGenericRenderPipelineDescription(STRING_VIEW_FROM_LITERAL("BasicGraphicsPipeline.pld"));

    if (GenericRenderPipelineInfoIndex() == pdsHandle)
    {
        return -1;
    }

    RenderShaderGraphIndex shaderGraphHandle = GlobalRenderer::gRenderInstance.CreateShaderGraphInstance(mainLogicalDevice, STRING_VIEW_FROM_LITERAL("BasicShaderLayout.sgr"));

    if (RenderShaderGraphIndex() == shaderGraphHandle)
    {
        return -1;
    }

    std::array frameGraphs = { basicGraphInstance };
    std::array frameRenderPassSelection = { 0 };

    pipelineHandle = GlobalRenderer::gRenderInstance.CreateGraphicRenderStateObject(shaderGraphHandle, pdsHandle, frameGraphs.data(), frameRenderPassSelection.data(), 1);

    if (GeneratedPipelineInstanceIndex() == pipelineHandle)
    {
        return -1;
    }

    std::array<DescriptorTypes, 3> descriptorTypes =
	{
		DescriptorTypes::UNIFORM_DESCRIPTOR,
		DescriptorTypes::SAMPLER_DESCRIPTOR,
		DescriptorTypes::SAMPLED_IMAGE_DESCRIPTOR,
	};

	std::array<uint32_t, 4> descriptorCounts =
	{
		2,
		1,
		2,
	};

	mainDescriptorManagerIndex = GlobalRenderer::gRenderInstance.CreateDescriptorHeap(mainLogicalDevice, descriptorTypes.data(), descriptorCounts.data(), 3, 5, 5);

	if (ShaderResourceManagerIndex() == mainDescriptorManagerIndex)
	{
		return -1;
	}

    uint16_t BoxIndices[36] = {
		2,  1,  0,
		1,  2,  3,
		4,  5,  6,
		7,  6,  5,
		8,  9,  10,
		11, 10, 9,
	   14, 13, 12,
	   13, 14, 15,
	   18, 17, 16,
	   17, 18, 19,
	   20, 21, 22,
	   23, 22, 21
	};

	Vector4f BoxVerts[24] =
	{
		Vector4f(1.0,  1.0,  1.0, 1.0),
		Vector4f(1.0,  1.0, -1.0, 1.0),
		Vector4f(1.0, -1.0,  1.0, 1.0),
		Vector4f(1.0, -1.0, -1.0, 1.0),
		Vector4f(-1.0,  1.0,  1.0, 1.0),
		Vector4f(-1.0,  1.0, -1.0, 1.0),
		Vector4f(-1.0, -1.0,  1.0, 1.0),
		Vector4f(-1.0, -1.0, -1.0, 1.0),
		Vector4f(-1.0,  1.0,  1.0, 1.0),
		Vector4f(1.0,  1.0,  1.0, 1.0),
		Vector4f(-1.0,  1.0, -1.0, 1.0),
		Vector4f(1.0,  1.0, -1.0, 1.0),
		Vector4f(-1.0, -1.0,  1.0, 1.0),
		Vector4f(1.0, -1.0,  1.0, 1.0),
		Vector4f(-1.0, -1.0, -1.0, 1.0),
		Vector4f(1.0, -1.0, -1.0, 1.0),
		Vector4f(-1.0,  1.0,  1.0, 1.0),
		Vector4f(1.0,  1.0,  1.0, 1.0),
		Vector4f(-1.0, -1.0,  1.0, 1.0),
		Vector4f(1.0, -1.0,  1.0, 1.0),
		Vector4f(-1.0,  1.0, -1.0, 1.0),
		Vector4f(1.0,  1.0, -1.0, 1.0),
		Vector4f(-1.0, -1.0, -1.0, 1.0),
		Vector4f(1.0, -1.0, -1.0, 1.0)
	};

    AllocationInstanceIndex globalBufferLocation = GlobalRenderer::gRenderInstance.GetAllocFromBuffer(mainHostBuffer, sizeof(Matrix4f)*2, 1, alignof(Matrix4f), AllocationType::PERFRAME, ComponentFormatType::NO_BUFFER_FORMAT, BufferAlignmentType::UNIFORM_BUFFER_ALIGNMENT, -1, &mainHostAllocator);

	AllocationInstanceIndex vertexAlloc = GlobalRenderer::gRenderInstance.GetAllocFromBuffer(mainDeviceBuffer, sizeof(BoxVerts), 1, 64, AllocationType::STATIC, ComponentFormatType::NO_BUFFER_FORMAT, BufferAlignmentType::NO_BUFFER_ALIGNMENT,  -1, &mainDeviceAllocator);
	AllocationInstanceIndex indexAlloc = GlobalRenderer::gRenderInstance.GetAllocFromBuffer(mainDeviceBuffer, sizeof(BoxIndices), 1, 64, AllocationType::STATIC, ComponentFormatType::NO_BUFFER_FORMAT, BufferAlignmentType::NO_BUFFER_ALIGNMENT, -1, &mainDeviceAllocator);

	GlobalRenderer::gRenderInstance.UpdateDriverMemory(BoxVerts, vertexAlloc, sizeof(BoxVerts), 0, TransferType::CACHED);
	GlobalRenderer::gRenderInstance.UpdateDriverMemory(BoxIndices, indexAlloc, sizeof(BoxIndices), 0, TransferType::CACHED);

	ShaderResourceSetBuilder boxDesc = GlobalRenderer::gRenderInstance.AllocateShaderResourceSet(mainDescriptorManagerIndex, shaderGraphHandle, 0, MAX_FRAMES_IN_FLIGHT);

    ShaderResourceSetContext genericboxRSontext{ &mainAppLogger, false };

	boxDesc.BindBufferToShaderResource(&genericboxRSontext, &globalBufferLocation, 0, 1, 0);

    if (genericboxRSontext.contextFailed)
	{
		genericboxRSontext.contextLogger->ProcessMessage();
		return -1;
	}

    ShaderResourceSetHandle handle = boxDesc();

    GraphicsIntermediaryPipelineInfo basicGraphicsInfo = {
        .vertexBufferHandle = vertexAlloc,
        .vertexCount = 24,
        .pipelinename = pipelineHandle,
        .descCount = 1,
        .descriptorsetid = &handle,
        .indexBufferHandle = indexAlloc,
        .indexCount = 36,
        .instanceCount = 1,
        .indexSize = 2,
        .indirectAllocation = -1,
        .indirectDrawCount = 0,
        .indirectCountAllocation = -1
    };

    basicPipeline = GlobalRenderer::gRenderInstance.CreateGraphicsPipelineObject(&basicGraphicsInfo);

    if (PipelineHandleIndex() == basicPipeline)
    {
        return -1;
    }

    GlobalRenderer::gRenderInstance.AddAttachmentCommandQueue(mainCommandStreamIndex, basicGraphInstance);

    mainCamera.CamLookAt(Vector3f(0.0f, 10.0f, -10.0f), Vector3f(0.0f, 0.0f, 0.0f), Vector3f(0.0f, 1.0f, 0.0f));

	mainCamera.CreateProjectionMatrix(GlobalRenderer::gRenderInstance.GetSwapChainWidth(mainPresentationSwapChain) / (float)GlobalRenderer::gRenderInstance.GetSwapChainHeight(mainPresentationSwapChain), 0.1f, 10000.0f, DegToRad(45.0f));

    mainCamera.UpdateCamera();

    GlobalRenderer::gRenderInstance.UpdateDriverMemory(&mainCamera.View, globalBufferLocation, sizeof(Matrix4f)*2, 0, TransferType::MEMORY);

    graphicsInit = 1;

   // printf("REACHED END OF GRAPHICS INIT\n");

    return 0;
}

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
                done = true;
                break;
            }
            else if (!strcmp(inputBuffer, "show"))
            {
                OSWindowShow(&window);
            }
            else if (!strcmp(inputBuffer, "hide"))
            {
               // OSWindowHide(&window);
            }
            else
            {
                OSWindowSetText(&window, inputBuffer);
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

    GenericWindowInfo info{};

    int closeWindow = 0;

    uint64_t frameCount = 0;

    OSCreateThread(&handle, nullptr, ScanSTDIN, OS_THREAD_NONE);

    closeWindow = OSCreateWindow("Multi Platform Test", windowWidth, windowHeight, &window);

    OSWindowSeedEventBuffer(&window, MainWindowEventBuffer, sizeof(MainWindowEventBuffer));

    if (closeWindow)
    {
        goto end;
    }

    closeWindow = InitGraphicsRuntime();

    if (closeWindow)
    {
        GlobalRenderer::gRenderInstance.DumpLogger();
        goto end;
    }

    OSWindowShow(&window);

    while(!done)
    {
        int ret = OSWindowPollEvents(&window, &info);

        if (info.shouldBeClosed) break;

        if (info.actions[KeyCodes::KC_D].state == PRESSED) break;

        if (ret)
        {
            done = true;
        }

        uint32_t swcImageIndex = GlobalRenderer::gRenderInstance.BeginFrame(mainPresentationSwapChain);

        if (swcImageIndex != (uint32_t)~0)
        {
            GlobalRenderer::gRenderInstance.AddPipelineToRPGraphicsQueue(basicPipeline, basicGraphInstance, 0);

            GlobalRenderer::gRenderInstance.DrawScene(mainLogicalDevice, mainCommandStreamIndex, swcImageIndex);

            int presentRet = GlobalRenderer::gRenderInstance.SubmitFrame(mainPresentationSwapChain, swcImageIndex);

            if (presentRet)
            {
                
            }

            GlobalRenderer::gRenderInstance.EndFrame(mainLogicalDevice, mainCommandStreamIndex);
        } 
        else
        {
            
            printf("Something is happening!!!\n");
        }
    }
end:
    done = true;

    GlobalRenderer::gRenderInstance.DestroyRenderInstance();

    CloseAllFiles();

    CloseAllSyncObject();

    CloseAllThreads();

    CloseAllWindows();
    
    return retCode;
}