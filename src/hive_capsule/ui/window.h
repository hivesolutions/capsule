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

/**
 * The width of the download window.
 */
#define DOWNLOAD_WINDOW_WIDTH 500

/**
 * The height of the download window.
 */
#define DOWNLOAD_WINDOW_HEIGHT 140

extern HINSTANCE instance;
extern HWND handler_progress;
extern UINT change_label_event_value;
extern UINT change_label_string_event_value;
extern UINT change_progress_event_value;

DWORD WINAPI WindowThread(LPVOID parameters);
LRESULT CALLBACK WndProc(HWND handler_window, UINT message, WPARAM w_param, LPARAM l_param);
int RegisterClass(HINSTANCE handler_instance);
int CancelProcess();
