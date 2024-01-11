#include "constants.h"
#include "image_utils.h"
#include "characteristics_utils.h"

static LPCSTR szWindowClass = "RequestClientApp";
static LPCSTR szTitle = "RequestClientApp";

static const int WINDOW_HEIGHT = 800;
static const int WINDOW_WIDTH = 1200;

static const int PC_INFO_WIDTH = WINDOW_WIDTH / 3;
static const int PC_INFO_HEIGHT = 35;

DWORD nrCPU = 1;
HINSTANCE hInst;
HWND submitButton, baseUrlInput, universityIdInput, hEdit;


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
        CreateWindow("Static", "Personal Computer Information", WS_VISIBLE | WS_CHILD | SS_CENTER,
            0, 0, PC_INFO_WIDTH, PC_INFO_HEIGHT, hWnd, NULL, NULL, NULL);

        CreateWindow("Static", "", WS_CHILD | WS_VISIBLE | SS_ETCHEDHORZ,
            0, PC_INFO_HEIGHT, PC_INFO_WIDTH, 2, hWnd, NULL, NULL, NULL);

        CreateWindow("Static", "", WS_CHILD | WS_VISIBLE | SS_ETCHEDVERT,
            PC_INFO_WIDTH, 0, 2, WINDOW_HEIGHT, hWnd, NULL, NULL, NULL);

        break;
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case 1:
            DWORD exitCode;
            SetWindowText(hEdit, "The output will be displayed here..");
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
        break;
    }

    return 0;
}

int WINAPI WinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPSTR     lpCmdLine,
    _In_ int       nCmdShow
)
{

    HRESULT result = SHCreateDirectoryEx(NULL, RESULTS_SEQ_FOLDER, NULL);
    CHECK(result == ERROR_SUCCESS || result == ERROR_FILE_EXISTS || result == ERROR_ALREADY_EXISTS, 1, "Folder creation failed");

    std::string cpuInfo;
    getStringSystemCpuSetsInformation(cpuInfo, &nrCPU);

    result = SHCreateDirectoryEx(NULL, RESULTS_STATIC_FOLDER, NULL);
    CHECK(result == ERROR_SUCCESS || result == ERROR_FILE_EXISTS || result == ERROR_ALREADY_EXISTS, 1, "Folder creation failed");

    result = SHCreateDirectoryEx(NULL, RESULTS_DYNAMIC_FOLDER, NULL);
    CHECK(result == ERROR_SUCCESS || result == ERROR_FILE_EXISTS || result == ERROR_ALREADY_EXISTS, 1, "Folder creation failed");

    WNDCLASSEX wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(wcex.hInstance, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, IDI_APPLICATION);

    if (!RegisterClassEx(&wcex))
    {
        MessageBox(NULL,
            "Call to RegisterClassEx failed!",
            "Windows Desktop Guided Tour",
            NULL);

        return 1;
    }

    hInst = hInstance;

    HWND hWnd = CreateWindowEx(
        WS_EX_OVERLAPPEDWINDOW,
        szWindowClass,
        szTitle,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    if (!hWnd)
    {
        MessageBox(NULL,
            "Call to CreateWindow failed!",
            "Windows Desktop Guided Tour",
            NULL);

        return 1;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}


//int main() {
//    writeComputerCharacteristics(INFO_FILE_PATH);
//    applyImageTransformations(IMAGE_PATH);
//    return 0;
//}