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

#include "stdafx.h"

#include "util.h"

#ifdef CS_DEBUG
void CSUtil::sartLogger() {
    JBLogger *logger = JBLogger::getLogger("setup");
    JBLoggerFileHandler *fileHandler = new JBLoggerFileHandler(std::string("setup.log"));
    logger->addHandler(fileHandler);
    logger->setLevel(DEBUG);
    logger->info("Log system started");
}

void CSUtil::startConsole() {
    // allocates the console interface in the windows
    // sub system and attaches it to the curren process
    AllocConsole();

    // saves the various default stream files into temporary
    // variables to be able to use them as value references
    FILE *_stdin = stdin;
    FILE *_stdout = stdout;
    FILE *_stderr = stderr;

    // reopens the standard input, output and error files
    // and sets then to the console input and output systems
    freopen_s(&_stdin, "CONIN$", "rb", stdin);
    freopen_s(&_stdout, "CONOUT$", "wb", stdout);
    freopen_s(&_stderr, "CONOUT$", "wb", stderr);
}

void CSUtil::stopConsole() {
    FreeConsole();
}
#else
void CSUtil::sartLogger() { }
void CSUtil::startConsole() { }
void CSUtil::stopConsole() { }
#endif
