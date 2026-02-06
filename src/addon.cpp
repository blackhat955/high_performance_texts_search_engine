#include <napi.h>
#include "SearchEngineWrapper.h"

Napi::Object InitAll(Napi::Env env, Napi::Object exports) {
    return SearchEngineWrapper::Init(env, exports);
}

NODE_API_MODULE(search_addon, InitAll)
