#include "image_utils.h"
#include <set>

DWORD extractBmpHeaders(HANDLE hImage, PBITMAPFILEHEADER bMapFileHeader, PBITMAPINFOHEADER bMapInfoHeader) {
    CHECK(ReadFile(hImage, bMapFileHeader, FILE_HEADER_SIZE, NULL, NULL), -1, "Reading the file header failed");
    CHECK(ReadFile(hImage, bMapInfoHeader, INFO_HEADER_SIZE, NULL, NULL), -1, "Reading the info header failed");
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

DWORD applyImageTransformation(DWORD nrCPU, HANDLE hImage, LPCSTR imageName, 
    BITMAPFILEHEADER& bMapFileHeader, BITMAPINFOHEADER& bMapInfoHeader,
    FileTransformFunction fileTransform, PixelTransformFunction pixelTransform, 
    LPCSTR resultFolder, LPCSTR operationName) {

    LARGE_INTEGER startTime, endTime;
    QueryPerformanceCounter(&startTime);

    CHECK(SetFilePointer(hImage, bMapFileHeader.bfOffBits, NULL, FILE_BEGIN) != INVALID_SET_FILE_POINTER, -1, "Setting the file pointer failed");

    CHAR resultImagePath[MAX_PATH];
    sprintf_s(resultImagePath, "%s\\%s_%s_temp.bmp", resultFolder, imageName, operationName);

    HANDLE hResult = CreateFile(resultImagePath, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    CHECK(hResult != INVALID_HANDLE_VALUE, -1, "Opening the results file failed");

    CHECK(appendBmapHeadersToFile(hResult, bMapFileHeader, bMapInfoHeader) == 0, -1, "Headers could not be appended",
        CLOSE_HANDLES(hResult));

    CHECK(fileTransform(hImage, hResult, pixelTransform, bMapFileHeader.bfOffBits, nrCPU) == 0, -1, "Transformation failed",
        CLOSE_HANDLES(hResult));

    CLOSE_HANDLES(hResult);

    QueryPerformanceCounter(&endTime);

    LARGE_INTEGER frequency;
    QueryPerformanceFrequency(&frequency);
    DWORD elapsedMilliseconds = (endTime.QuadPart - startTime.QuadPart) * 1000 / frequency.QuadPart;

    CHAR oldImagePath[MAX_PATH];
    memcpy(oldImagePath, resultImagePath, MAX_PATH);

    sprintf_s(resultImagePath, "%s\\%s_%s_%d.bmp", resultFolder, imageName, operationName, elapsedMilliseconds);
    CHECK(MoveFile(oldImagePath, resultImagePath), -1, "Error when naming the file");
    setFileReadPermissionOnly(resultImagePath);

    return 0;
}

DWORD applyImageTransformations(LPCSTR imagePath, DWORD totalNrCPU, const std::set<TransformationUtil>& transformationUtils, std::string& stringFileHeaderData, std::string& stringInfoHeaderData) {
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

    getStringFileHeaderData(bMapFileHeader, stringFileHeaderData);
    getStringInfoHeaderData(bMapInfoHeader, stringInfoHeaderData);

    CHAR imagePathCopy[MAX_PATH];
    memset(imagePathCopy, 0, sizeof(imagePathCopy));
    memcpy(imagePathCopy, imagePath, strlen(imagePath));

    LPSTR imageName = PathFindFileName(imagePathCopy);
    CHECK(imageName != imagePath, -1, "Could not extract the image name");
    PathRemoveExtension(imageName);


    for (auto& transformationUtil : transformationUtils) {
        CHECK(applyImageTransformation(totalNrCPU, hImage, imageName, bMapFileHeader, bMapInfoHeader, transformationUtil.fileTransformFunction, applyPixelGrayscaleTransform, transformationUtil.resultsFolder, SZ_GRAYSCALE_OPERATION) == 0,
            -1, "Grayscale Transformation Failed", CLOSE_HANDLES(hImage));

        CHECK(applyImageTransformation(totalNrCPU, hImage, imageName, bMapFileHeader, bMapInfoHeader, transformationUtil.fileTransformFunction, applyInvertBytesTransform, transformationUtil.resultsFolder, SZ_INVERT_BYTE_OPERATION) == 0,
            -1, "Grayscale Transformation Failed", CLOSE_HANDLES(hImage));
    }

    CLOSE_HANDLES(hImage);
    return 0;
}