#pragma once
#include "constants.h"
#include "static.h"

DWORD WINAPI threadDynamicTransform(LPVOID lpParam);

DWORD fileTransformParallelDynamic(HANDLE hImage, HANDLE hResult, PixelTransformFunction pixelTransform, DWORD bfOffBits, DWORD threadCount);