{
  "targets": [
    {
      "target_name": "search",
      "sources": [ "search.cpp" ],
      "include_dirs": [
        "node_modules/node-addon-api"
      ],
      "defines": [ "NAPI_CPP_EXCEPTIONS" ],
      "cflags": [ "-fexceptions" ],
      "cflags_cc": [ "-std=c++17", "-fexceptions" ],
      "xcode_settings": {
        "GCC_ENABLE_CPP_EXCEPTIONS": "YES",
        "CLANG_CXX_LIBRARY": "libc++",
        "MACOSX_DEPLOYMENT_TARGET": "10.15"
      }
    }
  ]
}
