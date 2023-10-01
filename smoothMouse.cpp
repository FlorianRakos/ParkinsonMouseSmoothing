#include <iostream>
#include <Windows.h>
#include <chrono>
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include <string>
#include <stdlib.h>
#include <fstream>
#include <filesystem>
#include <vector>

namespace fs = std::filesystem;

LRESULT CALLBACK EventHandler(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

int deltaX = 0;
int deltaY = 0;

std::vector<std::pair<int, int>> mouseMovements;
int lengthBuffer = 10000;

POINT curPos;

size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
    size_t totalSize = size * nmemb;
    output->append(static_cast<char*>(contents), totalSize);
    return totalSize;
}


bool isKeyPressed(int keyCode) {
    return GetAsyncKeyState(keyCode) & 0x8000;
}

int main() {
    std::cout << "Program start" << std::endl;

    std::vector<std::pair<int,int>> testVec;
    testVec.push_back(std::make_pair(1,2));
    std::cout << testVec.size() << std::endl;



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

    // Register Raw input device
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



    std::cout << "Apply Smoothing... Quit with Q" << std::endl;
    while (!quit) {
        while (PeekMessage(&event, 0, 0, 0, PM_REMOVE)) {
            if (event.message == WM_QUIT) {
                quit = true;
                break;
            }
            TranslateMessage(&event);
            DispatchMessage(&event);
        }

        // Keycode 81 = Q
        if (isKeyPressed(81)) {
            std::cout << "Key with code " << 81 << " is pressed. Exiting..." << std::endl;
            return 0; // Exit the program
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

                int deltaX = raw->data.mouse.lLastX;
                int deltaY = raw->data.mouse.lLastY;

                mouseMovements.push_back(std::make_pair(deltaX, deltaY));

                if (mouseMovements.size() > lengthBuffer) {
                    mouseMovements.erase(mouseMovements.begin());
                }
                float weightedSumX;
                float weightedSumY;
                int movementsLength = mouseMovements.size();
                int weightAmountLength = movementsLength * (movementsLength + 1) / 2; // e.a. 100+99+98...+1

                //std::cout <<"movementsLength: " << movementsLength << std::endl;
                for (int i = 0; i < movementsLength; i++) {
                    mouseMovements[i];
                    weightedSumX += mouseMovements[i].first * i / weightAmountLength;
                    weightedSumY += mouseMovements[i].second * i / weightAmountLength;
                }

                GetCursorPos(&curPos);
                //std::cout << curPos.x << " " << curPos.y << std::endl;

                int nextX = curPos.x + weightedSumX ;
                int nextY = curPos.y + weightedSumY;

                SetCursorPos(nextX, nextY);

            }
        }
            return 0;
        default: {
            //std::cout << "Any event" << std::endl;
        }
    }

    return DefWindowProc(hwnd, event, wparam, lparam);
}


