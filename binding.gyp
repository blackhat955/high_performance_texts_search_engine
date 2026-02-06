{
  "targets": [
    {
      "target_name": "search",
      "sources": [ 
        "src/addon.cpp",
        "src/SearchEngineWrapper.cpp",
        "src/SearchEngine.cpp"
      ],
      "include_dirs": [
        "<!@(node -p \"require('node-addon-api').include\")",
        "src"
      ],
      "defines": [ "NAPI_CPP_EXCEPTIONS" ],
      "cflags": [ "-fexceptions", "-std=c++17" ],
      "cflags_cc": [ "-std=c++17", "-fexceptions" ],
      "conditions": [
        ['OS=="mac"', {
          "xcode_settings": {
            "GCC_ENABLE_CPP_EXCEPTIONS": "YES",
            "CLANG_CXX_LIBRARY": "libc++",
            "MACOSX_DEPLOYMENT_TARGET": "10.15",
            "OTHER_CPLUSPLUSFLAGS": ["-std=c++17"],
            "OTHER_LDFLAGS": []
          }
        }],
        ['OS=="linux"', {
           "cflags_cc": [],
           "ldflags": []
        }]
      ]
    }
  ]
}
