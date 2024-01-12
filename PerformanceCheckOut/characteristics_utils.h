#pragma once
#include "constants.h"
#include "security_utils.h"

/*
    Printing the "File Header" of a bmp images on the screen 
    Maybe we should change this to append to a file
*/
VOID getStringFileHeaderData(BITMAPFILEHEADER bMapFileHeader, std::string& output);

/*
    Printing the "Info Header" of a bmp images on the screen
    Maybe we should change this to append to a file
*/
VOID getStringInfoHeaderData(BITMAPINFOHEADER bMapInfoHeader, std::string& output);

DWORD getStringProcessorMasksAndRelationships(std::string& output);

DWORD getStringAffinityMasksAndNumaNodesCounts(std::string& output);

DWORD getStringProcessDefaultCpuSetsInformation(std::string& output);

DWORD getStringSystemCpuSetsInformation(std::string& output, LPDWORD nrCPU);

DWORD getStringCpuSetsInformation(std::string& output);

DWORD writeComputerCharacteristics(LPCSTR filePath, std::string& computerCharacteristics);