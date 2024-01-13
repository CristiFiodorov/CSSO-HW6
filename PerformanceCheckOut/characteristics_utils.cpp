#include "characteristics_utils.h"


VOID getStringFileHeaderData(BITMAPFILEHEADER bMapFileHeader, std::string& output) {
    output += std::format("File Header Data\n");
    output += std::format("bfType: {}{}\n", bMapFileHeader.bfType, bMapFileHeader.bfType >> 8);
    output += std::format("bfSize: {}\n", bMapFileHeader.bfSize);
    output += std::format("bfReserved1: {}\n", bMapFileHeader.bfReserved1);
    output += std::format("bfReserved2: {}\n", bMapFileHeader.bfReserved2);
    output += std::format("bfOffBits: {}\n", bMapFileHeader.bfOffBits);
}

VOID getStringInfoHeaderData(BITMAPINFOHEADER bMapInfoHeader, std::string& output) {
    output += std::format("Info Header Data\n");
    output += std::format("biSize: {}\n", bMapInfoHeader.biSize);
    output += std::format("biWidth: {}\n", bMapInfoHeader.biWidth);
    output += std::format("biHeight: {}\n", bMapInfoHeader.biHeight);
    output += std::format("biPlanes: {}\n", bMapInfoHeader.biPlanes);
    output += std::format("biBitCount: {}\n", bMapInfoHeader.biBitCount);
    output += std::format("biCompression: {}\n", bMapInfoHeader.biCompression);
    output += std::format("biSizeImage: {}\n", bMapInfoHeader.biSizeImage);
    output += std::format("biXPelsPerMeter: {}\n", bMapInfoHeader.biXPelsPerMeter);
    output += std::format("biYPelsPerMeter: {}\n", bMapInfoHeader.biYPelsPerMeter);
    output += std::format("biClrUsed: {}\n", bMapInfoHeader.biClrUsed);
    output += std::format("biClrImportant: {}\n", bMapInfoHeader.biClrImportant);
}

DWORD getStringProcessorMasksAndRelationships(std::string& output) {
    LPFN_GLPI glpi = (LPFN_GLPI)GetProcAddress(GetModuleHandle(TEXT("kernel32")), "GetLogicalProcessorInformation");
    CHECK(glpi, -1, "GetLogicalProcessorInformation function is not supported");
    PSYSTEM_LOGICAL_PROCESSOR_INFORMATION buffer = NULL;
    DWORD returnLength = 0;

    while (1) {
        DWORD rc = glpi(buffer, &returnLength);
        if (rc) {
            break;
        }
        CHECK(GetLastError() == ERROR_INSUFFICIENT_BUFFER, -1, "Unexpected error") {
            if (buffer) {
                free(buffer);
            }
            buffer = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION)malloc(returnLength);
            CHECK(buffer, -1, "Allocation Failure");
        }
    }

    CHECK(glpi(buffer, &returnLength), -1, "Failed to extract processor information", free(buffer));
    DWORD byteOffset = 0;

    PSYSTEM_LOGICAL_PROCESSOR_INFORMATION ptr = buffer;
    while (byteOffset + sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION) <= returnLength) {
        output += std::format("Processor Mask: {}\n", ptr->ProcessorMask);
        switch (ptr->Relationship) {
        case RelationProcessorCore:
            output += std::format("Relationship: RelationProcessorCore\n");
            break;
        case RelationNumaNode:
            output += std::format("Relationship: RelationNumaNode\n");
            break;
        case RelationCache:
            output += std::format("Relationship: RelationCache\n");
            break;
        case RelationProcessorPackage:
            output += std::format("Relationship: RelationProcessorPackage\n");
            break;
        default:
            output += std::format("Relationship: Unknown\n");
            break;
        }
        byteOffset += sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);
        ptr++;
    }

    free(buffer);
    return 0;
}

DWORD getStringAffinityMasksAndNumaNodesCounts(std::string& output) {
    PDWORD_PTR processAffinityMask, systemAffinityMask;
    processAffinityMask = (PDWORD_PTR)malloc(sizeof(DWORD_PTR));
    systemAffinityMask = (PDWORD_PTR)malloc(sizeof(DWORD_PTR));

    CHECK(GetProcessAffinityMask(GetCurrentProcess(), processAffinityMask, systemAffinityMask), -1, "Could not extract affinity masks", 
        free(processAffinityMask), free(systemAffinityMask));
    output += std::format("Current Process' affinity mask: {}\n", *processAffinityMask);
    output += std::format("Current System's affinity mask: {}\n", *systemAffinityMask);

    ULONG highestNodeNumber;

    CHECK(GetNumaHighestNodeNumber(&highestNodeNumber), -1, "Could not extract the highest numa node",
        free(processAffinityMask), free(systemAffinityMask));
    output += std::format("Number of NUMA nodes: {}\n", highestNodeNumber + 1);

    free(processAffinityMask);
    free(systemAffinityMask);
    return 0;
}

DWORD getStringProcessDefaultCpuSetsInformation(std::string& output) {
    ULONG returnedLength;
    PULONG cpuSetIds = NULL;
    ULONG bufferSize = 0;

    while (1) {
        DWORD rc = GetProcessDefaultCpuSets(GetCurrentProcess(), cpuSetIds, bufferSize, &returnedLength);
        if (rc) {
            break;
        }
        CHECK(GetLastError() == ERROR_INSUFFICIENT_BUFFER, -1, "Unexpected error when extracting cpuSetIds");
        if (cpuSetIds) {
            free(cpuSetIds);
        }
        bufferSize = returnedLength;
        cpuSetIds = (PULONG)malloc(bufferSize);
        CHECK(cpuSetIds, -1, "Allocation Failure");
    }

    DWORD byteOffset = 0;
    PULONG ptr = cpuSetIds;
    DWORD index = 0;

    output += std::format("Process Deafult Cpu Information:\n");
    while (byteOffset + sizeof(ULONG) <= bufferSize) {
        output += std::format("Entry {}: {}\n", index, *ptr);

        index++;
        ptr++;
        byteOffset += sizeof(ULONG);
    }

    output += std::format("Number of cpuSetIds elements processed: {}\n", index);
    free(cpuSetIds);
    return 0;
}

DWORD getStringSystemCpuSetsInformation(std::string& output, LPDWORD nrCPU) {
    ULONG returnedLength;
    PSYSTEM_CPU_SET_INFORMATION cpuSetIds = NULL;
    ULONG bufferSize = 0;

    while (1) {
        DWORD rc = GetSystemCpuSetInformation(cpuSetIds, bufferSize, &returnedLength, GetCurrentProcess(), 0);
        if (rc) {
            break;
        }
        CHECK(GetLastError() == ERROR_INSUFFICIENT_BUFFER, -1, "Unexpected error when extracting cpuSetIds");
        if (cpuSetIds) {
            free(cpuSetIds);
        }
        bufferSize = returnedLength;
        cpuSetIds = (PSYSTEM_CPU_SET_INFORMATION)malloc(bufferSize);
        CHECK(cpuSetIds, -1, "Allocation Failure");
    }

    DWORD byteOffset = 0;
    PSYSTEM_CPU_SET_INFORMATION ptr = cpuSetIds;
    DWORD index = 0;

    output += std::format("System Cpu Information:\n");
    while (byteOffset + ptr->Size <= bufferSize) {
        output += std::format("Entry #{}\n", index);
        output += std::format("    Id: {}\n", ptr->CpuSet.Id);
        output += std::format("    Group: {}\n", ptr->CpuSet.Group);
        output += std::format("    LogicalProcessorIndex: {}\n", ptr->CpuSet.LogicalProcessorIndex);
        output += std::format("    CoreIndex: {}\n", ptr->CpuSet.CoreIndex);
        output += std::format("    LastLevelCacheIndex: {}\n", ptr->CpuSet.LastLevelCacheIndex);
        output += std::format("    NumaNodeIndex: {}\n", ptr->CpuSet.NumaNodeIndex);
        output += std::format("    EfficiencyClass: {}\n", ptr->CpuSet.EfficiencyClass);

        output += std::format("    AllFlags: 0x{}\n", ptr->CpuSet.AllFlags);
        output += std::format("    Parked: {}\n", (BYTE)ptr->CpuSet.Parked);
        output += std::format("    Allocated: {}\n", (BYTE)ptr->CpuSet.Allocated);
        output += std::format("    AllocatedToTargetProcess: {}\n", (BYTE)ptr->CpuSet.AllocatedToTargetProcess);
        output += std::format("    RealTime: {}\n", (BYTE)ptr->CpuSet.RealTime);
        output += std::format("    ReservedFlags: {}\n", (BYTE)ptr->CpuSet.ReservedFlags);

        output += std::format("    Reserved: {}\n", ptr->CpuSet.Reserved);
        output += std::format("    SchedulingClass: {}\n", ptr->CpuSet.SchedulingClass);

        index++;
        byteOffset += ptr->Size;
        ptr++;
    }

    output += std::format("The number of CPU_SET_INFORMATION structures processed: {}\n", index);
    if (nrCPU) {
        *nrCPU = index;
    }
    free(cpuSetIds);

    return 0;
}

DWORD getStringCpuSetsInformation(std::string& output) {
    CHECK(getStringProcessDefaultCpuSetsInformation(output) == 0, -1, "Failed to print process default cpu sets information");
    CHECK(getStringSystemCpuSetsInformation(output, NULL) == 0, -1, "Failed to print system cpu sets information");

    return 0;
}

DWORD writeComputerCharacteristics(LPCSTR filePath, std::string& computerCharacteristics) {
    computerCharacteristics.reserve(1000);
    CHECK(getStringCurrentUserSid(computerCharacteristics) == 0, -1, "Could not print the current user SID");
    CHECK(getStringWellKnownSid(WinWorldSid, "Everyone Group", computerCharacteristics) == 0, -1, "Could not print the everyone group SID");
    CHECK(getStringWellKnownSid(WinBuiltinAdministratorsSid, "Administrators Group", computerCharacteristics) == 0, -1, "Could not print the administrators group SID");
    CHECK(getStringProcessorMasksAndRelationships(computerCharacteristics) == 0, -1, "Could not print processor masks and relationships");
    CHECK(getStringAffinityMasksAndNumaNodesCounts(computerCharacteristics) == 0, -1, "Could not print affinity masks and numa nodes count");
    CHECK(getStringCpuSetsInformation(computerCharacteristics) == 0, -1, "Could not print CPU sets information");

    HANDLE hFile = CreateFile(filePath, GENERIC_WRITE, FILE_SHARE_READ, NULL, FILE_APPEND_DATA, FILE_ATTRIBUTE_NORMAL, 0);
    CHECK(hFile != INVALID_HANDLE_VALUE, -2, "Opening the file failed");

    LPCSTR cString = computerCharacteristics.c_str();
    CHECK(WriteFile(hFile, cString, strlen(cString), NULL, NULL), -2, "Error when writing in file", CLOSE_HANDLES(hFile));

    CLOSE_HANDLES(hFile);

    return 0;
}


std::string getStringFromTestResults(const std::vector<TEST_RESULT>& testResults) {
    std::string stringTestResults;
    for (auto testResult : testResults) {
        stringTestResults += std::format("Testing Method: {}, Nr workers: {}, Operation Name: {}, Elapsed time: {}ms \r\n\r\n", testResult.testingMethod, testResult.nrWorkers, testResult.OperationName, testResult.elapsedMilliseconds);
    }
    return stringTestResults;
}

std::string getCSVContentFromTestsResults(const std::vector<TEST_RESULT>& testResults) {
    std::string CSVOutput;
    CSVOutput += "Method,Transform Type,Workers Count,Time\r\n";

    for (auto testResult : testResults) {
        CSVOutput += std::format("{},{},{},{}\r\n", testResult.testingMethod, testResult.OperationName, testResult.nrWorkers, testResult.elapsedMilliseconds);
    }
    return CSVOutput;
}