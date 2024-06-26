#pragma once
#include <windows.h>
#include <windowsx.h>
#include <string.h>
#include <stdio.h>
#include <Shlobj.h>
#include <Aclapi.h>
#include <sddl.h>
#include <winnt.h>
#include <string>
#include <format>
#include <cmath>
#include <set>
#include <vector>


#pragma comment(lib, "Shlwapi.lib")
#include <shlwapi.h>

#define CHECK(condition, ret, errorText, ...){												    \
	if (!(condition)){																		    \
		printf("Error at line %d: %s; Error code: %d \n", __LINE__, errorText, GetLastError());	\
        __VA_ARGS__;																		    \
        return (ret);																		    \
	}																						    \
}

#define CHECK_GUI(condition, ret, errorText, ...) { \
    if (!(condition)){								\
		MessageBox(NULL, errorText, "Error", NULL); \
        __VA_ARGS__;								\
        return (ret);							    \
	}											    \
}

#define CLOSE_HANDLES(...){                                                         \
        HANDLE handles[] = { __VA_ARGS__ };                                         \
        int count = sizeof(handles) / sizeof(handles[0]);                           \
        for (int __i__ = 0; __i__ < count; ++__i__) {                               \
            if (handles[__i__] != NULL && handles[__i__] != INVALID_HANDLE_VALUE) { \
                CloseHandle(handles[__i__]);                                        \
                handles[__i__] = NULL;                                              \
            }                                                                       \
        }                                                                           \
}

#define CRITICAL_SECTION_OPERATION(criticalSection, ...){   \
    EnterCriticalSection(criticalSection);                  \
    __VA_ARGS__;                                            \
    LeaveCriticalSection(criticalSection);                  \
}

typedef BOOL(WINAPI* LPFN_GLPI)(
    PSYSTEM_LOGICAL_PROCESSOR_INFORMATION,
    PDWORD);


typedef struct RgbaPixel {
    BYTE red;
    BYTE green;
    BYTE blue;
    BYTE alpha;
} RGBA_PIXEL, *LPRGBA_PIXEL;


typedef struct TestResult {
    LPCSTR testingMethod;
    DWORD nrWorkers;
    DWORD elapsedMilliseconds;
    LPCSTR OperationName;
} TEST_RESULT, *LPTEST_RESULT;

using PixelTransformFunction = DWORD(*)(LPRGBA_PIXEL);
using FileTransformFunction = DWORD(*)(HANDLE, HANDLE, PixelTransformFunction, DWORD, DWORD);

typedef struct TransformationUtil {
    LPCSTR transformationName;
    FileTransformFunction fileTransformFunction;
    LPCSTR resultsFolder;
    BOOL iterate;

    bool operator<(const TransformationUtil& other) const {
        if (strcmp(transformationName, other.transformationName) < 0) {
            return true;
        }

        return false;
    }

} TRANSFORMATION_UTIL, *LPTRANSFORMATION_UTIL;

enum RequestStatus {
    PENDING, 
    READY, 
    FINISHED
};

typedef struct DynamicThreadRequest {
    DWORD64 startOffset;
    DWORD64 partSize;
    RequestStatus requestStatus;
} DYNAMIC_THREAD_REQUEST, *LPDYNAMIC_THREAD_REQUEST;

typedef struct StaticThreadParams {
    HANDLE hImage;
    HANDLE hResult;
    DWORD64 startOffset;
    DWORD64 partSize;
    PixelTransformFunction pixelTransform;
    LPCRITICAL_SECTION writeCriticalSection;
    LPCRITICAL_SECTION readCriticalSection;
} STATIC_THREAD_PARAMS, *LPSTATIC_THREAD_PARAMS;


typedef struct DynamicThreadParams {
    HANDLE hImage;
    HANDLE hResult;
    PixelTransformFunction pixelTransform;
    LPCRITICAL_SECTION writeCriticalSection;
    LPCRITICAL_SECTION readCriticalSection;
    LPCRITICAL_SECTION mapCriticalSection;
    LPDYNAMIC_THREAD_REQUEST threadRequest;
} DYNAMIC_THREAD_PARAMS, * LPDYNAMIC_THREAD_PARAMS;


typedef struct ImageTransformationsResults {
    std::string& stringFileHeaderData;
    std::string& stringInfoHeaderData;
    std::string& grayscaleOutputPath; 
    std::string& invertOutputPath; 
    std::vector<TEST_RESULT>& testResults;
} IMAGE_TRANSFORMATION_RESULTS, *LPIMAGE_TRANSFORMATION_RESULTS;

typedef struct BmpImageInfo {
    HANDLE hImage; 
    LPCSTR imageName;
    BITMAPFILEHEADER bMapFileHeader; 
    BITMAPINFOHEADER bMapInfoHeader;
} BMP_IMAGE_INFO, *LPBMP_IMAGE_INFO;

typedef struct TransformationInfo {
    DWORD nrCPU;
    LPCSTR operationName;
    FileTransformFunction fileTransform; 
    PixelTransformFunction pixelTransform; 
} TRANSFORMATION_INFO, *LPTRANSFORMATION_INFO;


#define IMAGE_PATH "C:\\Facultate\\CSSO\\Week6\\date\\forest.bmp"
#define CHUNK_SIZE 0x4000
#define FILE_HEADER_SIZE 14
#define INFO_HEADER_SIZE 40

#define INFO_FILE_PATH "C:\\Facultate\\CSSO\\Week6\\info.txt"

#define RESULTS_SEQ_FOLDER "C:\\Facultate\\CSSO\\Week6\\rezultate\\secvential"
#define RESULTS_STATIC_FOLDER "C:\\Facultate\\CSSO\\Week6\\rezultate\\static"
#define RESULTS_DYNAMIC_FOLDER "C:\\Facultate\\CSSO\\Week6\\rezultate\\dinamic"

#define GRAYSCALE_RED_COEFF 0.299
#define GRAYSCALE_GREEN_COEFF 0.587
#define GRAYSCALE_BLUE_COEFF 0.114

#define INVERT_BYTE_CONSTANT 0xFF
#define MAX_PATH_LEN 260 

#define SZ_GRAYSCALE_OPERATION "grayscale"
#define SZ_INVERT_BYTE_OPERATION "invert_bytes"
#define WORKER_MAPPING "worker_mapping"

#define SZ_SEQ_METHOD "Sequential"
#define SZ_STATIC_METHOD "Parallel Static"
#define SZ_DYNAMIC_METHOD "Parallel Dynamic"

#define CSV_TABLE_PATH "C:\\Facultate\\CSSO\\Week6\\comp.txt"
