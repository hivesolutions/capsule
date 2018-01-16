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

#include "data.h"

struct DataType *CSData::GetData() {
    char file_name[MAX_PATH];
    GetModuleFileName(NULL, file_name, MAX_PATH);
    return CSData::GetData(file_name);
}

struct DataType *CSData::GetData(char *file_path) {
    struct DataType *data;

    HMODULE library = LoadLibrary(file_path);
    if(library == NULL) {
        throw "Invalid data path";
    }

    HRSRC resource_path = FindResource(library, "DATA", RT_RCDATA);

    size_t data_size = sizeof(struct DataType);
    data = (struct DataType *) malloc(data_size);

    if(resource_path == NULL) {
        memset(data, 0, data_size);
        data->number_files = 0;
    } else {
        HGLOBAL resource = LoadResource(library, resource_path);
        struct DataType *_data = (struct DataType *) LockResource(resource);
        memcpy(data, _data, data_size);
    }

    resource_path = FindResource(library, "BUFFER", RT_RCDATA);

    size_t buffer_size = data->buffer_size;
    char *buffer = (char *) malloc(data->buffer_size);

    if(resource_path == NULL) {
        memset(buffer, 0, buffer_size);
    } else {
        HGLOBAL resource = LoadResource(library, resource_path);
        char *_buffer = (char *) LockResource(resource);
        memcpy(buffer, _buffer, buffer_size);
    }

    for(size_t index = 0; index < data->number_files; index++) {
        struct DataFileType *data_file = &data->data_files[index];
        data_file->buffer = buffer + data_file->buffer_offset;
    }

    FreeLibrary(library);

    return data;
}

void CSData::SetData(struct DataType *data) {
    char file_name[MAX_PATH];
    GetModuleFileName(NULL, file_name, MAX_PATH);
    CSData::SetData(file_name, data);
}

void CSData::SetData(char *file_path, struct DataType *data) {
    size_t data_size = sizeof(struct DataType);

    HANDLE resource = BeginUpdateResource(file_path, false);
    BOOL success_data = UpdateResource(resource, RT_RCDATA, "DATA", MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), data, data_size);

    if(success_data == TRUE) {
        EndUpdateResource(resource, FALSE);
    }
}

void CSData::SetBuffer(struct DataType *data) {
    char file_name[MAX_PATH];
    GetModuleFileName(NULL, file_name, MAX_PATH);
    CSData::SetBuffer(file_name, data);
}

void CSData::SetBuffer(char *file_path, struct DataType *data) {
    size_t buffer_size = 0;

    for(size_t index = 0; index < data->number_files; index++) {
        struct DataFileType *data_file = &data->data_files[index];

        buffer_size += data_file->buffer_size;
    }

    data->buffer_size = buffer_size;

    // prints a Debug message into the logger
    JBLogger::GetLogger("setup")->Debug("Allocating buffer of size %d bytes ...", buffer_size);

    char *buffer = (char *) malloc(buffer_size);
    char *buffer_original = buffer;

    size_t offset = 0;

    for(size_t index = 0; index < data->number_files; index++) {
        struct DataFileType *data_file = &data->data_files[index];
        memcpy(buffer, data_file->buffer, data_file->buffer_size);
        data_file->buffer_offset = offset;

        buffer += data_file->buffer_size;
        offset += data_file->buffer_size;
    }

    buffer = buffer_original;

    // prints a Debug message into the logger
    JBLogger::GetLogger("setup")->Debug("Saving buffer resource into file");

    HANDLE resource = BeginUpdateResource(file_path, false);
    BOOL success = UpdateResource(
        resource,
        RT_RCDATA,
        "BUFFER",
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        buffer,
        buffer_size
    );

    if(success == TRUE) {
        EndUpdateResource(resource, FALSE);
    }

    free(buffer);
}

void CSData::AppendDataFile(struct DataFileType *data_file) {
    char file_name[MAX_PATH];
    GetModuleFileName(NULL, file_name, MAX_PATH);
    CSData::AppendDataFile(file_name, data_file);
}

void CSData::AppendDataFile(char *file_path, struct DataFileType *data_file) {
    struct DataType *data = CSData::GetData(file_path);

    if(data->number_files == DATA_FILE_COUNT) {
        throw "No more space available in the data file";
    }

    memcpy(&data->data_files[data->number_files], data_file, sizeof(struct DataFileType));
    data->number_files++;

    CSData::SetBuffer(file_path, data);
    CSData::SetData(file_path, data);

    free(data);
}

void CSData::PopDataFile() {
    char file_name[MAX_PATH];
    GetModuleFileName(NULL, file_name, MAX_PATH);
    CSData::PopDataFile(file_name);
}

void CSData::PopDataFile(char *file_path) {
    struct DataType *data = CSData::GetData(file_path);

    if(data->number_files == 0) {
        throw "No more data files available to pop";
    }

    memset(&data->data_files[data->number_files - 1], 0, sizeof(struct DataFileType));
    data->number_files--;

    CSData::SetBuffer(file_path, data);
    CSData::SetData(file_path, data);

    free(data);
}

void CSData::PrintDataFile(char *file_path, size_t index) {
    CSData::PrintDataFile(file_path, index, std::cout);
}

void CSData::PrintDataFile(char *file_path, size_t index, std::ostream &stream) {
    struct DataType *data = CSData::GetData(file_path);

    if(index > data->number_files - 1) {
        throw "Data file index not found in data";
    }

    struct DataFileType *data_file = &data->data_files[index];

    stream << "Type => '" << data_file->type << "'\n";
    stream << "Name => '" << data_file->name << "'\n";
    stream << "Description => '" << data_file->description << "'\n";
    stream << "URL => '" << data_file->url << "'\n";
    stream << "Size => '" << data_file->buffer_size << "'\n";
    stream << "Offset => '" << data_file->buffer_offset << "'\n";

    free(data);
}

void CSData::PrintData(std::ostream &stream) {
    char file_name[MAX_PATH];
    GetModuleFileName(NULL, file_name, MAX_PATH);
    CSData::PrintData(file_name, stream);
}

void CSData::PrintData(char *file_path, std::ostream &stream) {
    struct DataType *data = CSData::GetData(file_path);

    bool is_first = true;

    for(size_t index = 0; index < data->number_files; index++) {
        if(is_first) { is_first = false; } else { stream << "\n"; }
        CSData::PrintDataFile(file_path, index, stream);
    }

    free(data);
}
