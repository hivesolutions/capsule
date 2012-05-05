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

#include "data.h"

struct Data_t *CSData::getData() {
    char fileName[MAX_PATH];
    GetModuleFileName(NULL, fileName, MAX_PATH);
    return CSData::getData(fileName);
}

struct Data_t *CSData::getData(char *filePath) {
    struct Data_t *data;

    HMODULE library = LoadLibrary(filePath);
    if(library == NULL) {
        throw "Invalid data path";
    }

    HRSRC resourcePath = FindResource(library, "DATA", RT_RCDATA);

    size_t dataSize = sizeof(struct Data_t);
    data = (struct Data_t *) malloc(dataSize);

    if(resourcePath == NULL) {
        memset(data, 0, dataSize);
        data->numberFiles = 0;
    } else {
        HGLOBAL resource = LoadResource(library, resourcePath);
        struct Data_t *_data = (struct Data_t *) LockResource(resource);
        memcpy(data, _data, dataSize);
    }

    resourcePath = FindResource(library, "BUFFER", RT_RCDATA);

    size_t bufferSize = data->bufferSize;
    char *buffer = (char *) malloc(data->bufferSize);

    if(resourcePath == NULL) {
        memset(buffer, 0, bufferSize);
    } else {
        HGLOBAL resource = LoadResource(library, resourcePath);
        char *_buffer = (char *) LockResource(resource);
        memcpy(buffer, _buffer, bufferSize);
    }

    for(size_t index = 0; index < data->numberFiles; index++) {
        struct DataFile_t *dataFile = &data->dataFiles[index];
        dataFile->buffer = buffer + dataFile->bufferOffset;
    }

    FreeLibrary(library);

    return data;
}

void CSData::setData(struct Data_t *data) {
    char fileName[MAX_PATH];
    GetModuleFileName(NULL, fileName, MAX_PATH);
    CSData::setData(fileName, data);
}

void CSData::setData(char *filePath, struct Data_t *data) {
    size_t dataSize = sizeof(Data_t);

    HANDLE resource = BeginUpdateResource(filePath, false);
    BOOL successData = UpdateResource(resource, RT_RCDATA, "DATA", MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), data, dataSize);

    if(successData == TRUE) {
        EndUpdateResource(resource, FALSE);
    }
}

void CSData::setBuffer(struct Data_t *data) {
    char fileName[MAX_PATH];
    GetModuleFileName(NULL, fileName, MAX_PATH);
    CSData::setBuffer(fileName, data);
}

void CSData::setBuffer(char *filePath, struct Data_t *data) {
    size_t bufferSize = 0;

    for(size_t index = 0; index < data->numberFiles; index++) {
        struct DataFile_t *dataFile = &data->dataFiles[index];

        bufferSize += dataFile->bufferSize;
    }

    data->bufferSize = bufferSize;

    // prints a debug message into the logger
    JBLogger::getLogger("setup")->debug("Allocating buffer of size %d bytes ...", bufferSize);

    char *buffer = (char *) malloc(bufferSize);
    char *bufferOriginal = buffer;

    size_t offset = 0;

    for(size_t index = 0; index < data->numberFiles; index++) {
        struct DataFile_t *dataFile = &data->dataFiles[index];
        memcpy(buffer, dataFile->buffer, dataFile->bufferSize);
        dataFile->bufferOffset = offset;

        buffer += dataFile->bufferSize;
        offset += dataFile->bufferSize;
    }

    buffer = bufferOriginal;

    // prints a debug message into the logger
    JBLogger::getLogger("setup")->debug("Saving buffer resource into file");

    HANDLE resource = BeginUpdateResource(filePath, false);
    BOOL success = UpdateResource(resource, RT_RCDATA, "BUFFER", MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), buffer, bufferSize);

    if(success == TRUE) {
        EndUpdateResource(resource, FALSE);
    }

    free(buffer);
}

void CSData::appendDataFile(struct DataFile_t *dataFile) {
    char fileName[MAX_PATH];
    GetModuleFileName(NULL, fileName, MAX_PATH);
    CSData::appendDataFile(fileName, dataFile);
}

void CSData::appendDataFile(char *filePath, struct DataFile_t *dataFile) {
    struct Data_t *data = CSData::getData(filePath);

    if(data->numberFiles == DATA_FILE_COUNT) {
        throw "No more space available in the data file";
    }

    memcpy(&data->dataFiles[data->numberFiles], dataFile, sizeof(struct DataFile_t));
    data->numberFiles++;

    CSData::setBuffer(filePath, data);
    CSData::setData(filePath, data);

    free(data);
}

void CSData::popDataFile() {
    char fileName[MAX_PATH];
    GetModuleFileName(NULL, fileName, MAX_PATH);
    CSData::popDataFile(fileName);
}

void CSData::popDataFile(char *filePath) {
    struct Data_t *data = CSData::getData(filePath);

    if(data->numberFiles == 0) {
        throw "No more data files available to pop";
    }

    memset(&data->dataFiles[data->numberFiles - 1], 0, sizeof(struct DataFile_t));
    data->numberFiles--;

    CSData::setBuffer(filePath, data);
    CSData::setData(filePath, data);

    free(data);
}

void CSData::printDataFile(char *filePath, size_t index) {
    CSData::printDataFile(filePath, index, std::cout);
}

void CSData::printDataFile(char *filePath, size_t index, std::ostream &stream) {
    struct Data_t *data = CSData::getData(filePath);

    if(index > data->numberFiles - 1) {
        throw "Data file index not found in data";
    }

    struct DataFile_t *dataFile = &data->dataFiles[index];

    stream << "Type => '" << dataFile->type << "'\n";
    stream << "Name => '" << dataFile->name << "'\n";
    stream << "Description => '" << dataFile->description << "'\n";
    stream << "URL => '" << dataFile->url << "'\n";
    stream << "Size => '" << dataFile->bufferSize << "'\n";
    stream << "Offset => '" << dataFile->bufferOffset << "'\n";

    free(data);
}

void CSData::printData(std::ostream &stream) {
    char fileName[MAX_PATH];
    GetModuleFileName(NULL, fileName, MAX_PATH);
    CSData::printData(fileName, stream);
}

void CSData::printData(char *filePath, std::ostream &stream) {
    struct Data_t *data = CSData::getData(filePath);

    bool isFirst = true;

    for(size_t index = 0; index < data->numberFiles; index++) {
        if(isFirst) { isFirst = false; } else { stream << "\n"; }
        CSData::printDataFile(filePath, index, stream);
    }

    free(data);
}
