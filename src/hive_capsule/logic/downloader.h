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

#define TEMP_FILE_PREFIX "hve"

class CSDownloadItem {
    private:
        std::string name;
        std::string description;
        std::string address;
        char *buffer;
        size_t bufferSize;
    public:
        CSDownloadItem();
        CSDownloadItem(std::string &name, std::string &description, std::string &address);
        CSDownloadItem(std::string &name, std::string &description, char *buffer, size_t bufferSize);
        ~CSDownloadItem();
        std::string &getName();
        std::string &getDescription();
        std::string &getAddress();
        char *getBuffer();
        size_t getBufferSize();
};

class CSDownloader {
    private:
        HWND *handlerWindowReference;
        std::string baseDownloadAddress;
        std::vector<CSDownloadItem> downloadItems;
        std::map<std::string, std::string> downloadItemsFilePathMap;
        std::vector<std::string> temporaryFiles;
        std::vector<std::string> temporaryDirectories;
        std::string tempPath;

        void loadItem(CSDownloadItem &downloadItem);
        void downloadItem(CSDownloadItem &downloadItem);
        void unpackItem(CSDownloadItem &downloadItem, std::string &targetPath);
        void generateTempPath();
        std::string &getTempPath();
    public:
        CSDownloader();
        ~CSDownloader();
        void createDownloadWindow(HINSTANCE handlerInstance, int nCmdShow);
        void downloadFiles();
        std::string unpackFiles(std::string targetPath = "");
        std::string deployFiles(std::string deployPath);
        void deleteTemporaryFiles();
        void setBaseDownloadAddress(std::string &baseDownloadAddress);
        void addDownloadItem(CSDownloadItem &downloadItem);
        CSDownloadItem &addDownloadFile(std::string &name, std::string &description);
        std::string &getDownloadItemFilePath(CSDownloadItem &downloadItem);
};
