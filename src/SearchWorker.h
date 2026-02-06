#ifndef SEARCH_WORKER_H
#define SEARCH_WORKER_H

#include <napi.h>
#include "SearchEngine.h"

class SearchWorker : public Napi::AsyncWorker {
public:
    SearchWorker(Napi::Function& callback, SearchEngine* engine, std::string query, int threshold)
        : Napi::AsyncWorker(callback), engine_(engine), query_(query), threshold_(threshold) {}

    ~SearchWorker() {}

    // Executed on a worker thread.
    void Execute() override {
        try {
            results_ = engine_->Search(query_, threshold_);
        } catch (const std::exception& e) {
            SetError(e.what());
        }
    }

    // Executed on the main event loop.
    void OnOK() override {
        Napi::HandleScope scope(Env());
        Napi::Array resultArray = Napi::Array::New(Env(), results_.size());
        for (size_t i = 0; i < results_.size(); ++i) {
            resultArray[i] = Napi::String::New(Env(), results_[i]);
        }
        Callback().Call({Env().Null(), resultArray});
    }

private:
    SearchEngine* engine_;
    std::string query_;
    int threshold_;
    std::vector<std::string> results_;
};

#endif // SEARCH_WORKER_H
