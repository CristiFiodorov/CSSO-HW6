#pragma once
#include "constants.h"
#include "security_utils.h"
#include "characteristics_utils.h"
#include "static.h"
#include "dynamic.h"
#include "sequential.h"

DWORD extractBmpHeaders(HANDLE hImage, PBITMAPFILEHEADER bMapFileHeader, PBITMAPINFOHEADER bMapInfoHeader);

DWORD appendBmapHeadersToFile(HANDLE hFile, const BITMAPFILEHEADER& bMapFileHeader, const BITMAPINFOHEADER& bMapInfoHeader);

DWORD applyPixelGrayscaleTransform(LPRGBA_PIXEL pixel);

DWORD applyInvertBytesTransform(LPRGBA_PIXEL pixel);

DWORD applyImageTransformation(const BMP_IMAGE_INFO& bmpImageInfo, const TRANSFORMATION_INFO& transformationInfo,
    LPCSTR resultFolder, LPCSTR guiResultsFolder, std::string& outputPath);


DWORD applyImageTransformations(LPCSTR imagePath, DWORD totalNrCPU, const std::set<TRANSFORMATION_UTIL>& transformationUtils, const IMAGE_TRANSFORMATION_RESULTS& imageTransformationResults);