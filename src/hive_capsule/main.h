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

#include "global/resource.h"

/**
 * The default name to be used used for generation
 * of duplicated (cloned) capsule files.
 * This name must be able to correclty identify
 * an installer file.
 */
#define DEFAULT_SETUP_NAME "setup.exe"

/**
 * The default name to be used used for generation
 * of the dump data files.
 */
#define DEFAULT_DUMP_NAME "dump.dat"

/**
 * The default title to be shown in the installer windows
 * when no other title overrides it by configuration.
 */
#define DEFAULT_WINDOW_TITLE "Capsule Installer"

/**
 * The text string to be used to presetn and help
 * orientation to the final end user.
 */
#define HELP_SUPPORT_MESSAGE "capsule run\n\
   Normal execution of the capsule installation system\n\n\
capsule duplicate <target>\n\
capsule clone <target>\n\
    Duplicates the current file creating an exact copy\n\n\
capsule append <target> name description url\n\
    Adds a data information file to the capsule file, may be used for runtime\n\n\
capsule pop\n\
    Removes the last data information file present in the capsule file\n\n\
capsule dump <file>\n\
    Removes the last data information file present in the capsule file"

int APIENTRY _tWinMain(HINSTANCE handlerInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow);
