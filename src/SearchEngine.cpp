#include "SearchEngine.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <unordered_set>
#include <numeric>

SearchEngine::SearchEngine() {
    trie_root_ = std::make_shared<TrieNode>();
}

SearchEngine::~SearchEngine() {}

void SearchEngine::InsertToTrie(const std::string& word) {
    std::shared_ptr<TrieNode> current = trie_root_;
    for (char ch : word) {
        if (current->children.find(ch) == current->children.end()) {
            current->children[ch] = std::make_shared<TrieNode>();
        }
        current = current->children[ch];
    }
    current->is_end_of_word = true;
}

void SearchEngine::LoadFile(const std::string& filepath) {
    std::lock_guard<std::mutex> lock(data_mutex_);
    
    std::ifstream file(filepath);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + filepath);
    }

    lines_.clear();
    inverted_index_.clear();
    trie_root_ = std::make_shared<TrieNode>(); // Reset Trie

    std::string line;
    int line_id = 0;
    while (std::getline(file, line)) {
        lines_.push_back(line);
        
        std::istringstream iss(line);
        std::string word;
        while (iss >> word) {
            // Optional: Normalize word (lowercase, remove punctuation) could go here
            // For now, keeping strict match as per original logic, but adding to structures
            
            // Add to Inverted Index
            inverted_index_[word].push_back(line_id);
            
            // Add to Trie
            InsertToTrie(word);
        }
        line_id++;
    }
}

// Redefining the recursive search to be cleaner
void SearchTrie(std::shared_ptr<TrieNode> node, char ch, const std::string& current_word, 
                const std::string& query, const std::vector<int>& previousRow, 
                std::unordered_set<std::string>& matched_words, int threshold) {
    
    int columns = query.size() + 1;
    std::vector<int> currentRow(columns);
    currentRow[0] = previousRow[0] + 1;

    int min_val = currentRow[0];

    for (int i = 1; i < columns; i++) {
        int insert_cost = currentRow[i - 1] + 1;
        int delete_cost = previousRow[i] + 1;
        int replace_cost = previousRow[i - 1];
        
        if (query[i - 1] != ch) {
            replace_cost += 1;
        }

        currentRow[i] = std::min({insert_cost, delete_cost, replace_cost});
        min_val = std::min(min_val, currentRow[i]);
    }

    if (node->is_end_of_word && currentRow.back() <= threshold) {
        matched_words.insert(current_word);
    }

    if (min_val <= threshold) {
        for (auto& pair : node->children) {
            SearchTrie(pair.second, pair.first, current_word + pair.first, query, currentRow, matched_words, threshold);
        }
    }
}


std::vector<std::string> SearchEngine::Search(const std::string& query, int threshold) {
    // 1. Find all words in the Trie that are within 'threshold' distance of 'query'
    std::unordered_set<std::string> matched_words;
    
    // Initial DP row: 0, 1, 2, ... query.length()
    std::vector<int> currentRow(query.size() + 1);
    std::iota(currentRow.begin(), currentRow.end(), 0);

    // Start recursion from root children
    for (auto& pair : trie_root_->children) {
        std::string start_char_str(1, pair.first);
        SearchTrie(pair.second, pair.first, start_char_str, query, currentRow, matched_words, threshold);
    }

    // 2. Collect line IDs for all matched words
    std::unordered_set<int> unique_line_ids;
    for (const auto& word : matched_words) {
        if (inverted_index_.count(word)) {
            const auto& ids = inverted_index_.at(word);
            unique_line_ids.insert(ids.begin(), ids.end());
        }
    }

    // 3. Retrieve lines
    std::vector<std::string> results;
    for (int id : unique_line_ids) {
        if (id >= 0 && id < (int)lines_.size()) {
            results.push_back(lines_[id]);
        }
    }
    
    // Sort results to be deterministic (optional but good for testing)
    // std::sort(results.begin(), results.end()); 

    return results;
}
