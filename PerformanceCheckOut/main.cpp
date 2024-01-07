#include "constants.h"
#include "static.h"
#include "dynamic.h"
#include "sequential.h"
#include "image_utils.h"
#include "characteristics_utils.h"

DWORD nrCPU = 1;

DWORD applyImageTransformations(LPCSTR imagePath) {
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

    // Sequential
    CHECK(applyImageTransformation(nrCPU, hImage, imageName, bMapFileHeader, bMapInfoHeader, fileTransformSequential, applyPixelGrayscaleTransform, RESULTS_SEQ_FOLDER, SZ_GRAYSCALE_OPERATION) == 0,
        -1, "Grayscale Transformation Failed", CLOSE_HANDLES(hImage));

    CHECK(applyImageTransformation(nrCPU, hImage, imageName, bMapFileHeader, bMapInfoHeader, fileTransformSequential, applyInvertBytesTransform, RESULTS_SEQ_FOLDER, SZ_INVERT_BYTE_OPERATION) == 0,
        -1, "Grayscale Transformation Failed", CLOSE_HANDLES(hImage));


    //// Parallel static
    CHECK(applyImageTransformation(nrCPU, hImage, imageName, bMapFileHeader, bMapInfoHeader, fileTransformParallelStatic, applyPixelGrayscaleTransform, RESULTS_STATIC_FOLDER, SZ_GRAYSCALE_OPERATION) == 0,
        -1, "Grayscale Transformation Failed", CLOSE_HANDLES(hImage));

    CHECK(applyImageTransformation(nrCPU, hImage, imageName, bMapFileHeader, bMapInfoHeader, fileTransformParallelStatic, applyInvertBytesTransform, RESULTS_STATIC_FOLDER, SZ_INVERT_BYTE_OPERATION) == 0,
        -1, "Grayscale Transformation Failed", CLOSE_HANDLES(hImage));

    // Parallel dynamic
    CHECK(applyImageTransformation(nrCPU, hImage, imageName, bMapFileHeader, bMapInfoHeader, fileTransformParallelDynamic, applyPixelGrayscaleTransform, RESULTS_DYNAMIC_FOLDER, SZ_GRAYSCALE_OPERATION) == 0,
        -1, "Grayscale Transformation Failed", CLOSE_HANDLES(hImage));

    CHECK(applyImageTransformation(nrCPU, hImage, imageName, bMapFileHeader, bMapInfoHeader, fileTransformParallelDynamic, applyInvertBytesTransform, RESULTS_DYNAMIC_FOLDER, SZ_INVERT_BYTE_OPERATION) == 0,
        -1, "Grayscale Transformation Failed", CLOSE_HANDLES(hImage));
        
    CLOSE_HANDLES(hImage);
    return 0;
}


int main() {
    HRESULT result = SHCreateDirectoryEx(NULL, RESULTS_SEQ_FOLDER, NULL);
    CHECK(result == ERROR_SUCCESS || result == ERROR_FILE_EXISTS || result == ERROR_ALREADY_EXISTS, 1, "Folder creation failed");

    std::string cpuInfo;
    getStringSystemCpuSetsInformation(cpuInfo, &nrCPU);

    result = SHCreateDirectoryEx(NULL, RESULTS_STATIC_FOLDER, NULL);
    CHECK(result == ERROR_SUCCESS || result == ERROR_FILE_EXISTS || result == ERROR_ALREADY_EXISTS, 1, "Folder creation failed");

    result = SHCreateDirectoryEx(NULL, RESULTS_DYNAMIC_FOLDER, NULL);
    CHECK(result == ERROR_SUCCESS || result == ERROR_FILE_EXISTS || result == ERROR_ALREADY_EXISTS, 1, "Folder creation failed");

    writeComputerCharacteristics(INFO_FILE_PATH);
    applyImageTransformations(IMAGE_PATH);
    return 0;
}