#include "dynamic.h"

DWORD WINAPI threadDynamicTransform(LPVOID lpParam) {
    LPDYNAMIC_THREAD_PARAMS params = (LPDYNAMIC_THREAD_PARAMS)lpParam;
    LPDYNAMIC_THREAD_REQUEST threadRequest = params->threadRequest;

    STATIC_THREAD_PARAMS staticThreadParams = {
        .hImage = params->hImage,
        .hResult = params->hResult,
        .pixelTransform = params->pixelTransform,
        .writeCriticalSection = params->writeCriticalSection,
        .readCriticalSection = params->readCriticalSection
    };


    while (true) {
        if (threadRequest->requestStatus == FINISHED) {
            break;
        }
        if (threadRequest->requestStatus == PENDING) {
            continue;
        }
        CRITICAL_SECTION_OPERATION(params->mapCriticalSection,
            if (threadRequest->requestStatus != READY) {
                LeaveCriticalSection(params->mapCriticalSection);
                continue;
            }
        staticThreadParams.partSize = params->threadRequest->partSize;
        staticThreadParams.startOffset = params->threadRequest->startOffset;
        threadRequest->requestStatus = PENDING;
        );
        threadStaticTransform((LPVOID)&staticThreadParams);
    }

    return 0;
}

DWORD fileTransformParallelDynamic(HANDLE hImage, HANDLE hResult, PixelTransformFunction pixelTransform, DWORD bfOffBits, DWORD threadCount) {
    HANDLE* threads = new HANDLE[threadCount];
    LPDYNAMIC_THREAD_PARAMS threadParams = new DYNAMIC_THREAD_PARAMS[threadCount];

    CRITICAL_SECTION writeCriticalSection;
    InitializeCriticalSection(&writeCriticalSection);

    CRITICAL_SECTION readCriticalSection;
    InitializeCriticalSection(&readCriticalSection);

    CRITICAL_SECTION mapCriticalSection;
    InitializeCriticalSection(&mapCriticalSection);


    DYNAMIC_THREAD_REQUEST threadRequest;
    threadRequest.startOffset = bfOffBits;
    threadRequest.requestStatus = READY;

    LARGE_INTEGER fileSizeLI;
    CHECK(GetFileSizeEx(hImage, &fileSizeLI), -1, "Failed to get file size", delete[] threads, delete[] threadParams,
        DeleteCriticalSection(&writeCriticalSection), DeleteCriticalSection(&readCriticalSection), DeleteCriticalSection(&mapCriticalSection));

    DWORD64 remainingBytes = (fileSizeLI.QuadPart - bfOffBits);
    threadRequest.partSize = ((remainingBytes / 4) / (2 * threadCount)) * 4;

    DWORD64 nextOffset = threadRequest.startOffset + threadRequest.partSize;
    remainingBytes -= threadRequest.partSize;

    for (int i = 0; i < threadCount; ++i) {
        threadParams[i].hImage = hImage;
        threadParams[i].hResult = hResult;
        threadParams[i].pixelTransform = pixelTransform;
        threadParams[i].writeCriticalSection = &writeCriticalSection;
        threadParams[i].readCriticalSection = &readCriticalSection;
        threadParams[i].mapCriticalSection = &mapCriticalSection;
        threadParams[i].threadRequest = &threadRequest;
        threads[i] = CreateThread(NULL, 0, threadDynamicTransform, &threadParams[i], 0, NULL);
    }

    while (true) {
        if (threadRequest.requestStatus == READY) {
            continue;
        }
        CRITICAL_SECTION_OPERATION(&mapCriticalSection,
            if (remainingBytes <= 0) {
                threadRequest.requestStatus = FINISHED;
                LeaveCriticalSection(&mapCriticalSection);
                break;
            }
        threadRequest.startOffset = nextOffset;
        threadRequest.partSize = ((remainingBytes / 4) / (2 * threadCount)) * 4;
        if (threadRequest.partSize == 0) {
            threadRequest.partSize = remainingBytes;
        }

        threadRequest.requestStatus = READY;
        nextOffset = threadRequest.startOffset + threadRequest.partSize;
        remainingBytes -= threadRequest.partSize;
        );
    }

    WaitForMultipleObjects(threadCount, threads, TRUE, INFINITE);

    for (int i = 0; i < threadCount; ++i) {
        CLOSE_HANDLES(threads[i]);
    }


    DeleteCriticalSection(&writeCriticalSection);
    DeleteCriticalSection(&readCriticalSection);
    DeleteCriticalSection(&mapCriticalSection);

    delete[] threads;
    delete[] threadParams;
    return 0;
}