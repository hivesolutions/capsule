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

#define TEMP_FILE_PREFIX "hve"

class CSDownloadItem {
    private:
        std::string name;
        std::string description;
        std::string address;
        char *buffer;
        size_t buffer_size;
    public:
        CSDownloadItem();
        CSDownloadItem(std::string &name, std::string &description, std::string &address);
        CSDownloadItem(std::string &name, std::string &description, char *buffer, size_t buffer_size);
        ~CSDownloadItem();
        std::string &GetName();
        std::string &GetDescription();
        std::string &GetAddress();
        char *GetBuffer();
        size_t GetBufferSize();
};

class CSDownloader {
    private:
        HWND *handler_window_reference;
        std::string base_download_address;
        std::vector<CSDownloadItem> download_items;
        std::map<std::string, std::string> download_items_file_path_map;
        std::vector<std::string> temporary_files;
        std::vector<std::string> temporary_directories;
        std::string temp_path;

        void LoadItem(CSDownloadItem &download_item);
        void DownloadItem(CSDownloadItem &download_item);
        void UnpackItem(CSDownloadItem &download_item, std::string &target_path);
        void GenerateTempPath();
        std::string &_GetTempPath();
    public:
        CSDownloader();
        ~CSDownloader();
        void CreateDownloadWindow(HINSTANCE handler_instance, int cmd_show);
        void DownloadFiles();
        std::string UnpackFiles(std::string target_path = "");
        std::string DeployFiles(std::string deploy_path);
        void DeleteTemporaryFiles();
        void SetBaseDownloadAddress(std::string &base_download_address);
        void AddDownloadItem(CSDownloadItem &download_item);
        CSDownloadItem &AddDownloadFile(std::string &name, std::string &description);
        std::string &GetDownloadItemFilePath(CSDownloadItem &download_item);
};
