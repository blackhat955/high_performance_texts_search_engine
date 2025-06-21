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
    // no manual deletion is required because unique pointer 
    // automatically manage the memory
    Trie() : root(make_unique<TrieNode>()) {}

    void insert(const string& word) {
       // Locke for exclusie access only one thread modify at a time
        unique_lock lock(trie_mutex);
        // start form root node
        TrieNode* node = root.get();
    
        for (char ch : word) {
            // if char is not present in trie make new trie node
            if (!node->children[ch]) {
                node->children[ch] = make_unique<TrieNode>();
            }
            // move to the next node
            node = node->children[ch].get();
        }
        // mark the last node 
        // as th end  of the word
        node->is_end_of_word = true;
    }

    bool search(const string& word) {
        // similar here make sure only oe thread modify at a 
        // time
        // searching is not modify the trie so 
        // we use share lock over here 
        // this allow the mutiple search at the same time and improve the performace significantly
        // since we don't neew to modified naything over here dob;t needt ot use unique ptr which block the othere 
        // thread which only wannt to read the data especiall reduce 
        // reduce the performance if may thread searching for thing a the smae time 

        // Uses shared_lock (from <shared_mutex>), which allows multiple concurrent readers but blocks writers.
        // Searching doesn't modify the Trie, so it's safe to let many threads search at the same time.
        shared_lock lock(trie_mutex);
        
        // satrt form th e  root node 
        TrieNode* node = root.get();
        for (char ch : word) {
            // if char  is not found return 
            // false 
            if (!node->children.count(ch)) return false;
            node = node->children[ch].get();
        }
        // return 
        // true if the end of the word
        return node->is_end_of_word;
    }
};


// Fuzzy search using Levenshtein Distance
int levenshteinDistance(const string &s1, const string &s2) {
    vector<vector<int>> dp(s1.size() + 1, vector<int>(s2.size() + 1));
    // fill up 
    // based case
    for (size_t i = 0; i <= s1.size(); ++i) dp[i][0] = i;// cost of deleting i charcted
    for (size_t j = 0; j <= s2.size(); ++j) dp[0][j] = j; // cost of deleting j characte
    
    for (size_t i = 1; i <= s1.size(); ++i) {
        for (size_t j = 1; j <= s2.size(); ++j) {
            //If the characters match, no operation is needed
            //Just carry forward the previous result dp[i][j] = dp[i-1][j-1]
            
            if (s1[i - 1] == s2[j - 1]) {// base case: no operation is needed
                dp[i][j] = dp[i - 1][j - 1];  // distace remain same as previous on
            } else {
                dp[i][j] = 1 + min({dp[i - 1][j],// Delete char for s1
                    dp[i][j - 1],   // insert char to s1
                    dp[i - 1][j - 1]}); // replce char in s1
            }
        }
    }
    return dp[s1.size()][s2.size()]; //  min number of edit req
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
    while (getline(file, line)) { // read file by line 
                                  // line
        ++line_number;

        // Ensure the line contains the keyword or is within Levenshtein distance.
        if (line.find(keyword) != -1 || levenshteinDistance(line, keyword) <= 4) { // first half before ot is ensure that
             // line contain exact keyword  and secode check if line line with in levestine distace 
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
        // this meand multiple thread can work simulteneoully and because of it we pass the reference of th trie and result so each thread has these attributes
        threads.emplace_back(searchFile, filename, keyword, ref(trie), ref(results));// assign chunks to file for the seraching using thre prallel serach
    }
    
    for (auto &t : threads) { // this enssur that all thread 
        // create is finish
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

    string filename = argv[1]; // exract file name

    string keyword = argv[2];// keywords
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

