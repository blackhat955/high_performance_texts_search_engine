#ifndef SEARCH_ENGINE_H
#define SEARCH_ENGINE_H

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <mutex>

struct TrieNode {
    std::unordered_map<char, std::shared_ptr<TrieNode>> children;
    bool is_end_of_word = false;
};

class SearchEngine {
public:
    SearchEngine();
    ~SearchEngine();

    // Loads the file and builds the index (Trie + Inverted Index)
    void LoadFile(const std::string& filepath);

    // Performs a fuzzy search
    // Returns a list of matching lines
    std::vector<std::string> Search(const std::string& query, int threshold);

private:
    void InsertToTrie(const std::string& word);

    // Data Stores
    std::vector<std::string> lines_; // Stores the actual content of the file
    std::unordered_map<std::string, std::vector<int>> inverted_index_; // word -> line IDs
    std::shared_ptr<TrieNode> trie_root_;

    // Thread safety (though index is read-only after load)
    std::mutex data_mutex_;
};

#endif // SEARCH_ENGINE_H
