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
    char szFileName[MAX_PATH];
    GetModuleFileName(NULL, szFileName, MAX_PATH);
    return CSData::getData(szFileName);
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

    if (resourcePath == NULL) {
        memset(data, 0, dataSize);
        data->numberFiles = 0;
    } else {
        HGLOBAL resource = LoadResource(library, resourcePath);
        struct Data_t *_data = (struct Data_t *) LockResource(resource);
        memcpy(data, _data, dataSize);
    }

    FreeLibrary(library);

    return data;
}

void CSData::setData(struct Data_t *data) {
    char szFileName[MAX_PATH];
    GetModuleFileName(NULL, szFileName, MAX_PATH);
    CSData::setData(szFileName, data);
}

void CSData::setData(char *filePath, struct Data_t *data) {
    size_t dataSize = sizeof(Data_t);

    HANDLE resource = BeginUpdateResource(filePath, false);
    BOOL success = UpdateResource(resource, RT_RCDATA, "DATA", MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), data, dataSize);
    if(success == TRUE) {
        EndUpdateResource(resource, FALSE);
    }
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

    CSData::setData(filePath, data);

    free(data);
}

void CSData::popDataFile() {
    char szFileName[MAX_PATH];
    GetModuleFileName(NULL, szFileName, MAX_PATH);
    CSData::popDataFile(szFileName);
}

void CSData::popDataFile(char *filePath) {
    struct Data_t *data = CSData::getData(filePath);

    if(data->numberFiles == 0) {
        throw "No more data files available to pop";
    }

    memset(&data->dataFiles[data->numberFiles - 1], 0, sizeof(struct DataFile_t));
    data->numberFiles--;

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

    stream << "Name => '" << dataFile->name << "'\n";
    stream << "Description => '" << dataFile->description << "'\n";
    stream << "URL => '" << dataFile->url << "'\n";

    free(data);
}

void CSData::printData(std::ostream &stream) {
    char szFileName[MAX_PATH];
    GetModuleFileName(NULL, szFileName, MAX_PATH);
    CSData::printData(szFileName, stream);
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
