#include "InputTracker.h"
#include "SessionManager.h"
#include "EventLogger.h"
#include <windows.h>
#include <iostream>
#include <memory> 

LRESULT CALLBACK InputWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_INPUT) {
        UINT dwSize = 0;
        GetRawInputData((HRAWINPUT)lParam, RID_INPUT, nullptr, &dwSize, sizeof(RAWINPUTHEADER));
        std::unique_ptr<BYTE[]> lpb(new BYTE[dwSize]);
        if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, lpb.get(), &dwSize, sizeof(RAWINPUTHEADER)) == dwSize) {
            RAWINPUT* raw = (RAWINPUT*)lpb.get();
            if (raw->header.dwType == RIM_TYPEKEYBOARD) {
                USHORT key = raw->data.keyboard.VKey;
                USHORT flags = raw->data.keyboard.Flags;
                std::string action = (flags & RI_KEY_BREAK) ? "released" : "pressed";

                std::cout << "[InputTracker] Key " << action << ": " << key << std::endl;
                LogEvent("Key" + action, std::to_string(key));
            }
            if (raw->header.dwType == RIM_TYPEMOUSE) {
                RAWMOUSE& mouse = raw->data.mouse;

                if (mouse.lLastX != 0 || mouse.lLastY != 0) {
                    std::string movement = "X=" + std::to_string(mouse.lLastX) + " Y=" + std::to_string(mouse.lLastY);
                    std::cout << "[Mouse] Move: " << movement << std::endl;
                    LogEvent("MouseMove", movement);
                }

                if (mouse.usButtonFlags & RI_MOUSE_LEFT_BUTTON_DOWN) {
                    std::cout << "[Mouse] Left button down" << std::endl;
                    LogEvent("MouseClick", "LeftDown");
                }
                if (mouse.usButtonFlags & RI_MOUSE_LEFT_BUTTON_UP) {
                    std::cout << "[Mouse] Left button up" << std::endl;
                    LogEvent("MouseClick", "LeftUp");
                }
                if (mouse.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_DOWN) {
                    std::cout << "[Mouse] Right button down" << std::endl;
                    LogEvent("MouseClick", "RightDown");
                }
                if (mouse.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_UP) {
                    std::cout << "[Mouse] Right button up" << std::endl;
                    LogEvent("MouseClick", "RightUp");
                }
                if (mouse.usButtonFlags & RI_MOUSE_MIDDLE_BUTTON_DOWN) {
                    std::cout << "[Mouse] Middle button down" << std::endl;
                    LogEvent("MouseClick", "MiddleDown");
                }
                if (mouse.usButtonFlags & RI_MOUSE_MIDDLE_BUTTON_UP) {
                    std::cout << "[Mouse] Middle button up" << std::endl;
                    LogEvent("MouseClick", "MiddleUp");
                }
                if (mouse.usButtonFlags & RI_MOUSE_WHEEL) {
                    SHORT wheelDelta = (SHORT)mouse.usButtonData;
                    std::cout << "[Mouse] Wheel delta: " << wheelDelta << std::endl;
                    LogEvent("MouseWheel", std::to_string(wheelDelta));
                }
            }
        }
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

void TrackInputs() {
    const wchar_t* className = L"InputTrackerWindow";
    const wchar_t* windowTitle = L"Input Tracker";

    WNDCLASS wc = {};
    wc.lpfnWndProc = InputWndProc;
    wc.hInstance = GetModuleHandle(nullptr);
    wc.lpszClassName = className;

    if (!RegisterClass(&wc)) {
        std::cerr << "[InputTracker] Failed to register window class." << std::endl;
        return;
    }
    HWND hwnd = CreateWindowEx(0, className, windowTitle, 0, 0, 0, 0, 0, nullptr, nullptr, wc.hInstance, nullptr);
    if (!hwnd) {
        std::cerr << "[InputTracker] Failed to create window." << std::endl;
        return;
    }

    RAWINPUTDEVICE devices[2];
    devices[0].usUsagePage = 0x01;
    devices[0].usUsage = 0x06;
    devices[0].dwFlags = RIDEV_INPUTSINK;
    devices[0].hwndTarget = hwnd;

    devices[1].usUsagePage = 0x01;
    devices[1].usUsage = 0x02;
    devices[1].dwFlags = RIDEV_INPUTSINK;
    devices[1].hwndTarget = hwnd;
    if (!RegisterRawInputDevices(devices, 2, sizeof(RAWINPUTDEVICE))) {
        std::cerr << "[InputTracker] Failed to register raw input devices." << std::endl;
        return;
    }

    std::string sessionFile = StartNewSession();
    InitLogger(sessionFile);
    std::cout << "[SessionManager] Started new session: " << sessionFile << std::endl;

    std::cout << "[InputTracker] Listening for keyboard and mouse input..." << std::endl;

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}