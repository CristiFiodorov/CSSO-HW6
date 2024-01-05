#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <Shlobj.h>
#include <Aclapi.h>
#include <sddl.h>
#include <winnt.h>
#include <string>
#include <format>


#pragma comment(lib, "Shlwapi.lib")
#include <shlwapi.h>

#include "utils.h"


DWORD extractBmpHeaders(HANDLE hImage, PBITMAPFILEHEADER bMapFileHeader, PBITMAPINFOHEADER bMapInfoHeader) {
    CHECK(ReadFile(hImage, bMapFileHeader, FILE_HEADER_SIZE, NULL, NULL), -1, "Reading the file header failed");
    CHECK(ReadFile(hImage, bMapInfoHeader, INFO_HEADER_SIZE, NULL, NULL), -1, "Reading the info header failed");
    return 0;
}

VOID printFileHeaderData(BITMAPFILEHEADER bMapFileHeader) {
    printf("File Header Data\n");
    printf("bfType: %c%c\n", bMapFileHeader.bfType, bMapFileHeader.bfType >> 8);
    printf("bfSize: %d\n", bMapFileHeader.bfSize);
    printf("bfReserved1: %d\n", bMapFileHeader.bfReserved1);
    printf("bfReserved2: %d\n", bMapFileHeader.bfReserved2);
    printf("bfOffBits: %d\n", bMapFileHeader.bfOffBits);
    printf("\n");
}

VOID printInfoHeaderData(BITMAPINFOHEADER bMapInfoHeader) {
    printf("Info Header Data\n");
    printf("biSize: %d\n", bMapInfoHeader.biSize);
    printf("biWidth: %d\n", bMapInfoHeader.biWidth);
    printf("biHeight: %d\n", bMapInfoHeader.biHeight);
    printf("biPlanes: %d\n", bMapInfoHeader.biPlanes);
    printf("biBitCount: %d\n", bMapInfoHeader.biBitCount);
    printf("biCompression: %d\n", bMapInfoHeader.biCompression);
    printf("biSizeImage: %d\n", bMapInfoHeader.biSizeImage);
    printf("biXPelsPerMeter: %d\n", bMapInfoHeader.biXPelsPerMeter);
    printf("biYPelsPerMeter: %d\n", bMapInfoHeader.biYPelsPerMeter);
    printf("biClrUsed: %d\n", bMapInfoHeader.biClrUsed);
    printf("biClrImportant: %d\n", bMapInfoHeader.biClrImportant);
}

PTOKEN_USER getCurrentUserToken() {
    HANDLE hToken;
    CHECK(OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken), NULL, "Failed to open process token");
    DWORD dwBufferSize = 0;
    GetTokenInformation(hToken, TokenUser, NULL, 0, &dwBufferSize);
    PTOKEN_USER pTokenUser = (PTOKEN_USER) new BYTE[dwBufferSize];
    CHECK(GetTokenInformation(hToken, TokenUser, pTokenUser, dwBufferSize, &dwBufferSize), NULL,
        "Failed GetTokenInformation", CLOSE_HANDLES(hToken); delete[] pTokenUser);
    CLOSE_HANDLES(hToken);
    return pTokenUser;
}

DWORD getStringCurrentUserSid(std::string& output) {
    PTOKEN_USER pTokenUser = getCurrentUserToken();
    CHECK(pTokenUser, -1, "Failed to extract the current user token", delete[] pTokenUser);

    LPSTR szUserSid;
    CHECK(ConvertSidToStringSid(pTokenUser->User.Sid, &szUserSid), -1, "Could not obtain the string representation of a SID", delete[] pTokenUser);
    
    output += std::format("Current user SID: {}\n", szUserSid);

    LocalFree(szUserSid);
    delete[] pTokenUser;
    return 0;
}

PSID getWellKnownSid(WELL_KNOWN_SID_TYPE sidType) {
    DWORD sizeOfPSID = SECURITY_MAX_SID_SIZE;
    PSID pSID = (PSID) new BYTE[sizeOfPSID];
    CHECK(CreateWellKnownSid(sidType, NULL, pSID, &sizeOfPSID),
        NULL, "Error at creating a wellknown SID", delete[] pSID);
    return pSID;
}

DWORD getStringWellKnownSid(WELL_KNOWN_SID_TYPE sidType, LPCSTR label, std::string& output) {
    PSID pSid = getWellKnownSid(sidType);
    CHECK(pSid, -1, "Failed to extract the everyone group sid");

    LPSTR szSid;
    CHECK(ConvertSidToStringSid(pSid, &szSid), -1, "Could not obtain the string representation of a SID", delete[] pSid);

    output += std::format("{} SID: {}\n", label, szSid);

    LocalFree(szSid);
    delete[] pSid;
    return 0;
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
            buffer = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION) malloc(returnLength);
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

    CHECK(GetProcessAffinityMask(GetCurrentProcess(), processAffinityMask, systemAffinityMask), -1, "Could not extract affinity masks");
    output += std::format("Current Process' affinity mask: {}\n", *processAffinityMask);
    output += std::format("Current System's affinity mask: {}\n", *systemAffinityMask);
    
    ULONG highestNodeNumber;

    CHECK(GetNumaHighestNodeNumber(&highestNodeNumber), -1, "Could not extract the highest numa node");
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
        cpuSetIds = (PULONG) malloc(bufferSize);
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


DWORD getStringSystemCpuSetsInformation(std::string& output) {
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
        cpuSetIds = (PSYSTEM_CPU_SET_INFORMATION) malloc(bufferSize);
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
    free(cpuSetIds);
}

DWORD getStringCpuSetsInformation(std::string& output) {
    CHECK(getStringProcessDefaultCpuSetsInformation(output) == 0, -1, "Failed to print process default cpu sets information");
    CHECK(getStringSystemCpuSetsInformation(output) == 0, -1, "Failed to print system cpu sets information");
}

DWORD setFileReadPermissionOnly(LPSTR filename) {
    PTOKEN_USER pTokenUser = getCurrentUserToken();
    CHECK(pTokenUser, -1, "Failed to extract the current user token");

    EXPLICIT_ACCESS eaAllow;
    memset(&eaAllow, 0, sizeof(EXPLICIT_ACCESS));
    eaAllow.grfAccessPermissions = GENERIC_READ;
    eaAllow.grfAccessMode = SET_ACCESS;
    eaAllow.grfInheritance = NO_INHERITANCE;
    eaAllow.Trustee.TrusteeForm = TRUSTEE_IS_SID;
    eaAllow.Trustee.TrusteeType = TRUSTEE_IS_USER;
    eaAllow.Trustee.ptstrName = (LPSTR)pTokenUser->User.Sid;

    PACL pACL;
    CHECK(SetEntriesInAcl(1, &eaAllow, NULL, &pACL) == ERROR_SUCCESS, -1, "Fail to set entries in ACL",
        LocalFree(pACL), delete[] pTokenUser);

    CHECK(SetNamedSecurityInfo(filename, SE_FILE_OBJECT, DACL_SECURITY_INFORMATION | PROTECTED_DACL_SECURITY_INFORMATION, NULL, NULL, pACL, NULL) == ERROR_SUCCESS,
        -1, "Error in function SetNamedSecurityInfo", LocalFree(pACL), delete[] pTokenUser);

    delete[] pTokenUser;
    LocalFree(pACL);

    return 0;
}

DWORD appendBmapHeadersToFile(HANDLE hFile, BITMAPFILEHEADER& bMapFileHeader, BITMAPINFOHEADER& bMapInfoHeader) {
    DWORD bytesWritten;

    CHECK(WriteFile(hFile, &bMapFileHeader, sizeof(bMapFileHeader), &bytesWritten, NULL), -1, "Error when writing into file");
    CHECK(bytesWritten == sizeof(bMapFileHeader), -1, "The file header could not be written");

    CHECK(WriteFile(hFile, &bMapInfoHeader, sizeof(bMapInfoHeader), &bytesWritten, NULL), -1, "Error when writing into file");
    CHECK(bytesWritten == sizeof(bMapInfoHeader), -1, "The info header could not be written");

    return 0;
}

DWORD applyPixelGrayscaleTransform(LPRGBA_PIXEL pixel) {
    BYTE grayscaleFactor = pixel->red * GRAYSCALE_RED_COEFF + pixel->green * GRAYSCALE_GREEN_COEFF + pixel->blue * GRAYSCALE_BLUE_COEFF;
    memset(pixel, grayscaleFactor, 3 * sizeof(BYTE));
    return 0;
}

DWORD applyInvertBytesTransform(LPRGBA_PIXEL pixel) {
    pixel->red = 0xFF - pixel->red;
    pixel->blue = 0xFF - pixel->blue;
    pixel->green = 0xFF - pixel->green;
    return 0;
}

DWORD applyImageTransformation(HANDLE hImage, LPCSTR imageName, LPCSTR operationName, BITMAPFILEHEADER& bMapFileHeader, BITMAPINFOHEADER& bMapInfoHeader,
    PixelTransformFunction pixelTransform) {
    LARGE_INTEGER startTime, endTime;
    QueryPerformanceCounter(&startTime);

    CHECK(SetFilePointer(hImage, bMapFileHeader.bfOffBits, NULL, FILE_BEGIN) != INVALID_SET_FILE_POINTER, -1, "Setting the file pointer failed");

    CHAR resultImagePath[MAX_PATH];
    sprintf_s(resultImagePath, "%s\\%s_%s_temp.bmp", RESULTS_SEQ_FOLDER, imageName, operationName);

    HANDLE hResult = CreateFile(resultImagePath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    CHECK(hResult != INVALID_HANDLE_VALUE, -1, "Opening the results file failed");

    CHECK(appendBmapHeadersToFile(hResult, bMapFileHeader, bMapInfoHeader) == 0, -1, "Headers could not be appended", CLOSE_HANDLES(hResult));

    while (true) {
        CHAR buffer[CHUNK_SIZE + 1];
        DWORD bytesRead;
        memset(buffer, 0, sizeof(buffer));
        CHECK(ReadFile(hImage, buffer, CHUNK_SIZE, &bytesRead, NULL), -1, "Reading from a file failed", CLOSE_HANDLES(hResult));
        if (!bytesRead) {
            break;
        }
        for (DWORD i = 0; i < bytesRead; i += 4 * sizeof(BYTE)) {
            LPRGBA_PIXEL pixel = (LPRGBA_PIXEL)(buffer + i * sizeof(BYTE));
            pixelTransform(pixel);
        }
        CHECK(WriteFile(hResult, buffer, bytesRead, NULL, NULL), -1, "Error when writing into file", CLOSE_HANDLES(hResult));
    }

    CLOSE_HANDLES(hResult);

    QueryPerformanceCounter(&endTime);

    LARGE_INTEGER frequency;
    QueryPerformanceFrequency(&frequency);
    DWORD elapsedMilliseconds = (endTime.QuadPart - startTime.QuadPart) * 1000 / frequency.QuadPart;

    CHAR oldImagePath[MAX_PATH];
    memcpy(oldImagePath, resultImagePath, MAX_PATH);

    sprintf_s(resultImagePath, "%s\\%s_%s_%d.bmp", RESULTS_SEQ_FOLDER, imageName, operationName, elapsedMilliseconds);
    CHECK(MoveFile(oldImagePath, resultImagePath), -1, "Error when naming the file");
    setFileReadPermissionOnly(resultImagePath);

    return 0;
}

DWORD applySequentialImageTransform(LPCSTR imagePath) {
    HANDLE hImage = CreateFile(imagePath, GENERIC_READ,
        NULL, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    CHECK(hImage != INVALID_HANDLE_VALUE, -1, "Opening an existing file failed");

    BITMAPFILEHEADER bMapFileHeader;
    BITMAPINFOHEADER bMapInfoHeader;

    extractBmpHeaders(hImage, &bMapFileHeader, &bMapInfoHeader);

    CHECK((CHAR)bMapFileHeader.bfType == 'B' && (CHAR)(bMapFileHeader.bfType >> 8) == 'M', -1, "The image type is not bitmap");
    CHECK(bMapInfoHeader.biBitCount == 32, -1, "The application supports only a count of 4 bytes per pixel");
    CHECK(bMapInfoHeader.biCompression == 0, -1, "The application supports only compression method None");
    CHECK(bMapFileHeader.bfOffBits < GetFileSize(hImage, NULL), -1, "The offset where the image data begins is invalid");

    printFileHeaderData(bMapFileHeader);
    printInfoHeaderData(bMapInfoHeader);

    CHAR imagePathCopy[MAX_PATH];
    memset(imagePathCopy, 0, sizeof(imagePathCopy));
    memcpy(imagePathCopy, imagePath, strlen(imagePath));

    LPSTR imageName = PathFindFileName(imagePathCopy);
    CHECK(imageName != imagePath, -1, "Could not extract the image name");
    PathRemoveExtension(imageName);
    CHECK(applyImageTransformation(hImage, imageName, SZ_GRAYSCALE_OPERATION, bMapFileHeader, bMapInfoHeader, applyPixelGrayscaleTransform) == 0, -1, "Grayscale Transformation Failed",
        CLOSE_HANDLES(hImage));

    CHECK(applyImageTransformation(hImage, imageName, SZ_INVERT_BYTE_OPERATION, bMapFileHeader, bMapInfoHeader, applyInvertBytesTransform) == 0, -1, "Grayscale Transformation Failed",
        CLOSE_HANDLES(hImage));
    CLOSE_HANDLES(hImage);
    return 0;
}

DWORD writeComputerCharacteristics(LPCSTR filePath) {
    std::string computerCharacteristics;
    computerCharacteristics.reserve(1000);
    CHECK(getStringCurrentUserSid(computerCharacteristics) == 0, -1, "Could not print the current user SID");
    CHECK(getStringWellKnownSid(WinWorldSid, "Everyone Group", computerCharacteristics) == 0, -1, "Could not print the everyone group SID");
    CHECK(getStringWellKnownSid(WinBuiltinAdministratorsSid, "Administrators Group", computerCharacteristics) == 0, -1, "Could not print the administrators group SID");
    CHECK(getStringProcessorMasksAndRelationships(computerCharacteristics) == 0, -1, "Could not print processor masks and relationships");
    CHECK(getStringAffinityMasksAndNumaNodesCounts(computerCharacteristics) == 0, -1, "Could not print affinity masks and numa nodes count");
    CHECK(getStringCpuSetsInformation(computerCharacteristics) == 0, -1, "Could not print CPU sets information");

    HANDLE hFile = CreateFile(filePath, GENERIC_WRITE, FILE_SHARE_READ, NULL, FILE_APPEND_DATA, FILE_ATTRIBUTE_NORMAL, 0);
    CHECK(hFile != INVALID_HANDLE_VALUE, -1, "Opening the file failed");

    LPCSTR cString = computerCharacteristics.c_str();
    CHECK(WriteFile(hFile, cString, strlen(cString), NULL, NULL), -1, "Error when writing in file", CLOSE_HANDLES(hFile));

    CLOSE_HANDLES(hFile);
}

int main() {
    HRESULT result = SHCreateDirectoryEx(NULL, RESULTS_SEQ_FOLDER, NULL);
    CHECK(result == ERROR_SUCCESS || result == ERROR_FILE_EXISTS || result == ERROR_ALREADY_EXISTS, 1, "Folder creation failed");

    result = SHCreateDirectoryEx(NULL, RESULTS_STATIC_FOLDER, NULL);
    CHECK(result == ERROR_SUCCESS || result == ERROR_FILE_EXISTS || result == ERROR_ALREADY_EXISTS, 1, "Folder creation failed");

    result = SHCreateDirectoryEx(NULL, RESULTS_DYNAMIC_FOLDER, NULL);
    CHECK(result == ERROR_SUCCESS || result == ERROR_FILE_EXISTS || result == ERROR_ALREADY_EXISTS, 1, "Folder creation failed");

    writeComputerCharacteristics(INFO_FILE_PATH);
    applySequentialImageTransform(IMAGE_PATH);
    return 0;
}