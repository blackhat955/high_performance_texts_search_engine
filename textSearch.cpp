#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <thread>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <algorithm>
#include <unordered_set>
#include <memory>  // Include for std::unique_ptr
#include <chrono>  // For timing execution

using namespace std;

// Trie Node Definition using unique_ptr
struct TrieNode {
    unordered_map<char, unique_ptr<TrieNode>> children;
    bool is_end_of_word = false;
};

class Trie {
    unique_ptr<TrieNode> root;
    shared_mutex trie_mutex;

public:
    Trie() : root(make_unique<TrieNode>()) {}

    void insert(const string& word) {
        unique_lock lock(trie_mutex);
        TrieNode* node = root.get();
        for (char ch : word) {
            if (!node->children[ch]) {
                node->children[ch] = make_unique<TrieNode>();
            }
            node = node->children[ch].get();
        }
        node->is_end_of_word = true;
    }

    bool search(const string& word) {
        shared_lock lock(trie_mutex);
        TrieNode* node = root.get();
        for (char ch : word) {
            if (!node->children.count(ch)) return false;
            node = node->children[ch].get();
        }
        return node->is_end_of_word;
    }
};


// Fuzzy search using Levenshtein Distance
int levenshteinDistance(const string &s1, const string &s2) {
    vector<vector<int>> dp(s1.size() + 1, vector<int>(s2.size() + 1));
    for (size_t i = 0; i <= s1.size(); ++i) dp[i][0] = i;
    for (size_t j = 0; j <= s2.size(); ++j) dp[0][j] = j;
    
    for (size_t i = 1; i <= s1.size(); ++i) {
        for (size_t j = 1; j <= s2.size(); ++j) {
            if (s1[i - 1] == s2[j - 1]) {
                dp[i][j] = dp[i - 1][j - 1];
            } else {
                dp[i][j] = 1 + min({dp[i - 1][j], dp[i][j - 1], dp[i - 1][j - 1]});
            }
        }
    }
    return dp[s1.size()][s2.size()];
}

mutex file_mutex;
unordered_set<string> unique_lines;

void searchFile(const string &filename, const string &keyword, Trie &trie, vector<string> &results) {
    ifstream file(filename);
    if (!file) {
        cerr << "Error opening file: " << filename << endl;
        return;
    }

    string line;
    size_t line_number = 0;
    while (getline(file, line)) {
        ++line_number;

        // Ensure the line contains the keyword or is within Levenshtein distance.
        if (line.find(keyword) != string::npos || levenshteinDistance(line, keyword) <= 2) {
            lock_guard<mutex> lock(file_mutex);  // Ensure thread-safe access to the results vector
            // Ensure the line is unique (avoid duplicates)
            if (unique_lines.find(line) == unique_lines.end()) {
                unique_lines.insert(line);
                results.push_back("Line " + to_string(line_number) + ": " + line);
            }
        }
    }
}

void parallelSearch(const string &filename, const string &keyword, int thread_count) {
    Trie trie;
    trie.insert(keyword);

    vector<string> results;
    vector<thread> threads;
    for (int i = 0; i < thread_count; ++i) {
        threads.emplace_back(searchFile, filename, keyword, ref(trie), ref(results));
    }
    
    for (auto &t : threads) {
        t.join();
    }

    // Output the results once all threads have finished
    for (const auto &res : results) {
        cout << res << endl;
    }
}

void singleThreadSearch(const string &filename, const string &keyword) {
    Trie trie;
    trie.insert(keyword);
    vector<string> results;
    searchFile(filename, keyword, trie, results);

    // Output the results
    for (const auto &res : results) {
        cout << res << endl;
    }
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        cerr << "Usage: " << argv[0] << " <filename> <keyword> [threads]" << endl;
        return 1;
    }

    string filename = argv[1];
    string keyword = argv[2];
    int thread_count = (argc > 3) ? stoi(argv[3]) : 4; // Default to 4 threads

    // Measure performance for single-threaded execution
    auto start = chrono::high_resolution_clock::now();
    cout << "Running multi-threaded search..." << endl;
    singleThreadSearch(filename, keyword);
    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> single_thread_duration = end - start;
    cout << "Multi-threaded search took: " << single_thread_duration.count() << " seconds." << endl;

    // Measure performance for multi-threaded execution
    start = chrono::high_resolution_clock::now();
    cout << "\nRunning single-threaded search with " << thread_count << " threads..." << endl;
    parallelSearch(filename, keyword, thread_count);
    end = chrono::high_resolution_clock::now();
    chrono::duration<double> multi_thread_duration = end - start;
    cout << "Single-threaded search took: " << multi_thread_duration.count() << " seconds." << endl;

    return 0;
}

