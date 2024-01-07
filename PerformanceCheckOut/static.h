#pragma once
#include "constants.h"

DWORD WINAPI threadStaticTransform(LPVOID lpParam);

DWORD fileTransformParallelStatic(HANDLE hImage, HANDLE hResult, PixelTransformFunction pixelTransform, DWORD bfOffBits, DWORD threadCount);