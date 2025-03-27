# Text Search with Fuzzy Matching (Levenshtein Distance)

## Description

This program performs a parallel search for a keyword in a text file. It can find exact matches and fuzzy matches using the Levenshtein distance (edit distance) for keyword similarity. The program can handle large files efficiently by using multiple threads for concurrent searching. It also ensures thread safety while adding results, preventing duplicate entries from being output. This tool is particularly useful when you need to search for a keyword that may have slight variations (e.g., typos or spelling differences).

## Features

- **Parallel Search**: Utilizes multiple threads to speed up the search process, making it ideal for large files.
- **Fuzzy Matching**: Supports fuzzy search using Levenshtein distance for inexact matches, allowing you to find keywords even with small errors.
- **Thread Safety**: Ensures that multiple threads can safely add results to the shared resource (results vector) without causing data races.
- **Efficient Result Handling**: Avoids duplicate entries in the results by checking each line for uniqueness, ensuring only unique matches are displayed.

## Requirements

- **C++11 or later**: The program is written using C++11 features for better efficiency and parallelism.
- **g++ Compiler**: You need the `g++` compiler to compile the program. Ensure that it supports C++11 or later and that it has threading capabilities enabled.

## Compilation

To compile the program, use the following command:

```bash
g++ -std=c++11 or uese C++17 -o textSearch textSearch.cpp -pthread

run the file with the word you need to search

─ ./textSearch sample.txt "pizza" 4  
