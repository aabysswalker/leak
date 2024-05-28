#include <windows.h>

LRESULT Win32WindowCallBack(HWND WindowHandle, UINT Message, WPARAM WParam, LPARAM LParam)
{
    LRESULT Result = {};

    switch (Message)
    {
    case WM_DESTROY:
    case WM_CLOSE:
    {
        DestroyWindow(WindowHandle);
    } break;

    default:
    {
        Result = DefWindowProcA(WindowHandle, Message, WParam, LParam);
    } break;
    }

    return Result;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) 
{
    WNDCLASSA WindowClass = {};
    WindowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    WindowClass.lpfnWndProc = Win32WindowCallBack;
    WindowClass.hInstance = hInstance;
    WindowClass.lpszClassName = "dxd";

    RegisterClassA(&WindowClass);

    HWND hWindow = CreateWindowExA(
        0,
        WindowClass.lpszClassName,
        "dxd",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        1280,
        720,
        NULL,
        NULL,
        hInstance,
        NULL);

    ShowWindow(hWindow, nCmdShow);

    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
