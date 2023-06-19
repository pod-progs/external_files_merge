#pragma once
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

class DirectoryUtils
{
public:
    static std::vector<std::string> get_all_text_files(const std::string& path)
    {
        std::vector<std::string> files;

        try {
            if (!fs::exists(path)) {
                throw std::runtime_error("Directory does not exist: " + path);
            }

            for (const auto& file : fs::directory_iterator(path)) {
                if (fs::is_regular_file(file) && file.path().extension() == ".txt")
                {
                    files.push_back(file.path().string());
                }
            }
        }
        catch (const std::exception& ex) {
            std::cerr << "Error: " << ex.what() << std::endl;
        }

        return files;
    }

    static int create_directory(const std::string& path) {
        try {
            if (fs::exists(path)) {
                throw std::runtime_error("Directory already exists: " + path);
            }

            if (fs::create_directory(path)) {
                std::cout << "Directory created successfully: " << path << std::endl;
            }
            else {
                throw std::runtime_error("Failed to create the directory: " + path);
            }
        }
        catch (const std::exception& ex) {
            std::cerr << "Error: " << ex.what() << std::endl;
            return -1;
        }
        return 0;
    }

    static int delete_directory(const std::string& path)
    {
        try {
            fs::remove_all(path);
        }
        catch (const fs::filesystem_error& ex) {
            std::cerr << "Failed to delete the tmp directory: " << ex.what() << std::endl;
            return ex.code().value();
        }
        return 0;
    }
};