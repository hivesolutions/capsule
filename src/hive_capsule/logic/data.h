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
 * The value defining the amximum number
 * of data files that may be stored in
 * a single executable file.
 */
#define DATA_FILE_COUNT 8

typedef struct DataFile_t {
    char name[256];
    char description[256];
    char url[2048];
} DataFile;

typedef struct Data_t {
    size_t numberFiles;
    struct DataFile_t dataFiles[DATA_FILE_COUNT];
} Data;

class CSData {
    public:
        static struct Data_t *getData();
        static struct Data_t *getData(char *filePath);
        static void setData(struct Data_t *data);
        static void setData(char *filePath, struct Data_t *data);
        static void appendDataFile(struct DataFile_t *dataFile);
        static void appendDataFile(char *filePath, struct DataFile_t *dataFile);
        static void popDataFile();
        static void popDataFile(char *filePath);
        static void printDataFile(char *filePath, size_t index);
        static void printDataFile(char *filePath, size_t index, std::ostream &stream);
        static void printData(std::ostream &stream);
        static void printData(char *filePath, std::ostream &stream);
};
