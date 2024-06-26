#include "constants.h"
#include "image_utils.h"
#include "characteristics_utils.h"
#include <set>

static LPCSTR szWindowClass = "RequestClientApp";
static LPCSTR szTitle = "RequestClientApp";

#define WINDOW_HEIGHT 800
#define WINDOW_WIDTH 1200

#define THIRD_WIDTH_PART WINDOW_WIDTH / 3 - 20

#define VERTICAL_LINE_WIDTH 5
#define CHECKBOX_COMPONENT_HEIGHT 120

#define RESULT_HEADER_AREA WINDOW_HEIGHT / 3 - 150
#define RESULT_TESTS_HEIGHT WINDOW_HEIGHT - RESULT_HEADER_AREA

#define HEADER_HEIGHT 35
#define OUTPUT_COMPONENT_HEIGHT 2 * HEADER_HEIGHT + 5


HINSTANCE hInst;
HWND openFileButton, filePathLabel, filePathInput, sequentialBox, dynamicBox, staticBox, testingMethodLabel, startButton, grayscaleLabel, grayscaleOutput, invertLabel, invertOutput, bitmapFileHeaderLabel, bitmapFileHeaderTextArea,
dibHeaderLabel, dibHeaderTextArea, testsPerformanceLabel, testsPerformanceTextArea, pcInfoTextArea, selectFolderButton, outputFolderBox;

std::set<TRANSFORMATION_UTIL> testingMethods{};
std::vector<TEST_RESULT> testResults{};


DWORD addCarriageReturnToBuffer(LPSTR* buffer) {
    DWORD newLineNo = 0, oldSize = 0;
    for (DWORD i = 0; (*buffer)[i] != '\0'; i++) {
        oldSize++;
        if ((*buffer)[i] == '\n') {
            newLineNo++;
        }
    }

    LPSTR replaced = new CHAR[oldSize + newLineNo + 2];

    DWORD p = 0;
    for (DWORD i = 0; i < oldSize; ++i) {
        if ((*buffer)[i] == '\n') {
            replaced[p++] = '\r';
            replaced[p++] = '\n';
        }
        else {
            replaced[p++] = (*buffer)[i];
        }
    }

    replaced[p++] = '\0';
    delete[](*buffer);
    *buffer = replaced;
    return 0;
}

DWORD printDataToComponent(std::string dataString, HWND componentHandler) {
    LPSTR textToPrint = new CHAR[dataString.size() + 1];
    memcpy(textToPrint, dataString.c_str(), dataString.size() + 1);
    addCarriageReturnToBuffer(&textToPrint);
    SetWindowText(componentHandler, textToPrint);
    delete[] textToPrint;
    return 0;
}


DWORD generateUIComponents(HWND &hWnd) {
    CreateWindow("Static", "Personal Computer Information", WS_VISIBLE | WS_CHILD | SS_CENTER,
        0, 0, THIRD_WIDTH_PART, HEADER_HEIGHT, hWnd, NULL, NULL, NULL);

    CreateWindow("Static", "", WS_CHILD | WS_VISIBLE | SS_ETCHEDHORZ,
        0, HEADER_HEIGHT, THIRD_WIDTH_PART, 2, hWnd, NULL, NULL, NULL);

    CreateWindow("Static", "", WS_CHILD | WS_VISIBLE | SS_ETCHEDVERT,
        THIRD_WIDTH_PART, 0, VERTICAL_LINE_WIDTH, WINDOW_HEIGHT, hWnd, NULL, NULL, NULL);

    pcInfoTextArea = CreateWindowEx(WS_EX_CLIENTEDGE, "Edit", "", WS_VISIBLE | WS_CHILD | WS_BORDER | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY,
        0, HEADER_HEIGHT + 5, THIRD_WIDTH_PART, WINDOW_HEIGHT - 80, hWnd, NULL, NULL, NULL);

    openFileButton = CreateWindow("Button", "Select BMP Image", WS_VISIBLE | WS_CHILD,
        THIRD_WIDTH_PART + VERTICAL_LINE_WIDTH, 0, THIRD_WIDTH_PART, HEADER_HEIGHT, hWnd, (HMENU)2, NULL, NULL);

    CreateWindow("Static", "", WS_CHILD | WS_VISIBLE | SS_ETCHEDVERT,
        THIRD_WIDTH_PART + 2 * VERTICAL_LINE_WIDTH + THIRD_WIDTH_PART, 0, VERTICAL_LINE_WIDTH, WINDOW_HEIGHT, hWnd, NULL, NULL, NULL);

    filePathLabel = CreateWindow("Static", "Selected File Path:", WS_VISIBLE | WS_CHILD,
        THIRD_WIDTH_PART + VERTICAL_LINE_WIDTH, HEADER_HEIGHT + 5, THIRD_WIDTH_PART, HEADER_HEIGHT, hWnd, NULL, NULL, NULL);

    filePathInput = CreateWindow("Edit", "", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL | ES_READONLY,
        THIRD_WIDTH_PART + VERTICAL_LINE_WIDTH, 2 * HEADER_HEIGHT + 10, THIRD_WIDTH_PART, HEADER_HEIGHT, hWnd, NULL, NULL, NULL);

    CreateWindow("Static", "", WS_CHILD | WS_VISIBLE | SS_ETCHEDHORZ,
        THIRD_WIDTH_PART, 3 * HEADER_HEIGHT + 15,
        THIRD_WIDTH_PART + 2 * VERTICAL_LINE_WIDTH, 10, hWnd, NULL, NULL, NULL);

    testingMethodLabel = CreateWindow("Static", "Please select the testing method:", WS_VISIBLE | WS_CHILD,
        THIRD_WIDTH_PART + VERTICAL_LINE_WIDTH, 3 * HEADER_HEIGHT + 25,
        THIRD_WIDTH_PART, HEADER_HEIGHT, hWnd, NULL, NULL, NULL);

    sequentialBox = CreateWindow("Button", "Sequential", WS_VISIBLE | WS_CHILD | BS_CHECKBOX,
        THIRD_WIDTH_PART + VERTICAL_LINE_WIDTH,
        4 * HEADER_HEIGHT + 30,
        THIRD_WIDTH_PART,
        HEADER_HEIGHT, hWnd, (HMENU)3, NULL, NULL);

    staticBox = CreateWindow("Button", "Parallel Static", WS_VISIBLE | WS_CHILD | BS_CHECKBOX,
        THIRD_WIDTH_PART + VERTICAL_LINE_WIDTH,
        4 * HEADER_HEIGHT + 60,
        THIRD_WIDTH_PART, HEADER_HEIGHT, hWnd, (HMENU)4, NULL, NULL);

    dynamicBox = CreateWindow("Button", "Parallel Dynamic", WS_VISIBLE | WS_CHILD | BS_CHECKBOX,
        THIRD_WIDTH_PART + VERTICAL_LINE_WIDTH,
        4 * HEADER_HEIGHT + 90,
        THIRD_WIDTH_PART, HEADER_HEIGHT, hWnd, (HMENU)5, NULL, NULL);

    selectFolderButton = CreateWindow("Button", "Select Folder", WS_VISIBLE | WS_CHILD,
        THIRD_WIDTH_PART + VERTICAL_LINE_WIDTH,
        4 * HEADER_HEIGHT + CHECKBOX_COMPONENT_HEIGHT + 15,
        THIRD_WIDTH_PART, HEADER_HEIGHT, hWnd, (HMENU)7, NULL, NULL);

    outputFolderBox = CreateWindow("Static", "", WS_VISIBLE | WS_CHILD | WS_BORDER,
        THIRD_WIDTH_PART + VERTICAL_LINE_WIDTH,
        5 * HEADER_HEIGHT + CHECKBOX_COMPONENT_HEIGHT + 30,
        THIRD_WIDTH_PART, HEADER_HEIGHT, hWnd, NULL, NULL, NULL);

    startButton = CreateWindow("Button", "Start Transformation", WS_VISIBLE | WS_CHILD,
        THIRD_WIDTH_PART + VERTICAL_LINE_WIDTH,
        6 * HEADER_HEIGHT + CHECKBOX_COMPONENT_HEIGHT + 45,
        THIRD_WIDTH_PART, HEADER_HEIGHT, hWnd, (HMENU)6, NULL, NULL);

    grayscaleLabel = CreateWindow("Static", "Grayscale Operation Output", WS_VISIBLE | WS_CHILD,
        THIRD_WIDTH_PART + VERTICAL_LINE_WIDTH,
        7 * HEADER_HEIGHT + CHECKBOX_COMPONENT_HEIGHT + 60,
        THIRD_WIDTH_PART, HEADER_HEIGHT, hWnd, NULL, NULL, NULL);

    grayscaleOutput = CreateWindow("Edit", "", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL | ES_AUTOVSCROLL | ES_READONLY | ES_MULTILINE,
        THIRD_WIDTH_PART + VERTICAL_LINE_WIDTH,
        8 * HEADER_HEIGHT + CHECKBOX_COMPONENT_HEIGHT + 75,
        THIRD_WIDTH_PART, HEADER_HEIGHT, hWnd, NULL, NULL, NULL);

    invertLabel = CreateWindow("Static", "Invert Bytes Operation Output", WS_VISIBLE | WS_CHILD,
        THIRD_WIDTH_PART + VERTICAL_LINE_WIDTH,
        7 * HEADER_HEIGHT + CHECKBOX_COMPONENT_HEIGHT + OUTPUT_COMPONENT_HEIGHT + 90,
        THIRD_WIDTH_PART, HEADER_HEIGHT, hWnd, NULL, NULL, NULL);

    invertOutput = CreateWindow("Edit", "", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL | ES_AUTOVSCROLL | ES_READONLY | ES_MULTILINE,
        THIRD_WIDTH_PART + VERTICAL_LINE_WIDTH,
        8 * HEADER_HEIGHT + CHECKBOX_COMPONENT_HEIGHT + OUTPUT_COMPONENT_HEIGHT + 105,
        THIRD_WIDTH_PART, HEADER_HEIGHT, hWnd, NULL, NULL, NULL);

    bitmapFileHeaderLabel = CreateWindow("Static", "Bitmap File Header", WS_VISIBLE | WS_CHILD | SS_CENTER,
        2 * THIRD_WIDTH_PART + 2 * VERTICAL_LINE_WIDTH, 0, THIRD_WIDTH_PART, HEADER_HEIGHT, hWnd, NULL, NULL, NULL);

    CreateWindow("Static", "", WS_CHILD | WS_VISIBLE | SS_ETCHEDHORZ,
        2 * THIRD_WIDTH_PART + 2 * VERTICAL_LINE_WIDTH, HEADER_HEIGHT, THIRD_WIDTH_PART, 2, hWnd, NULL, NULL, NULL);

    bitmapFileHeaderTextArea = CreateWindowEx(WS_EX_CLIENTEDGE, "Edit", "", WS_VISIBLE | WS_CHILD | WS_BORDER | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY,
        2 * THIRD_WIDTH_PART + 2 * VERTICAL_LINE_WIDTH, HEADER_HEIGHT + 5, THIRD_WIDTH_PART, RESULT_HEADER_AREA, hWnd, NULL, NULL, NULL);

    dibHeaderLabel = CreateWindow("Static", "Dib Header", WS_VISIBLE | WS_CHILD | SS_CENTER,
        2 * THIRD_WIDTH_PART + 2 * VERTICAL_LINE_WIDTH, HEADER_HEIGHT + RESULT_HEADER_AREA + 10, THIRD_WIDTH_PART, HEADER_HEIGHT, hWnd, NULL, NULL, NULL);

    dibHeaderTextArea = CreateWindowEx(WS_EX_CLIENTEDGE, "Edit", "", WS_VISIBLE | WS_CHILD | WS_BORDER | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY,
        2 * THIRD_WIDTH_PART + 2 * VERTICAL_LINE_WIDTH, 2 * HEADER_HEIGHT + RESULT_HEADER_AREA + 15, THIRD_WIDTH_PART, RESULT_HEADER_AREA, hWnd, NULL, NULL, NULL);

    testsPerformanceLabel = CreateWindow("Static", "Tests Performance", WS_VISIBLE | WS_CHILD | SS_CENTER,
        2 * THIRD_WIDTH_PART + 2 * VERTICAL_LINE_WIDTH, 2 * (HEADER_HEIGHT + RESULT_HEADER_AREA) + 25, THIRD_WIDTH_PART, HEADER_HEIGHT, hWnd, NULL, NULL, NULL);

    testsPerformanceTextArea = CreateWindowEx(WS_EX_CLIENTEDGE, "Edit", "", WS_VISIBLE | WS_CHILD | WS_BORDER | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY,
        2 * THIRD_WIDTH_PART + 2 * VERTICAL_LINE_WIDTH, 2 * (HEADER_HEIGHT + RESULT_HEADER_AREA) + 30 + HEADER_HEIGHT, THIRD_WIDTH_PART, RESULT_TESTS_HEIGHT, hWnd, NULL, NULL, NULL);

    return 0;
}

DWORD insertCSVToFile(LPCSTR filePath, std::string csvResults) {
    HANDLE hFile = CreateFile(filePath, GENERIC_WRITE, NULL, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
    CHECK(hFile != INVALID_HANDLE_VALUE, -2, "Opening the file failed");

    CHECK(WriteFile(hFile, csvResults.c_str(), csvResults.size(), NULL, NULL), -2, "Error when writing in file", 
        CLOSE_HANDLES(hFile));

    CLOSE_HANDLES(hFile);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE: {
        generateUIComponents(hWnd);
        std::string output;
        CHECK_GUI(writeComputerCharacteristics(INFO_FILE_PATH, output) != -1, -1, "Failed to write Computer Characteristics");
        printDataToComponent(output, pcInfoTextArea);
        break;
    }
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case 2:
            OPENFILENAME ofn;
            memset(&ofn, 0, sizeof(ofn));

            CHAR szFile[MAX_PATH_LEN];
            memset(szFile, 0, sizeof(szFile));

            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = hWnd;
            ofn.lpstrFile = szFile;
            ofn.nMaxFile = sizeof(szFile);
            ofn.lpstrFilter = "BMP Files (*.bmp)\0*.bmp\0All Files (*.*)\0*.*\0";
            ofn.nFilterIndex = 1;

            CHECK_GUI(GetOpenFileName(&ofn), -1, "Failure when selecting a .bmp image");
            SetWindowText(filePathInput, szFile);

            break;
        case 3:
            SendMessage(sequentialBox, BM_SETCHECK, !SendMessage(sequentialBox, BM_GETCHECK, 0, 0), 0);
            if (SendMessage(sequentialBox, BM_GETCHECK, 0, 0)) {
                testingMethods.insert({ SZ_SEQ_METHOD, fileTransformSequential, RESULTS_SEQ_FOLDER, FALSE });
            }
            else {
                testingMethods.erase({ SZ_SEQ_METHOD, fileTransformSequential, RESULTS_SEQ_FOLDER, FALSE });
            }
        break;
        case 4:
            SendMessage(staticBox, BM_SETCHECK, !SendMessage(staticBox, BM_GETCHECK, 0, 0), 0);
            if (SendMessage(staticBox, BM_GETCHECK, 0, 0)) {
                testingMethods.insert({ SZ_STATIC_METHOD, fileTransformParallelStatic, RESULTS_STATIC_FOLDER, TRUE });
            }
            else {
                testingMethods.erase({ SZ_STATIC_METHOD, fileTransformParallelStatic, RESULTS_STATIC_FOLDER, TRUE });
            }
            break;
        case 5:
            SendMessage(dynamicBox, BM_SETCHECK, !SendMessage(dynamicBox, BM_GETCHECK, 0, 0), 0);
            if (SendMessage(dynamicBox, BM_GETCHECK, 0, 0)) {
                testingMethods.insert({ SZ_DYNAMIC_METHOD, fileTransformParallelDynamic, RESULTS_DYNAMIC_FOLDER, TRUE });
            }
            else {
                testingMethods.erase({ SZ_DYNAMIC_METHOD, fileTransformParallelDynamic, RESULTS_DYNAMIC_FOLDER, TRUE });
            }
            break;
        case 6: {
            if (testingMethods.size() < 1) {
                MessageBox(NULL, "You have to select a testing method!", "Error", NULL);
                break;
            }

            CHAR imagePath[MAX_PATH_LEN];
            memset(imagePath, 0, sizeof(imagePath));

            GetWindowText(filePathInput, imagePath, MAX_PATH_LEN);
            if (strlen(imagePath) < 1) {
                MessageBox(NULL, "Invalid file path!", "Error", NULL);
                break;
            }

            std::string output;
            DWORD nrCPU = 1;
            getStringSystemCpuSetsInformation(output, &nrCPU);

            std::string stringFileHeaderData;
            std::string stringInfoHeaderData;
            std::string grayscaleOutputPath;
            std::string invertOutputPath;

            testResults.clear();

            CHAR outputFolder[MAX_PATH_LEN];
            memset(outputFolder, 0, sizeof(outputFolder));

            GetWindowText(outputFolderBox, outputFolder, MAX_PATH_LEN);
            if (strlen(outputFolder) < 1) {
                MessageBox(NULL, "Invalid file path!", "Error", NULL);
                break;
            }

            applyImageTransformations(imagePath, nrCPU, outputFolder, testingMethods, { stringFileHeaderData, stringInfoHeaderData, grayscaleOutputPath, invertOutputPath, testResults });

            std::string stringTestResults = getStringFromTestResults(testResults);
            std::string csvResults = getCSVContentFromTestsResults(testResults);
            insertCSVToFile(CSV_TABLE_PATH, csvResults);

            stringTestResults += std::format("The results in a CSV format:\r\n\r\n{}\r\n\r\n", csvResults);

            SetWindowText(testsPerformanceTextArea, stringTestResults.c_str());

            printDataToComponent(stringFileHeaderData, bitmapFileHeaderTextArea);
            printDataToComponent(stringInfoHeaderData, dibHeaderTextArea);
            printDataToComponent(grayscaleOutputPath, grayscaleOutput);
            printDataToComponent(invertOutputPath, invertOutput);
            
            break;
        }
        case 7: 
        {
            BROWSEINFO browseInfo = { 0 };
            browseInfo.hwndOwner = hWnd;
            browseInfo.ulFlags = BIF_RETURNONLYFSDIRS;
            LPITEMIDLIST pidl = SHBrowseForFolder(&browseInfo);
            CHECK_GUI(pidl, -1, "Could not extract folder");

            CHAR folderPath[MAX_PATH];
            SHGetPathFromIDList(pidl, folderPath);

            SetWindowText(outputFolderBox, folderPath);

            CoTaskMemFree(pidl);
            break;
        }
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