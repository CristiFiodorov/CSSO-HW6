#pragma once
#include "constants.h"
#include "security_utils.h"
#include "characteristics_utils.h"
#include "static.h"
#include "dynamic.h"
#include "sequential.h"

DWORD extractBmpHeaders(HANDLE hImage, PBITMAPFILEHEADER bMapFileHeader, PBITMAPINFOHEADER bMapInfoHeader);

DWORD appendBmapHeadersToFile(HANDLE hFile, BITMAPFILEHEADER& bMapFileHeader, BITMAPINFOHEADER& bMapInfoHeader);

DWORD applyPixelGrayscaleTransform(LPRGBA_PIXEL pixel);

DWORD applyInvertBytesTransform(LPRGBA_PIXEL pixel);

DWORD applyImageTransformation(DWORD nrCPU, HANDLE hImage, LPCSTR imageName,
    BITMAPFILEHEADER& bMapFileHeader, BITMAPINFOHEADER& bMapInfoHeader,
    FileTransformFunction fileTransform, PixelTransformFunction pixelTransform,
    LPCSTR resultFolder, LPCSTR guiResultsFolder, LPCSTR operationName, 
    std::string& outputPath);


DWORD applyImageTransformations(LPCSTR imagePath, DWORD totalNrCPU, const std::set<TransformationUtil>& transformationUtils, std::string& stringFileHeaderData, std::string& stringInfoHeaderData, 
    std::string& grayscaleOutputPath, std::string& invertOutputPath, std::vector<TEST_RESULT>& testResults);