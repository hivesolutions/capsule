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

#pragma once

/**
 * The value defining the amximum number
 * of data files that may be stored in
 * a single executable file.
 */
#define DATA_FILE_COUNT 8

enum DataFileModeType {
    REMOTE = 1,
    LOCAL
};

struct DataFileType {
    enum DataFileModeType type;
    char name[256];
    char description[256];
    char url[2048];
    char *buffer;
    unsigned int buffer_size;
    unsigned int buffer_offset;
};

struct DataType {
    size_t number_files;
    size_t buffer_size;
    struct DataFileType data_files[DATA_FILE_COUNT];
};

class CSData {
    public:
        static struct DataType *GetData();
        static struct DataType *GetData(char *file_path);
        static void SetData(struct DataType *data);
        static void SetData(char *file_path, struct DataType *data);
        static void SetBuffer(struct DataType *data);
        static void SetBuffer(char *file_path, struct DataType *data);
        static void AppendDataFile(struct DataFileType *data_file);
        static void AppendDataFile(char *file_path, struct DataFileType *data_file);
        static void PopDataFile();
        static void PopDataFile(char *file_path);
        static void PrintDataFile(char *file_path, size_t index);
        static void PrintDataFile(char *file_path, size_t index, std::ostream &stream);
        static void PrintData(std::ostream &stream);
        static void PrintData(char *file_path, std::ostream &stream);
};
