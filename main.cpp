#include <iostream>
#include <Windows.h>
#include <chrono>

LRESULT CALLBACK EventHandler(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

int deltaX = 0;
int deltaY = 0;

int main() {

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

    MSG event;
    bool quit = false;



    auto counter = std::chrono::system_clock::now();
    double timeDelay = 0.001; //s

    while (!quit) {
        while (PeekMessage(&event, 0, 0, 0, PM_REMOVE)) {
            if (event.message == WM_QUIT) {
                quit = true;
                break;
            }
            TranslateMessage(&event);
            DispatchMessage(&event);

        }
        auto now = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed = now - counter;
        double timeDiff = elapsed.count();


            if (timeDiff >= timeDelay) {
                std::cout << "Time: " << timeDiff << " DeltaX: " << deltaX << " DeltaY: " << deltaY << std::endl;
                deltaX = 0;
                deltaY = 0;
                counter = std::chrono::system_clock::now();
            }


    }

    return 0;
}

int lastMS = 0;

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
                auto currentTime = std::chrono::system_clock::now();
                auto durationSinceEpoch = currentTime.time_since_epoch();
                int msSinceEpoch = std::chrono::duration_cast<std::chrono::milliseconds>(durationSinceEpoch).count();
                int time = msSinceEpoch - lastMS;
//                std::cout << "Time:" << time << " Delta X: " << raw->data.mouse.lLastX << ", Delta Y: " << raw->data.mouse.lLastY
//                          << std::endl;
                deltaX += raw->data.mouse.lLastX;
                deltaY += raw->data.mouse.lLastY;
                lastMS = msSinceEpoch;
            }
        }
            return 0;
        default: {
            std::cout << "Any event" << std::endl;
        }
    }

    return DefWindowProc(hwnd, event, wparam, lparam);
}