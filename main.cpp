#include <iostream>
#include <Windows.h>

LRESULT CALLBACK EventHandler(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

int main() {
    std::cout << "Code runs" << std::endl;

    HINSTANCE hInstance = GetModuleHandle(NULL);

    const char *class_name = "SimpleEngine Class";
    WNDCLASS wc = {0};
    wc.lpfnWndProc = EventHandler; // Set the custom window procedure
    wc.hInstance = hInstance; // Your application instance handle
    wc.lpszClassName = class_name; // Your window class name
    RegisterClass(&wc);

    // Create a hidden window
    HWND hwnd = CreateWindow(
            class_name, // Window class name
            "My Window",     // Window title
            0,                // Window style (0 for hidden)
            0, 0, 0, 0,       // Window position and size (hidden)
            NULL, NULL, hInstance, NULL);


    RAWINPUTDEVICE rid;
    rid.usUsagePage = 0x01;
    rid.usUsage = 0x02;
    rid.dwFlags = RIDEV_INPUTSINK;
    rid.hwndTarget = hwnd; // Specify the window handle for raw input

    if (RegisterRawInputDevices(&rid, 1, sizeof(rid)) == FALSE) {
        DWORD er = GetLastError();
        std::cerr << "Failed to register for raw input -> Error: " << er << std::endl;
        return 1;
    }
    std::cout << "Code runs3" << std::endl;

    MSG event;
    bool quit = false;

    while (!quit) {
        while (PeekMessage(&event, 0, 0, 0, PM_REMOVE)) {
//            if (event.message == WM_QUIT) {
//                quit = true;
//                break;
//            }
            TranslateMessage(&event);
            DispatchMessage(&event);
            //std::cout << "Event: " << event.message << std::endl;
        }


    }

    return 0;
}

LRESULT CALLBACK EventHandler(
        HWND hwnd,
        UINT event,
        WPARAM wparam,
        LPARAM lparam
) {
    switch (event) {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        case WM_INPUT: {
            //std::cout << "Input Event" << std::endl;
            unsigned size = sizeof(RAWINPUT);
            static RAWINPUT raw[sizeof(RAWINPUT)];
            GetRawInputData((HRAWINPUT) lparam, RID_INPUT, raw, &size, sizeof(RAWINPUTHEADER));


            if (raw->header.dwType == RIM_TYPEMOUSE) {
                std::cout << "Delta X: " << raw->data.mouse.lLastX << ", Delta Y: " << raw->data.mouse.lLastY
                          << std::endl;
            }
        }
            return 0;
        default: {
            std::cout << "Any event" << std::endl;
        }
    }

    return DefWindowProc(hwnd, event, wparam, lparam);
}