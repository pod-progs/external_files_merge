#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <queue>
#include <filesystem>
#include <algorithm>
#include <chrono>

#include "market_data.hpp"
#include "directory_util.hpp"
#include "thread_pool.hpp"

class FileMerge
{
    std::string input_directory;
    std::string output_directory;
    std::string tmp_directory;

    std::vector<std::string> input_files;
    size_t file_count;

    unsigned int concurrency_count;
    unsigned int max_files_to_process;

    const std::string headers = "Symbol, Timestamp, Price, Size, Exchange, Type";

public:
    FileMerge(const std::string& i_dir, const std::string& o_dir)
        : input_directory(i_dir), output_directory(o_dir),
        file_count(0),
        concurrency_count(std::thread::hardware_concurrency()),
        max_files_to_process(100)
    {
    }

    ~FileMerge() = default;


    void run()
    {
        try {
            auto start = std::chrono::high_resolution_clock::now();

            init();
            process_input_files();
            process_intermediary_files();
            delete_temp_directory();

            auto end = std::chrono::high_resolution_clock::now();
            std::cout << "Execution Time: " << std::chrono::duration_cast<std::chrono::seconds>(end - start).count() << " seconds\n";
        }
        catch (const std::exception& e) {
            std::cerr << "An error occurred: " << e.what() << std::endl;
        }
    }

private:

    void init()
    {
        input_files = DirectoryUtils::get_all_text_files(input_directory);
        file_count = input_files.size();

        tmp_directory = output_directory + "/tmp";
        if (DirectoryUtils::create_directory(tmp_directory) != 0) {
            throw std::runtime_error("Failed to create temporary directory.");
        }
    }

    void merge_files(const std::vector<std::string>& files, const std::string& out_file, const bool intermediary_files = false)
    {
        std::vector<std::ifstream> inputs;
        inputs.reserve(files.size());

        for (const auto& file : files) {
            inputs.emplace_back(file);
        }

        std::priority_queue<std::pair<MarketData, size_t>, std::vector<std::pair<MarketData, size_t>>, decltype(&compare_market_data)> min_heap(&compare_market_data);

        // Read initial data from each file
        for (size_t i = 0; i < inputs.size(); ++i) {
            std::string line;
            std::getline(inputs[i], line); // Skip header line

            if (std::getline(inputs[i], line)) {
                std::istringstream iss(line);
                MarketData md;

                if (intermediary_files) {
                    std::getline(iss, md.symbol, ',');
                }
                else {
                    md.symbol = fs::path(files[i]).stem().string();
                }

                std::getline(iss, md.timestamp, ',');
                iss >> md.price;
                iss.ignore();
                iss >> md.size;
                iss.ignore();
                std::getline(iss, md.exchange, ',');
                std::getline(iss, md.type);
                min_heap.push(std::make_pair(md, i));
            }
        }

        // Opening a new output file and writing headers
        std::ofstream ofile(out_file);
        ofile << headers << '\n';

        // Merge data from files
        while (!min_heap.empty()) {
            auto entry = min_heap.top();
            min_heap.pop();
            ofile << entry.first << "\n";

            auto fileIndex = entry.second;

            if (inputs[fileIndex]) {
                std::string line;
                if (std::getline(inputs[fileIndex], line)) {
                    std::istringstream iss(line);
                    MarketData md;

                    if (intermediary_files) {
                        std::getline(iss, md.symbol, ',');
                    }
                    else {
                        md.symbol = fs::path(files[fileIndex]).stem().string();
                    }

                    std::getline(iss, md.timestamp, ',');
                    iss >> md.price;
                    iss.ignore();
                    iss >> md.size;
                    iss.ignore();
                    std::getline(iss, md.exchange, ',');
                    std::getline(iss, md.type);
                    min_heap.push(std::make_pair(md, fileIndex));
                }
                else {
                    inputs[fileIndex].close();
                }
            }
        }

        ofile.close();
    }

    void process_input_files()
    {
        ThreadPool threadPool(concurrency_count);
        for (size_t i = 0; i < file_count; i += max_files_to_process) {
            size_t left = i;
            size_t right = std::min(i + max_files_to_process, file_count);
            std::vector<std::string> files(input_files.begin() + left, input_files.begin() + right);
            std::string out_file = tmp_directory + "/tmp_" + std::to_string(i) + ".txt";
            //merge_files(files, out_file, false);
            auto merge_task = [this, _files = std::move(files), _out_path = std::move(out_file)]()
            {
                merge_files(_files, _out_path, false);
            };
            threadPool.enqueue(merge_task);            
        }
    }

    void process_intermediary_files()
    {
        std::vector<std::string> tmp_files = DirectoryUtils::get_all_text_files(tmp_directory);
        std::string res_file = output_directory + "/merged_market_data.txt";

        if (tmp_files.size() == 1)
        {
            if (fs::exists(res_file)) {
                fs::copy_file(tmp_files[0], res_file, fs::copy_options::overwrite_existing);
            }
            else {
                fs::copy_file(tmp_files[0], res_file);
            }
            return;
        }

        merge_files(tmp_files, res_file, true);
    }

    void delete_temp_directory()
    {
        if (DirectoryUtils::delete_directory(tmp_directory) != 0) {
            std::cerr << "Failed to delete temporary directory." << std::endl;
        }
    }
};



