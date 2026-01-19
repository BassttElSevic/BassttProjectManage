#include "Inc/task_manager.h"
#include <windows.h>
#include <shlwapi.h>

// Function pointer types for DPI awareness functions
typedef HRESULT(WINAPI *SetProcessDpiAwarenessFunc)(int);
typedef BOOL(WINAPI *SetProcessDPIAwareFunc)(void);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Enable high DPI support for clearer interface
    HMODULE hShcore = LoadLibrary(TEXT("shcore.dll"));
    if (hShcore) {
        SetProcessDpiAwarenessFunc setDpiAwareness = (SetProcessDpiAwarenessFunc)GetProcAddress(hShcore, "SetProcessDpiAwareness");
        if (setDpiAwareness) {
            setDpiAwareness(2); // Set to Per-Monitor DPI Awareness
        }
        FreeLibrary(hShcore);
    } else {
        // Fallback to SetProcessDPIAware for older systems - load dynamically
        HMODULE hUser32 = GetModuleHandle(TEXT("user32.dll"));
        if (hUser32) {
            SetProcessDPIAwareFunc setDPIAware = (SetProcessDPIAwareFunc)GetProcAddress(hUser32, "SetProcessDPIAware");
            if (setDPIAware) {
                setDPIAware();
            }
        }
    }

    const TCHAR CLASS_NAME[] = L"TaskManagerClass";

    // 从资源中加载嵌入的图标 - 这样图标就直接在exe文件中了
    HICON hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_MAIN_ICON));

    WNDCLASS wc = { };
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = CreateSolidBrush(COLOR_BG_MAIN);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    // 设置嵌入的图标，如果加载失败则使用默认图标
    wc.hIcon = hIcon ? hIcon : LoadIcon(NULL, IDI_APPLICATION);

    RegisterClass(&wc);

    // 居中显示窗口
    int screenW = GetSystemMetrics(SM_CXSCREEN);
    int screenH = GetSystemMetrics(SM_CYSCREEN);
    int winW = 1000;
    int winH = 700;
    int posX = (screenW - winW) / 2;
    int posY = (screenH - winH) / 2;

    HWND hwnd = CreateWindowEx(
        WS_EX_APPWINDOW,
        CLASS_NAME,
        L"👁️ 任务管理器 - Task Manager",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        posX, posY, winW, winH,
        NULL,
        NULL,
        GetModuleHandle(NULL),
        NULL
    );

    if (hwnd == NULL) {
        return 0;
    }

    // 设置窗口图标和任务栏图标
    if (hIcon) {
        SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
        SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
    }

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
