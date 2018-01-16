// Hive Capsule Installer
// Copyright (c) 2008-2018 Hive Solutions Lda.
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
// __copyright__ = Copyright (c) 2008-2018 Hive Solutions Lda.
// __license__   = Apache License, Version 2.0

#include "stdafx.h"

#include "util.h"

#ifdef CS_DEBUG
void CSUtil::SartLogger() {
    JBLogger *logger = JBLogger::GetLogger("setup");
    JBLoggerFileHandler *file_handler = new JBLoggerFileHandler(std::string("setup.log"));
    logger->AddHandler(file_handler);
    logger->SetLevel(DEBUG);
    logger->Info("Log system started");
}

void CSUtil::StartConsole() {
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

void CSUtil::StopConsole() {
    FreeConsole();
}
#else
void CSUtil::SartLogger() { }
void CSUtil::StartConsole() { }
void CSUtil::StopConsole() { }
#endif
