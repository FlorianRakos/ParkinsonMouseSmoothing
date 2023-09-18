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

namespace fs = std::filesystem;

LRESULT CALLBACK EventHandler(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

int deltaX = 0;
int deltaY = 0;
rapidjson::Document document;
rapidjson::Value dataArray(rapidjson::kArrayType);

size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
    size_t totalSize = size * nmemb;
    output->append(static_cast<char*>(contents), totalSize);
    return totalSize;
}

std::string getTimestamp() {
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::tm* now_tm = std::localtime(&now_c);

    char buffer[80];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d_%H-%M-%S", now_tm);

    return std::string(buffer);
}

void pythonPost (std::string filePath) {

    const std::string pythonCommand = "python ../post.py -f \""  + filePath + "\"";
    std::cout <<"Run: " << pythonCommand << std::endl;

    // Open a pipe to capture the output of the Python script
    FILE* pipe = popen(pythonCommand.c_str(), "r");
    if (!pipe) {
        std::cerr << "Error opening pipe to Python script." << std::endl;
        return;
    }

    char buffer[128];
    std::string output;

    // Read the output of the Python script line by line
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        output += buffer;
    }

    // Close the pipe
    pclose(pipe);

    // Print the captured output
    std::cout << "Output from Python script:\n" << output << std::endl;
}

void saveJSON () {
    std::cout << "-------Save JSON-------" << std::endl;

    document.AddMember("data", dataArray, document.GetAllocator());// Add the dataArray to the document

    rapidjson::StringBuffer buffer;// Serialize the document to a JSON string
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    document.Accept(writer);

    std::string jsonString = buffer.GetString();

    std::string filename = "\\MouseData_" + getTimestamp() + ".json";
    std::string dir = "C:\\Users\\flori\\Desktop\\International Minor\\ParkinsonMouse\\RawMouseAnalyzer\\output";
    std::string filePath = dir + filename;

    std::ofstream outputFile(filePath);

    // Check if the file was opened successfully
    if (!outputFile.is_open()) {
        std::cerr << "Failed to open the file for writing: " << filename << std::endl;
        return;
    }

    outputFile << jsonString;// Write the JSON string to the file
    outputFile.close();// Close the file
    std::cout << "JSON saved to file: " << filename << std::endl;

    pythonPost(filePath);
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
}

bool isKeyPressed(int keyCode) {
    return GetAsyncKeyState(keyCode) & 0x8000;
}

int getSessionID (std::string directoryPath) {
    // Check if the directory exists
    if (!std::filesystem::exists(directoryPath)) {
        std::cerr << "Directory does not exist." << std::endl;
        return 1;
    }

    // Iterate over the files in the directory
    int fileCount = 0;
    for (const auto& entry : fs::directory_iterator(directoryPath)) {
        if (entry.is_regular_file()) {
            fileCount++;
        }
    }
    return fileCount + 1;
}

int main() {
    std::cout << "Program start" << std::endl;
    std::atexit(saveJSON);

    document.SetObject();//Prepare json

    int id = getSessionID("../output");
    rapidjson::Value sessionID;
    sessionID.SetInt(id);
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

    std::cout << "Collect Mouse Data... Quit with Q" << std::endl;
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
            //std::cout << "Any event" << std::endl;
        }
    }

    return DefWindowProc(hwnd, event, wparam, lparam);
}

