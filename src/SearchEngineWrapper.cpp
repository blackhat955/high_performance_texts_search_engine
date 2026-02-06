#include "SearchEngineWrapper.h"
#include "SearchWorker.h"
#include <iostream>

Napi::FunctionReference SearchEngineWrapper::constructor;

Napi::Object SearchEngineWrapper::Init(Napi::Env env, Napi::Object exports) {
    Napi::HandleScope scope(env);

    Napi::Function func = DefineClass(env, "SearchEngine", {
        InstanceMethod("search", &SearchEngineWrapper::Search)
    });

    constructor = Napi::Persistent(func);
    constructor.SuppressDestruct();

    exports.Set("SearchEngine", func);
    return exports;
}

SearchEngineWrapper::SearchEngineWrapper(const Napi::CallbackInfo& info) : Napi::ObjectWrap<SearchEngineWrapper>(info) {
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);

    if (info.Length() < 1 || !info[0].IsString()) {
        Napi::TypeError::New(env, "String expected (file path)").ThrowAsJavaScriptException();
        return;
    }

    std::string filepath = info[0].As<Napi::String>();
    this->searchEngine_ = std::unique_ptr<SearchEngine>(new SearchEngine());
    
    try {
        this->searchEngine_->LoadFile(filepath);
        // std::cout << "Index loaded successfully from " << filepath << std::endl;
    } catch (const std::exception& e) {
        Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
    }
}

Napi::Value SearchEngineWrapper::Search(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    if (info.Length() < 3) {
        Napi::TypeError::New(env, "Expected 3 arguments: query, threshold, callback").ThrowAsJavaScriptException();
        return env.Null();
    }

    std::string query = info[0].As<Napi::String>();
    int threshold = info[1].As<Napi::Number>().Int32Value();
    Napi::Function callback = info[2].As<Napi::Function>();

    SearchWorker* worker = new SearchWorker(callback, searchEngine_.get(), query, threshold);
    worker->Queue();

    return env.Undefined();
}
