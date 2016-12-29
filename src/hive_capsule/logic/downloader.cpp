// Hive Capsule Installer
// Copyright (c) 2008-2017 Hive Solutions Lda.
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
// __copyright__ = Copyright (c) 2008-2017 Hive Solutions Lda.
// __license__   = Apache License, Version 2.0

#include "stdafx.h"

#include "../ui/ui.h"
#include "../main.h"
#include "observer.h"
#include "util.h"

#include "downloader.h"

CSDownloadItem::CSDownloadItem() {
}

CSDownloadItem::CSDownloadItem(std::string &name, std::string &description, std::string &address) {
    this->name = name;
    this->description = description;
    this->address = address;
    this->buffer = NULL;
    this->buffer_size = 0;
}

CSDownloadItem::CSDownloadItem(std::string &name, std::string &description, char *buffer, size_t buffer_size) {
    this->name = name;
    this->description = description;
    this->address = name;
    this->buffer = buffer;
    this->buffer_size = buffer_size;
}

CSDownloadItem::~CSDownloadItem() {
}

std::string &CSDownloadItem::GetName() {
    return this->name;
}

std::string &CSDownloadItem::GetDescription() {
    return this->description;
}

std::string &CSDownloadItem::GetAddress() {
    return this->address;
}

char *CSDownloadItem::GetBuffer() {
    return this->buffer;
}

size_t CSDownloadItem::GetBufferSize() {
    return this->buffer_size;
}

CSDownloader::CSDownloader() {
    this->temp_path = "";
}

CSDownloader::~CSDownloader() {
}

void CSDownloader::LoadItem(CSDownloadItem &download_item) {
    // allocates space for the file name
    char file_name[MAX_PATH];

    // retrieves the temp path in char
    const char *temp_path_char = this->_GetTempPath().c_str();

    // tries to retrieve a temporary file name
    if(!GetTempFileName(temp_path_char, TEMP_FILE_PREFIX, NULL, file_name)) {
        throw "Unable to create temporary file name";
    }

    // opens the file for read and write, writes the contents to the file
    // and closes it (avoid leaking)
    std::fstream file = std::fstream(file_name, std::fstream::in | std::fstream::out | std::fstream::trunc | std::fstream::binary);
    file.write(download_item.GetBuffer(), download_item.GetBufferSize());
    file.close();

    // sets the download item file path association in the map
    this->download_items_file_path_map[download_item.GetAddress()] = file_name;
}

void CSDownloader::DownloadItem(CSDownloadItem &download_item) {
    // creates a new (jimbo) http client
    JBHttpClient http_client = JBHttpClient();

    // creates a new download observer object
    CSDownloadObserver download_observer = CSDownloadObserver();

    // sets the window handler in the download observer
    download_observer.SetWindowHandler(*(this->handler_window_reference));

    // sets the progress handler in the download observer
    download_observer.SetProgressHandler(handler_progress);

    // sets the download item in the download observer
    download_observer.SetDownloadItem(download_item);

    // registers the observer for the header loaded, download
    // changed and download completed events
    http_client.RegisterObserverForEvent("header_loaded", download_observer);
    http_client.RegisterObserverForEvent("download_changed", download_observer);
    http_client.RegisterObserverForEvent("download_completed", download_observer);

    // retrieves the remote contents
    http_client.GetContents(download_item.GetAddress());

    // allocates space for the file name
    char file_name[MAX_PATH];

    // retrieves the temp path in char
    const char *temp_path_char = this->_GetTempPath().c_str();

    // tries to retrieve a temporary file name
    if(!GetTempFileName(temp_path_char, TEMP_FILE_PREFIX, NULL, file_name)) {
        throw "Unable to create temporary file name";
    }

    // opens the file for read and write, writes the contents to the file
    // and closes it (avoid leaking)
    std::fstream file = std::fstream(
        file_name,
        std::fstream::in | std::fstream::out |
        std::fstream::trunc | std::fstream::binary
    );
    file.write(http_client.GetMessageBuffer(), http_client.GetMessageSize());
    file.close();

    // sets the download item file path association in the map
    this->download_items_file_path_map[download_item.GetAddress()] = file_name;
}

void CSDownloader::UnpackItem(CSDownloadItem &download_item, std::string &target_path) {
    // retrieves the download item file path
    std::string &download_item_file_path = this->GetDownloadItemFilePath(download_item);

    // constructs the tar file path from the download item file path
    std::string tar_file_path = download_item_file_path + ".tar";

    // unpacks the gz file to the tar file
    JBPackerGz::UnpackFile(download_item_file_path, tar_file_path);

    // unpacks the tar file to the temp path
    JBPackerTar::UnpackFile(tar_file_path, target_path + "/");

    this->temporary_files.push_back(download_item_file_path);
    this->temporary_files.push_back(tar_file_path);
}

void CSDownloader::GenerateTempPath() {
    // allocates space for the temp path then retrieves
    // the local temp path (by copy) and converts it into
    // string setting it in the current object
    char temp_path[1024];
    GetTempPath(1024, temp_path);
    this->temp_path = std::string(temp_path);
}

std::string &CSDownloader::_GetTempPath() {
    // in case no temp path is currently set in the object
    // a new one must be generated (on demand generation)
    if(this->temp_path == "") { this->GenerateTempPath(); }

    // retrieves the current associated temp path, must be
    // unique in the system
    return this->temp_path;
}

void CSDownloader::CreateDownloadWindow(HINSTANCE handler_instance, int cmd_show) {
    // allocates space for the thread id
    DWORD thread_id;

    // creates the window event to controll the window creation
    HANDLE window_event = CreateEvent(NULL, false, false, NULL);

    // allocates space for the thread arguments
    std::vector<void *> thread_arguments;

    // adds the arguments to the thread arguments list
    thread_arguments.push_back(handler_instance);
    thread_arguments.push_back(&cmd_show);
    thread_arguments.push_back(&this->handler_window_reference);
    thread_arguments.push_back(window_event);

    // creates the thread and retrieves the thread handle
    HANDLE thread_handle = CreateThread(NULL, 0, WindowThread, (void *) &thread_arguments, 0, &thread_id);

    // waits for the window event to be released
    WaitForSingleObject(window_event, INFINITE);

    // closes the window event handle
    CloseHandle(window_event);
}

void CSDownloader::DownloadFiles() {
    for(size_t index = 0; index < this->download_items.size(); index++) {
        CSDownloadItem &download_item = this->download_items[index];
        if(download_item.GetBuffer() == NULL) { this->DownloadItem(download_item); }
        else { this->LoadItem(download_item); }
    }
}

std::string CSDownloader::UnpackFiles(std::string target_path) {
    // sends the message to change the label of the window,
    // then invalidates the window rectangle and after that
    // sends a message to change the progress bar to marquee
    SendMessage(
        *this->handler_window_reference,
        change_label_event_value,
        (WPARAM) "Uncompressing installation files", NULL
    );
    InvalidateRect(*this->handler_window_reference, NULL, true);
    SendMessage(
        *this->handler_window_reference,
        change_progress_event_value,
        2,
        NULL
    );

    if(target_path == "") {
        // allocates space for the file name
        char file_name[MAX_PATH];

        // retrieves the temp path in char
        const char *temp_path_char = this->_GetTempPath().c_str();

        // tries to retrieve a temporary file name
        if(!GetTempFileName(temp_path_char, TEMP_FILE_PREFIX, NULL, file_name)) {
            throw "Unable to create temporary file name";
        }

        // uses the temporary name to create a directory with it
        // appends a suffix to it for the purpose
        std::string &directory_name = std::string(file_name) + std::string("dir");

        // creates the temporary directory
        if(!CreateDirectory(directory_name.c_str(), NULL)) {
            throw "Unable to create directory";
        }

        // sets the current directory as the target path for
        // the unpacking of the files
        target_path = directory_name;

        // adds the temporary file name to the list of temporary files
        this->temporary_files.push_back(std::string(file_name));
    }

    // iterates over all the downloaded items to unpack them
    // into the target path (directory)
    for(size_t index = 0; index < this->download_items.size(); index++) {
        // retrieves the current iteration download item
        // and then unpacks it into the target path
        CSDownloadItem &download_item = this->download_items[index];
        this->UnpackItem(download_item, target_path);
    }

    // adds the target path to the list of tempoary directories
    // (keeps track of them for latter removal)
    this->temporary_directories.push_back(target_path);

    return target_path;
}

std::string CSDownloader::DeployFiles(std::string deploy_path) {
    // prints an Info message into the logger
    JBLogger::GetLogger("setup")->Info("Deploying files ...");

    // sends the message to change the label of the window,
    // then invalidates the window rectangle and after that
    // sends a message to change the progress bar to marquee
    SendMessage(*this->handler_window_reference, change_label_event_value, (WPARAM) "Deploying files to destination", NULL);
    InvalidateRect(*this->handler_window_reference, NULL, true);
    SendMessage(*this->handler_window_reference, change_progress_event_value, 2, NULL);

    // creates the directory for the deployment (this will ensure
    // that the directory exists)
    CreateDirectory(deploy_path.c_str(), NULL);

    // iterates over all the temporary directories to copy them into
    // the "final" deploy target path
    for(size_t index = 0; index < this->temporary_directories.size(); index++) {
        std::string &temporary_directory = this->temporary_directories[index];
        JBWindows::CopyRecursiveShell(deploy_path, temporary_directory);
    }

    // returns the deploy path that was used in the deployment
    // phase (this should be used by the caller for confirmation)
    return deploy_path;
}

void CSDownloader::DeleteTemporaryFiles() {
    // prints an Info message into the logger
    JBLogger::GetLogger("setup")->Info("Deleting temporary files ...");

    // sends the message to change the label of the window,
    // then invalidates the window rectangle and after that
    // sends a message to change the progress bar to marquee
    SendMessage(*this->handler_window_reference, change_label_event_value, (WPARAM) "Deleting temporary files", NULL);
    InvalidateRect(*this->handler_window_reference, NULL, true);
    SendMessage(*this->handler_window_reference, change_progress_event_value, 2, NULL);

    for(size_t index = 0; index < this->temporary_directories.size(); index++) {
        std::string &temporary_directory = this->temporary_directories[index];
        JBWindows::DeleteRecursiveShell(temporary_directory.c_str(), false);
    }

    for(size_t index = 0; index < this->temporary_files.size(); index++) {
        std::string &temporary_file = this->temporary_files[index];
        DeleteFile(temporary_file.c_str());
    }

    // prints an Info message into the logger
    JBLogger::GetLogger("setup")->Info("Finished deleting temporary files");

    // closes (destroy) the window in the most correct procedure
    // this call is done to prevent pending handles
    DestroyWindow(*this->handler_window_reference);
}

void CSDownloader::SetBaseDownloadAddress(std::string &base_download_address){
    this->base_download_address = base_download_address;
}

void CSDownloader::AddDownloadItem(CSDownloadItem &download_item) {
    this->download_items.push_back(download_item);
}

CSDownloadItem &CSDownloader::AddDownloadFile(std::string &name, std::string &description) {
    CSDownloadItem download_item = CSDownloadItem(name, description, this->base_download_address + "/" + name);
    this->download_items.push_back(download_item);
    return this->download_items.back();
}

std::string &CSDownloader::GetDownloadItemFilePath(CSDownloadItem &download_item) {
    std::string &download_item_address = download_item.GetAddress();
    std::string &file_path = this->download_items_file_path_map[download_item_address];
    return file_path;
}
