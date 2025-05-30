#include <napi.h>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <algorithm>

std::mutex result_mutex;
std::vector<std::string> global_results;

int levenshteinDistance(const std::string &s1, const std::string &s2) {
    size_t m = s1.size(), n = s2.size();
    std::vector<std::vector<int>> dp(m + 1, std::vector<int>(n + 1));

    for (size_t i = 0; i <= m; ++i) dp[i][0] = i;
    for (size_t j = 0; j <= n; ++j) dp[0][j] = j;

    for (size_t i = 1; i <= m; ++i) {
        for (size_t j = 1; j <= n; ++j) {
            if (s1[i - 1] == s2[j - 1])
                dp[i][j] = dp[i - 1][j - 1];
            else
                dp[i][j] = 1 + std::min({ dp[i - 1][j], dp[i][j - 1], dp[i - 1][j - 1] });
        }
    }

    return dp[m][n];
}

void processChunk(const std::vector<std::string>& lines, const std::string& query, int threshold) {
    std::vector<std::string> local_results;

    for (const auto& line : lines) {
        std::istringstream iss(line);
        std::string word;
        bool match_found = false;

        while (iss >> word) {
            if (levenshteinDistance(query, word) <= threshold || word.find(query) != std::string::npos) {
                match_found = true;
                break;
            }
        }

        if (match_found) {
            local_results.push_back(line);
        }
    }

    std::lock_guard<std::mutex> lock(result_mutex);
    global_results.insert(global_results.end(), local_results.begin(), local_results.end());
}

Napi::Value SearchFile(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    if (info.Length() < 1 || !info[0].IsString()) {
        Napi::TypeError::New(env, "Expected a string as query").ThrowAsJavaScriptException();
        return env.Null();
    }

    std::string query = info[0].As<Napi::String>();
    int threshold = 2;  // typo tolerance

    std::ifstream file("sample.txt");
    if (!file.is_open()) {
        Napi::Error::New(env, "Failed to open sample.txt").ThrowAsJavaScriptException();
        return env.Null();
    }

    std::vector<std::string> lines;
    std::string line;
    while (std::getline(file, line)) {
        lines.push_back(line);
    }

    global_results.clear();
    size_t thread_count = std::thread::hardware_concurrency();
    size_t chunk_size = lines.size() / thread_count + 1;

    std::vector<std::thread> threads;
    for (size_t i = 0; i < lines.size(); i += chunk_size) {
        size_t end = std::min(i + chunk_size, lines.size());
        std::vector<std::string> chunk(lines.begin() + i, lines.begin() + end);
        threads.emplace_back(processChunk, chunk, query, threshold);
    }

    for (auto& t : threads) t.join();

    Napi::Array resultArray = Napi::Array::New(env);
    for (size_t i = 0; i < global_results.size(); ++i) {
        resultArray[i] = Napi::String::New(env, global_results[i]);
    }

    return resultArray;
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
    exports.Set("search", Napi::Function::New(env, SearchFile));
    return exports;
}

NODE_API_MODULE(search, Init)
