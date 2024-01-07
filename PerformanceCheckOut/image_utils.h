#pragma once
#include "constants.h"
#include "security_utils.h"

DWORD extractBmpHeaders(HANDLE hImage, PBITMAPFILEHEADER bMapFileHeader, PBITMAPINFOHEADER bMapInfoHeader);

DWORD appendBmapHeadersToFile(HANDLE hFile, BITMAPFILEHEADER& bMapFileHeader, BITMAPINFOHEADER& bMapInfoHeader);

DWORD applyPixelGrayscaleTransform(LPRGBA_PIXEL pixel);

DWORD applyInvertBytesTransform(LPRGBA_PIXEL pixel);

DWORD applyImageTransformation(DWORD nrCPU, HANDLE hImage, LPCSTR imageName,
    BITMAPFILEHEADER& bMapFileHeader, BITMAPINFOHEADER& bMapInfoHeader,
    FileTransformFunction fileTransform, PixelTransformFunction pixelTransform,
    LPCSTR resultFolder, LPCSTR operationName);

