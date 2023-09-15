#include <iostream>
#include <Windows.h>
#include <chrono>
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include <string>
#include <stdlib.h>

//mousetrap

LRESULT CALLBACK EventHandler(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

int deltaX = 0;
int deltaY = 0;
rapidjson::Document document;
rapidjson::Value dataArray(rapidjson::kArrayType);


void saveJSON () {
    std::cout << "-------Save JSON-------" << std::endl;
    // Add the dataArray to the document
    document.AddMember("data", dataArray, document.GetAllocator());

    // Serialize the document to a JSON string
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    document.Accept(writer);

    // Print the JSON string
    std::string jsonString = buffer.GetString();
    std::cout << "JSON: " << jsonString << std::endl;
}

void processJSON (double time, int deltaX, int deltaY) {

    // Create a data object
    rapidjson::Value dataObject(rapidjson::kObjectType);

    // Add timestamp, deltaX, and deltaY to the data object
    dataObject.AddMember("timestamp", time, document.GetAllocator());
    dataObject.AddMember("deltaX", deltaX, document.GetAllocator());
    dataObject.AddMember("deltaY", deltaY, document.GetAllocator());

    // Add the data object to the dataArray
    dataArray.PushBack(dataObject, document.GetAllocator());

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    dataArray.Accept(writer);

    // Print the JSON string
    std::string jsonString = buffer.GetString();
    //std::cout << "JSON: " << jsonString << std::endl;

 // print json
    //saveJSON(dataArray);

}

bool isKeyPressed(int keyCode) {
    return GetAsyncKeyState(keyCode) & 0x8000;
}

int main() {

    std::atexit(saveJSON);

    //Prepare json
    document.SetObject();

    // Add the sessionID as a string
    rapidjson::Value sessionID;
    sessionID.SetString("1", document.GetAllocator());
    document.AddMember("sessionID", sessionID, document.GetAllocator());



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
    double timeDelay = 0.005; //s


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
        auto durationSinceEpoch = now.time_since_epoch();
        long long msSinceEpoch = std::chrono::duration_cast<std::chrono::milliseconds>(durationSinceEpoch).count();

        std::chrono::duration<double> elapsed = now - counter;
        double timeDiff = elapsed.count();


        if (timeDiff >= timeDelay) {
            //std::cout << "Time: " << timeDiff << " DeltaX: " << deltaX << " DeltaY: " << deltaY << std::endl;

            //reset counter
            counter = std::chrono::system_clock::now();
            processJSON(msSinceEpoch, deltaX, deltaY);
            deltaX = 0;
            deltaY = 0;
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

