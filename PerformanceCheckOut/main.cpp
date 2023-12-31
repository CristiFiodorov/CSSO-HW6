#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <Shlobj.h>

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

DWORD applyImageTransformation(HANDLE hImage, LPCSTR imageName, LPCSTR operationName, BITMAPFILEHEADER &bMapFileHeader, BITMAPINFOHEADER &bMapInfoHeader, 
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
			LPRGBA_PIXEL pixel = (LPRGBA_PIXEL) (buffer + i * sizeof(BYTE));
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
	CloseHandle(hImage);
	return 0;
}

int main() {
	HRESULT result = SHCreateDirectoryEx(NULL, RESULTS_SEQ_FOLDER, NULL);
	CHECK(result == ERROR_SUCCESS || result == ERROR_FILE_EXISTS || result == ERROR_ALREADY_EXISTS, 1, "Folder creation failed");

	result = SHCreateDirectoryEx(NULL, RESULTS_STATIC_FOLDER, NULL);
	CHECK(result == ERROR_SUCCESS || result == ERROR_FILE_EXISTS || result == ERROR_ALREADY_EXISTS, 1, "Folder creation failed");

	result = SHCreateDirectoryEx(NULL, RESULTS_DYNAMIC_FOLDER, NULL);
	CHECK(result == ERROR_SUCCESS || result == ERROR_FILE_EXISTS || result == ERROR_ALREADY_EXISTS, 1, "Folder creation failed");

	applySequentialImageTransform(IMAGE_PATH);
	return 0;
}