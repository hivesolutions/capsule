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

#include "../ui/ui.h"
#include "observer.h"

CSDownloadObserver::CSDownloadObserver() : JBObserver() {
    this->last_value = 0;
};

CSDownloadObserver::~CSDownloadObserver() {
};

void CSDownloadObserver::SetWindowHandler(HWND &window_handler) {
    this->window_handler = window_handler;
};

void CSDownloadObserver::SetProgressHandler(HWND &progress_handler) {
    this->progress_handler = progress_handler;
};

void CSDownloadObserver::SetDownloadItem(CSDownloadItem &download_item) {
    this->download_item = download_item;
};

void CSDownloadObserver::Update(JBObservable &element, std::string &event_name, void *arguments) {
    JBObserver::Update(element, event_name, arguments);

    if(event_name == "header_loaded") {
        // retrieves the value from the arguments
        int value = *(int *) arguments;

        // sets the scale
        this->scale = value;

        // creates the message to be used in the transfer
        std::string *message = new std::string("Transfering " + this->download_item.GetDescription());

        // sends the message to change the label string of the window
        // and also sets the progress handler to the original (start position)
        SendMessage(this->window_handler, change_label_string_event_value, (WPARAM) message, NULL);
        SendMessage(this->progress_handler, PBM_SETPOS, 0, NULL);

        // invalidates the window rectangle
        InvalidateRect(this->window_handler, NULL, true);
    }
    else if(event_name == "download_changed") {
        // retrieves the value from the arguments
        int value = *(int *) arguments;

        // in case the delta value is less than 100 KB
        if(value - this->last_value < 102400)
            return;

        // sets the last value
        this->last_value = value;

        // calculates the final value
        int final_value = (int) (((float) value / (float) this->scale) * 100);

        // sends a message to set the position in the progress handler
        SendMessage(this->progress_handler, PBM_SETPOS, final_value, NULL);
    }
    else if(event_name == "download_completed") {
        // sets the progress bar at the end
        SendMessage(this->progress_handler, PBM_SETPOS, 100, NULL);
    }
};
