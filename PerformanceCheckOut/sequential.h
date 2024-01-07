#pragma once
#include "constants.h"

DWORD fileTransformSequential(HANDLE hImage, HANDLE hResult, PixelTransformFunction pixelTransform, DWORD bfOffBits, DWORD threadCount);