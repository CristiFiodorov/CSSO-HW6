#pragma once
#include <stdio.h>

#define CHECK(condition, ret, errorText, ...){												    \
	if (!(condition)){																		    \
		printf("Error at line %d: %s; Error code: %d \n", __LINE__, errorText, GetLastError());	\
        __VA_ARGS__;																		    \
        return (ret);																		    \
	}																						    \
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

using PixelTransformFunction = DWORD(*)(LPRGBA_PIXEL);
using FileTransformFunction = DWORD(*)(HANDLE, HANDLE, PixelTransformFunction);

typedef struct ThreadParams {
    HANDLE hImage;
    HANDLE hResult;
    DWORD64 startOffset;
    DWORD64 partSize;
    PixelTransformFunction pixelTransform;
    CRITICAL_SECTION* writeCriticalSection;
    CRITICAL_SECTION* readCriticalSection;
} THREAD_PARAMS, *LPTHREAD_PARAMS;


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

#define SZ_GRAYSCALE_OPERATION "grayscale"
#define SZ_INVERT_BYTE_OPERATION "invert_bytes"