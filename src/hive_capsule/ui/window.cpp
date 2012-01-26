// Hive Colony Framework
// Copyright (C) 2008 Hive Solutions Lda.
//
// This file is part of Hive Colony Framework.
//
// Hive Colony Framework is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Hive Colony Framework is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Hive Colony Framework. If not, see <http://www.gnu.org/licenses/>.

// __author__    = João Magalhães <joamag@hive.pt>
// __version__   = 1.0.0
// __revision__  = $LastChangedRevision$
// __date__      = $LastChangedDate$
// __copyright__ = Copyright (c) 2008 Hive Solutions Lda.
// __license__   = GNU General Public License (GPL), Version 3

#include "stdafx.h"

#include "../global/resource.h"
#include "../util.h"
#include "window.h"

// the handler instance variable
HINSTANCE hInst;

// the handler progress handler
HWND handlerProgress;

// the change label event value
UINT changeLabelEventValue;

// the change label string event value
UINT changeLabelStringEventValue;

// the change progress event value
UINT changeProgressEventValue;

// sets the window label value
std::string windowLabelValue = std::string("Transfering Colony Installer requisites...");

DWORD WINAPI windowThread(void *parameters) {
    // retrieves the thread arguments converting the parameters to a vector
    std::vector<void *> threadArguments =  *((std::vector<void *> *) parameters);

    // retrieves the handler instance
    HINSTANCE handlerInstance = (HINSTANCE) threadArguments[0];

    // retrieves the number command show
    int nCmdShow = *((int *) threadArguments[1]);

    // retrieves the pointer to the handler window reference
    HWND **handlerWindowReference = (HWND **) threadArguments[2];

    // retrieves the window event
    HANDLE windowEvent = (HANDLE) threadArguments[3];

    // retrieves the current screen width
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);

    // retrieves the current screen height
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    // calculates the x position of the window to be centered
    int windowX = (screenWidth / 2) - (DOWNLOAD_WINDOW_WIDTH / 2);

    // calculates the y position of the window to be centered
    int windowY = (screenHeight / 2) - (DOWNLOAD_WINDOW_HEIGHT / 2);

    // creates a new window to represent the download progress
    HWND handlerWindow = CreateWindow("defaultWindow", "Hive Colony Installer", WS_BORDER | WS_CAPTION | WS_SYSMENU, windowX,
                                      windowY, DOWNLOAD_WINDOW_WIDTH, DOWNLOAD_WINDOW_HEIGHT, NULL, NULL, handlerInstance, NULL);

    // sets the handler window reference
    *handlerWindowReference = &handlerWindow;

    // shows the window
    ShowWindow(handlerWindow, nCmdShow);

    // updates the window
    UpdateWindow(handlerWindow);

    // sets the event
    SetEvent(windowEvent);

    // allocates space for the message
    MSG message;

    // the main message loop
    // start by retrieving the message
    while(GetMessage(&message, NULL, 0, 0)) {
        // translates the message
        TranslateMessage(&message);

        // dispatches the message to the event handler
        DispatchMessage(&message);
    }

    return 0;
}

LRESULT CALLBACK WindowProc(HWND handlerWindow, UINT message, WPARAM wParam, LPARAM lParam) {
    int wmId;
    int wmEvent;
    PAINTSTRUCT paintStructure;
    HDC handlerDc;
    HFONT handlerFont;
    NONCLIENTMETRICS nonClientMetrics;

    // in case the event is of type change label
    if(message == changeLabelEventValue) {
        // sets the new window label value
        windowLabelValue = std::string((char *) wParam);
    }
    else if(message == changeLabelStringEventValue) {
        // retrieves the string value pointer
        std::string *stringValue = (std::string *) wParam;

        // sets the new window label value
        windowLabelValue = *stringValue;

        // deletes the string value
        delete stringValue;
    }
    else if(message == changeProgressEventValue) {
        switch(wParam) {
            case 1:
                // sends a message to close the progress bar
                SendMessage(handlerProgress, WM_CLOSE, NULL, NULL);

                // creates the progress bar retrieving the handler to the progress bar
                handlerProgress = CreateWindowEx(NULL, PROGRESS_CLASS, NULL, WS_CHILD | WS_VISIBLE | PBS_SMOOTH, 20, 60, 455, 17, handlerWindow, NULL, hInst, NULL);
            case 2:
                // sends a message to close the progress bar
                SendMessage(handlerProgress, WM_CLOSE, NULL, NULL);

                // creates the progress bar retrieving the handler to the progress bar
                handlerProgress = CreateWindowEx(NULL, PROGRESS_CLASS, NULL, WS_CHILD | WS_VISIBLE | PBS_MARQUEE, 20, 60, 455, 17, handlerWindow, NULL, hInst, NULL);

                // sends a message to start the marquee progress bar
                SendMessage(handlerProgress, PBM_SETMARQUEE, 100, NULL);
            default:
                break;
        }
    }

    // switches in the messa value
    switch(message) {
        case WM_CREATE:
            // creates the progress bar retrieving the handler to the progress bar
            handlerProgress = CreateWindowEx(NULL, PROGRESS_CLASS, NULL, WS_CHILD | WS_VISIBLE | PBS_SMOOTH, 20, 60, 455, 17, handlerWindow, NULL, hInst, NULL);

            // registers the change label message event
            changeLabelEventValue = RegisterWindowMessage("changeLabel");

            // registers the change label string message event
            changeLabelStringEventValue = RegisterWindowMessage("changeLabelString");

            // registers the progress message event
            changeProgressEventValue = RegisterWindowMessage("changeProgress");

            break;
        case WM_COMMAND:
            // retrieves the command id
            wmId = LOWORD(wParam);

            // retrieves the command event
            wmEvent = HIWORD(wParam);

            break;
        case WM_PAINT:
            // retrieves the paint context of the handler
            handlerDc = BeginPaint(handlerWindow, &paintStructure);

            // sets the background mode as transparent
            SetBkMode(handlerDc, TRANSPARENT);

            // retrieves the windows major version value
            if(GetWindowsMajorVersion() > 5) {
                // sets the non client metrics size
                nonClientMetrics.cbSize = sizeof(NONCLIENTMETRICS);

                // retrieves the system parameters
                SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(nonClientMetrics), &nonClientMetrics, 0);

                // creates the font base in the system settings
                handlerFont = (HFONT) CreateFontIndirect(&nonClientMetrics.lfMessageFont);
            }
            else {
                // retrieves the default gui font
                handlerFont = (HFONT) GetStockObject(DEFAULT_GUI_FONT);
            }

            // sets the current font
            SelectObject(handlerDc, handlerFont);

            // writes the text to the output
            TextOut(handlerDc, 20, 20, windowLabelValue.c_str(), windowLabelValue.size());

            // deletes the font handler
            DeleteObject(handlerFont);

            // finishes the painting
            EndPaint(handlerWindow, &paintStructure);

            break;
        case WM_DESTROY:
            // sends the post quit message
            PostQuitMessage(0);

            break;
        default:
            return DefWindowProc(handlerWindow, message, wParam, lParam);
    }

    return 0;
}

int registerClass(HINSTANCE handlerInstance) {
    WNDCLASSEX windowClassEx;

    windowClassEx.cbSize = sizeof(WNDCLASSEX);
    windowClassEx.style = CS_HREDRAW | CS_VREDRAW;
    windowClassEx.lpfnWndProc = WindowProc;
    windowClassEx.cbClsExtra = 0;
    windowClassEx.cbWndExtra = 0;
    windowClassEx.hInstance = handlerInstance;
    windowClassEx.hIcon = NULL;
    windowClassEx.hIconSm = LoadIcon(windowClassEx.hInstance, MAKEINTRESOURCE(IDI_SMALL));
    windowClassEx.hCursor = LoadCursor(NULL, IDC_ARROW);
    windowClassEx.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    windowClassEx.lpszMenuName = NULL;
    windowClassEx.lpszClassName = "defaultWindow";

    return RegisterClassEx(&windowClassEx);
}
