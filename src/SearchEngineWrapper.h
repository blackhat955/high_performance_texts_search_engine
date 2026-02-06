#ifndef SEARCH_ENGINE_WRAPPER_H
#define SEARCH_ENGINE_WRAPPER_H

#include <napi.h>
#include "SearchEngine.h"

class SearchEngineWrapper : public Napi::ObjectWrap<SearchEngineWrapper> {
public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports);
    SearchEngineWrapper(const Napi::CallbackInfo& info);

private:
    static Napi::FunctionReference constructor;
    
    // JS Methods
    Napi::Value Search(const Napi::CallbackInfo& info);
    
    // Internal C++ Instance
    std::unique_ptr<SearchEngine> searchEngine_;
};

#endif
