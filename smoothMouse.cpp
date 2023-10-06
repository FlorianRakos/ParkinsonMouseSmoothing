#include <iostream>
#include <Windows.h>
#include <chrono>
#include <string>
#include <stdlib.h>
#include <fstream>
#include <filesystem>
#include <vector>
#include <cmath>

using namespace std;
namespace fs = filesystem;


LRESULT CALLBACK EventHandler(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);




size_t WriteCallback(void* contents, size_t size, size_t nmemb, string* output) {
    size_t totalSize = size * nmemb;
    output->append(static_cast<char*>(contents), totalSize);
    return totalSize;
}


bool isKeyPressed(int keyCode) {
    return GetAsyncKeyState(keyCode) & 0x8000;
}

int main() {
    cout << "Program start" << endl;

    vector<pair<int,int>> testVec;
    testVec.push_back(make_pair(1,2));
    cout << testVec.size() << endl;



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
        cerr << "Failed to register for raw input -> Error: " << er << endl;
        return 1;
    }

    MSG event;
    bool quit = false;



    cout << "Apply Smoothing... Quit with Q" << endl;
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
            cout << "Key with code " << 81 << " is pressed. Exiting..." << endl;
            return 0; // Exit the program
        }


    }

    return 0;
}


vector<pair<int, int>> mouseMovements;
int lengthBuffer = 1;
POINT curPos;
int lastMS = 0;
int lastXPos = 0;
int lastYPos = 0;

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
            //cout << "Input Event" << endl;
            unsigned size = sizeof(RAWINPUT);
            static RAWINPUT raw[sizeof(RAWINPUT)];
            GetRawInputData((HRAWINPUT) lparam, RID_INPUT, raw, &size, sizeof(RAWINPUTHEADER));


            if (raw->header.dwType == RIM_TYPEMOUSE) {

                int deltaXRaw = raw->data.mouse.lLastX;
                int deltaYRaw = raw->data.mouse.lLastY;

                mouseMovements.emplace_back(deltaXRaw, deltaYRaw);


                if (mouseMovements.size() > lengthBuffer) {
                    mouseMovements.erase(mouseMovements.begin());
                }


                float weightedSumX;
                float weightedSumY;
                int movementsLength = mouseMovements.size();
                int weightAmountLength = movementsLength * (movementsLength + 1) / 2; // e.a. 100+99+98...+1

                GetCursorPos(&curPos);

                for (int i = 0; i < movementsLength; i++) {
                    mouseMovements[i];
                    weightedSumX += mouseMovements[i].first * (i + 1);// / weightAmountLength;
                    weightedSumY += mouseMovements[i].second * (i + 1) ;// / weightAmountLength;

                }

                float deltaX = weightedSumX / weightAmountLength;
                float deltaY = weightedSumY / weightAmountLength;

                float speed = 0.9f;
                float exponent = 0.7f;
                float distance = (abs(deltaX) + abs(deltaY)) / 2.0f;
                float xTransformed = deltaX * pow(exponent, distance ) * speed;
                float yTransformed = deltaY * pow(exponent, distance) * speed;

                int nextX = lastXPos + round(xTransformed); //movementsLength ;
                int nextY = lastYPos + round(yTransformed); //movementsLength;


                SetCursorPos(nextX, nextY);

                GetCursorPos(&curPos);
                lastXPos = curPos.x;
                lastYPos = curPos.y;

            }
        }
            return 0;
        default: {
            //cout << "Any event" << endl;
        }
    }

    return DefWindowProc(hwnd, event, wparam, lparam);
}


