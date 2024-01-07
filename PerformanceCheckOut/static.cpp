#include "static.h"

DWORD WINAPI threadStaticTransform(LPVOID lpParam) {
    LPSTATIC_THREAD_PARAMS params = (LPSTATIC_THREAD_PARAMS)lpParam;
    DWORD64 remainingSize = params->partSize;
    DWORD64 startOffset = params->startOffset;

    BYTE buffer[CHUNK_SIZE + 1];
    DWORD bytesRead;

    while (remainingSize > 0) {
        memset(buffer, 0, sizeof(buffer));
        DWORD chunkSize = min(CHUNK_SIZE, remainingSize);

        CRITICAL_SECTION_OPERATION(params->readCriticalSection,
            SetFilePointer(params->hImage, startOffset, NULL, FILE_BEGIN);
        CHECK(ReadFile(params->hImage, buffer, chunkSize, &bytesRead, NULL), -1, "Failed to read from hImage",
            LeaveCriticalSection(params->readCriticalSection));
        );

        if (bytesRead == 0) {
            break;
        }

        for (DWORD i = 0; i < bytesRead; i += 4 * sizeof(BYTE)) {
            LPRGBA_PIXEL pixel = (LPRGBA_PIXEL)(buffer + i * sizeof(BYTE));
            params->pixelTransform(pixel);
        }

        CRITICAL_SECTION_OPERATION(params->writeCriticalSection,
            SetFilePointer(params->hResult, startOffset, NULL, FILE_BEGIN);
        CHECK(WriteFile(params->hResult, buffer, bytesRead, NULL, NULL), -1, "Failed to write to result file",
            LeaveCriticalSection(params->writeCriticalSection));
        );

        startOffset += bytesRead;
        remainingSize -= bytesRead;
    }

    return 0;
}

DWORD fileTransformParallelStatic(HANDLE hImage, HANDLE hResult, PixelTransformFunction pixelTransform, DWORD bfOffBits, DWORD threadCount) {
    HANDLE* threads = new HANDLE[threadCount];
    LPSTATIC_THREAD_PARAMS threadParams = new STATIC_THREAD_PARAMS[threadCount];

    CRITICAL_SECTION writeCriticalSection;
    InitializeCriticalSection(&writeCriticalSection);

    CRITICAL_SECTION readCriticalSection;
    InitializeCriticalSection(&readCriticalSection);

    LARGE_INTEGER fileSizeLI;
    CHECK(GetFileSizeEx(hImage, &fileSizeLI), -1, "Failed to get file size", delete[] threads, delete[] threadParams,
        DeleteCriticalSection(&writeCriticalSection), DeleteCriticalSection(&readCriticalSection));

    DWORD64 fileSize = (fileSizeLI.QuadPart - bfOffBits);
    DWORD64 partSize = (DWORD)(std::ceil((fileSize / 4) / (float)threadCount) * 4);

    for (DWORD i = 0; i < threadCount; ++i) {
        threadParams[i].hImage = hImage;
        threadParams[i].hResult = hResult;
        threadParams[i].startOffset = bfOffBits + (i * partSize);
        threadParams[i].partSize = (i == threadCount - 1) ? (fileSize - (i * partSize)) : partSize;
        threadParams[i].pixelTransform = pixelTransform;
        threadParams[i].writeCriticalSection = &writeCriticalSection;
        threadParams[i].readCriticalSection = &readCriticalSection;

        threads[i] = CreateThread(NULL, 0, threadStaticTransform, &threadParams[i], 0, NULL);
    }

    WaitForMultipleObjects(threadCount, threads, TRUE, INFINITE);

    for (DWORD i = 0; i < threadCount; ++i) {
        CLOSE_HANDLES(threads[i]);
    }

    DeleteCriticalSection(&writeCriticalSection);
    DeleteCriticalSection(&readCriticalSection);

    delete[] threads;
    delete[] threadParams;
    return 0;
}