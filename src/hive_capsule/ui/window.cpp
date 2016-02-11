// Hive Capsule Installer
// Copyright (c) 2008-2016 Hive Solutions Lda.
//
// This file is part of Hive Capsule Installer.
//
// Hive Capsule Installer is free software: you can redistribute it and/or modify
// it under the terms of the Apache License as published by the Apache
// Foundation, either version 2.0 of the License, or (at your option) any
// later version.
//
// Hive Capsule Installer is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// Apache License for more details.
//
// You should have received a copy of the Apache License along with
// Hive Capsule Installer. If not, see <http://www.apache.org/licenses/>.

// __author__    = João Magalhães <joamag@hive.pt>
// __version__   = 1.0.0
// __revision__  = $LastChangedRevision$
// __date__      = $LastChangedDate$
// __copyright__ = Copyright (c) 2008-2016 Hive Solutions Lda.
// __license__   = Apache License, Version 2.0

#include "stdafx.h"

#include "../global/resource.h"
#include "../main.h"
#include "window.h"

// the handler instance variable
HINSTANCE instance;

// the handler progress handler
HWND handler_progress;

// the change label event value
UINT change_label_event_value;

// the change label string event value
UINT change_label_string_event_value;

// the change progress event value
UINT change_progress_event_value;

// sets the window label value
std::string window_label_value = std::string("Transfering Installer dependencies ...");

DWORD WINAPI WindowThread(void *parameters) {
    // retrieves the thread arguments converting the parameters to a vector
    std::vector<void *> thread_arguments =  *((std::vector<void *> *) parameters);

    // retrieves the handler instance
    HINSTANCE handler_instance = (HINSTANCE) thread_arguments[0];

    // retrieves the number command show
    int cmd_show = *((int *) thread_arguments[1]);

    // retrieves the pointer to the handler window reference
    HWND **handler_window_reference = (HWND **) thread_arguments[2];

    // retrieves the window event
    HANDLE window_event = (HANDLE) thread_arguments[3];

    // retrieves the current screen width and height for centering
    // window calculation
    int screen_width = GetSystemMetrics(SM_CXSCREEN);
    int screen_height = GetSystemMetrics(SM_CYSCREEN);

    // calculates the x and y positions of the window and makes
    // sure it kees centered in the screen
    int window_x = (screen_width / 2) - (DOWNLOAD_WINDOW_WIDTH / 2);
    int window_y = (screen_height / 2) - (DOWNLOAD_WINDOW_HEIGHT / 2);

    // creates a new window to represent the download progress
    HWND handler_window = CreateWindow(
        "default_window",
        DEFAULT_WINDOW_TITLE,
        WS_BORDER | WS_CAPTION | WS_SYSMENU,
        window_x, window_y, DOWNLOAD_WINDOW_WIDTH, DOWNLOAD_WINDOW_HEIGHT,
        NULL,
        NULL,
        handler_instance,
        NULL
    );

    // sets the handler window reference
    *handler_window_reference = &handler_window;

    // shows the window
    ShowWindow(handler_window, cmd_show);

    // updates the window
    UpdateWindow(handler_window);

    // sets the event
    SetEvent(window_event);

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

LRESULT CALLBACK WindowProc(HWND handler_window, UINT message, WPARAM w_param, LPARAM l_param) {
    int wm_id;
    int wm_event;
    PAINTSTRUCT paint_structure;
    HDC handler_dc;
    HFONT handler_font;
    NONCLIENTMETRICS non_client_metrics;

    // in case the event is of type change label
    if(message == change_label_event_value) {
        // sets the new window label value
        window_label_value = std::string((char *) w_param);
    }
    else if(message == change_label_string_event_value) {
        // retrieves the string value pointer
        std::string *string_value = (std::string *) w_param;

        // sets the new window label value
        window_label_value = *string_value;

        // deletes the string value
        delete string_value;
    }
    else if(message == change_progress_event_value) {
        switch(w_param) {
            case 1:
                // sends a message to close the progress bar
                SendMessage(handler_progress, WM_CLOSE, NULL, NULL);

                // creates the progress bar retrieving the handler to the progress bar
                handler_progress = CreateWindowEx(
                    NULL,
                    PROGRESS_CLASS,
                    NULL,
                    WS_CHILD | WS_VISIBLE | PBS_SMOOTH,
                    20, 60, 455, 17,
                    handler_window,
                    NULL,
                    instance,
                    NULL
                );

            case 2:
                // sends a message to close the progress bar
                SendMessage(handler_progress, WM_CLOSE, NULL, NULL);

                // creates the progress bar retrieving the handler to the progress bar
                handler_progress = CreateWindowEx(
                    NULL,
                    PROGRESS_CLASS,
                    NULL,
                    WS_CHILD | WS_VISIBLE | PBS_MARQUEE,
                    20, 60, 455, 17,
                    handler_window,
                    NULL,
                    instance,
                    NULL
                );

                // sends a message to start the marquee progress bar
                SendMessage(handler_progress, PBM_SETMARQUEE, 100, NULL);

            default:
                break;
        }
    }

    // switches over the windows the message value
    switch(message) {
        case WM_CREATE:
            // creates the progress bar retrieving the handler to the progress bar
            handler_progress = CreateWindowEx(
                NULL,
                PROGRESS_CLASS,
                NULL,
                WS_CHILD | WS_VISIBLE | PBS_SMOOTH,
                20, 60, 455, 17,
                handler_window,
                NULL,
                instance,
                NULL
            );

            // registers the change label message event
            change_label_event_value = RegisterWindowMessage("change_label");

            // registers the change label string message event
            change_label_string_event_value = RegisterWindowMessage("change_label_string");

            // registers the progress message event
            change_progress_event_value = RegisterWindowMessage("change_progress");

            break;

        case WM_COMMAND:
            // retrieves the command id
            wm_id = LOWORD(w_param);

            // retrieves the command event
            wm_event = HIWORD(w_param);

            break;

        case WM_PAINT:
            // retrieves the paint context of the handler
            handler_dc = BeginPaint(handler_window, &paint_structure);

            // sets the background mode as transparent
            SetBkMode(handler_dc, TRANSPARENT);

            // retrieves the windows major version value
            if(JBWindows::_GetVersion() > 5) {
                // sets the non client metrics size
                non_client_metrics.cbSize = sizeof(NONCLIENTMETRICS);

                // retrieves the system parameters
                SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(non_client_metrics), &non_client_metrics, 0);

                // creates the font base in the system settings
                handler_font = (HFONT) CreateFontIndirect(&non_client_metrics.lfMessageFont);
            }
            else {
                // retrieves the default gui font
                handler_font = (HFONT) GetStockObject(DEFAULT_GUI_FONT);
            }

            // sets the current font
            SelectObject(handler_dc, handler_font);

            // writes the text to the output
            TextOut(handler_dc, 20, 20, window_label_value.c_str(), window_label_value.size());

            // deletes the font handler
            DeleteObject(handler_font);

            // finishes the painting
            EndPaint(handler_window, &paint_structure);

            break;

        case WM_DESTROY:
            // sends the post quit message
            PostQuitMessage(0);

            break;

        case WM_CLOSE:
            // tries to cancel the current process
            // (interrogates the user about it)
            CancelProcess();

            break;

        default:
            return DefWindowProc(handler_window, message, w_param, l_param);
    }

    return 0;
}

int RegisterClass(HINSTANCE handler_instance) {
    WNDCLASSEX window_class_ex;

    window_class_ex.cbSize = sizeof(WNDCLASSEX);
    window_class_ex.style = CS_HREDRAW | CS_VREDRAW;
    window_class_ex.lpfnWndProc = WindowProc;
    window_class_ex.cbClsExtra = 0;
    window_class_ex.cbWndExtra = 0;
    window_class_ex.hInstance = handler_instance;
    window_class_ex.hIcon = NULL;
    window_class_ex.hCursor = LoadCursor(NULL, IDC_ARROW);
    window_class_ex.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    window_class_ex.lpszMenuName = NULL;
    window_class_ex.lpszClassName = "default_window";

    return RegisterClassEx(&window_class_ex);
}

int CancelProcess() {
    // shows a message box asking for confirmation
    // on the canceling of the installation
    int return_value = MessageBox(
        NULL,
        "Are you sure you want to cancel installation",
        DEFAULT_WINDOW_TITLE,
        MB_ICONWARNING | MB_OKCANCEL
    );

    // in case the return from the message box is cancel, the installer
    // process continues from where it was
    if(return_value == IDCANCEL) {
        // returns immediately with no error
        // set (no problems)
        return 0;
    }

    // exits the current process in an error state, this
    // should be considered an erroneous situation, (this
    // should be able to provide some garbage collection)
    exit(-1);
}
