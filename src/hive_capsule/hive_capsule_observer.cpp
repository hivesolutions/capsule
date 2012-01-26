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

#include <commctrl.h>

#include "hive_capsule_window.h"
#include "hive_capsule_observer.h"

CColonyDownloadObserver::CColonyDownloadObserver() : JBObserver() {
    this->lastValue = 0;
};

CColonyDownloadObserver::~CColonyDownloadObserver() {
};

void CColonyDownloadObserver::setWindowHandler(HWND &windowHandler) {
    this->windowHandler = windowHandler;
};

void CColonyDownloadObserver::setProgressHandler(HWND &progressHandler) {
    this->progressHandler = progressHandler;
};

void CColonyDownloadObserver::setDownloadItem(CColonyDownloadItem &downloadItem) {
    this->downloadItem = downloadItem;
};

void CColonyDownloadObserver::update(JBObservable &element, std::string &eventName, void *arguments) {
    JBObserver::update(element, eventName, arguments);

    if(eventName == "header_loaded") {
        // retrieves the value from the arguments
        int value = *(int *) arguments;

        // sets the scale
        this->scale = value;

        // creates the message to be used in the transfer
        std::string *message = new std::string("Transfering " + this->downloadItem.getDescription());

        // sends the message to change the label string of the window
        SendMessage(this->windowHandler, changeLabelStringEventValue, (WPARAM) message, NULL);

        // invalidates the window rectangle
        InvalidateRect(this->windowHandler, NULL, true);
    }
    else if(eventName == "download_changed") {
        // retrieves the value from the arguments
        int value = *(int *) arguments;

        // in case the delta value is less than 100 KB
        if(value - this->lastValue < 102400)
            return;

        // sets the last value
        this->lastValue = value;

        // calculates the final value
        int finalValue = (int) (((float) value / (float) this->scale) * 100);

        // sends a message to set the position in the progress handler
        SendMessage(this->progressHandler, PBM_SETPOS, finalValue, NULL);
    }
    else if(eventName == "download_completed") {
        // sets the progress bar at the end
        SendMessage(this->progressHandler, PBM_SETPOS, 100, NULL);
    }
};
