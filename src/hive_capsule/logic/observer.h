// Hive Capsule Installer
// Copyright (C) 2008 Hive Solutions Lda.
//
// This file is part of Hive Capsule Installer.
//
// Hive Capsule Installer is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Hive Capsule Installer is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Hive Capsule Installer. If not, see <http://www.gnu.org/licenses/>.

// __author__    = João Magalhães <joamag@hive.pt>
// __version__   = 1.0.0
// __revision__  = $LastChangedRevision$
// __date__      = $LastChangedDate$
// __copyright__ = Copyright (c) 2008 Hive Solutions Lda.
// __license__   = GNU General Public License (GPL), Version 3

#pragma once

#include "downloader.h"

class CSDownloadObserver : public JBObserver {
    private:
        HWND window_handler;
        HWND progress_handler;
        CSDownloadItem download_item;
        int scale;
        int last_value;
    public:
        CSDownloadObserver();
        ~CSDownloadObserver();
        void Update(JBObservable &element, std::string &event_name, void *arguments);
        void SetWindowHandler(HWND &window_handler);
        void SetProgressHandler(HWND &progress_handler);
        void SetDownloadItem(CSDownloadItem &download_item);
};
