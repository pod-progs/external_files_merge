# external_files_merge

This code merges the market data symbol files into a single merged file.
It achieves this in follwing steps
1. Initialize the files details fron input paths and creates temp directory to store intermediary files.
2. Merge the input files in the batch of 100 files and create merged temporary file.
3. Merge the temp files to generate final merged result file
4. Cleans all temporary generated files and directory.
