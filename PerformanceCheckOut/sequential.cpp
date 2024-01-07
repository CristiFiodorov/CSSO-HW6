#include "sequential.h"

DWORD fileTransformSequential(HANDLE hImage, HANDLE hResult, PixelTransformFunction pixelTransform, DWORD bfOffBits, DWORD threadCount) {
    while (true) {
        BYTE buffer[CHUNK_SIZE + 1];
        DWORD bytesRead;
        memset(buffer, 0, sizeof(buffer));
        CHECK(ReadFile(hImage, buffer, CHUNK_SIZE, &bytesRead, NULL), -1, "Reading from a file failed");
        if (!bytesRead) {
            break;
        }
        for (DWORD i = 0; i < bytesRead; i += 4 * sizeof(BYTE)) {
            LPRGBA_PIXEL pixel = (LPRGBA_PIXEL)(buffer + i * sizeof(BYTE));
            pixelTransform(pixel);
        }
        CHECK(WriteFile(hResult, buffer, bytesRead, NULL, NULL), -1, "Error when writing into file");
    }

    return 0;
}