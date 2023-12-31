#include <windows.h>
#include <string.h>
#include <stdio.h>

#define CHECK(condition, message, ...)										    \
	if(!(condition))														    \
	{																		    \
		printf("Error Message: %s\nError Code: %d\n", message, GetLastError()); \
		__VA_ARGS__;														    \
		return FALSE;															\
	}	


LPCSTR imagePath = "C:\\Facultate\\CSSO\\Week6\\date\\forest.bmp";
const DWORD CHUNK_SIZE = 0x4000;

DWORD readBmpImage(LPCSTR imagePath) {
    HANDLE imageHandle = CreateFile(imagePath, GENERIC_READ,
		NULL, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	CHECK(imageHandle != INVALID_HANDLE_VALUE, "Opening an existing file failed");

    while (true) {
		CHAR buffer[CHUNK_SIZE + 1];
		DWORD bytesRead;
		memset(buffer, 0, sizeof(buffer));
		CHECK(ReadFile(imageHandle, buffer, CHUNK_SIZE, &bytesRead, NULL),
			"Reading from a file failed",
			CloseHandle(imageHandle));
		if (!bytesRead) {
			break;
		}
		printf("%s", buffer);
	}

	return TRUE;
}

int main() {
	readBmpImage(imagePath);
}