#pragma once
#include "files_merge.hpp"

int main()
{
    std::string input_directory = "C:/temp/files/";
    std::string output_file = "C:/temp/";

    FileMerge m(input_directory, output_file);
    m.run();

    return 0;
}