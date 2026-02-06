#!/bin/bash
set -e

echo "Building C++ Addon (Debug with ASan)..."
npx node-gyp rebuild --debug

echo "Debug build complete. Run with:"
echo "DYLD_INSERT_LIBRARIES=/usr/lib/clang/lib/darwin/libclang_rt.asan_osx_dynamic.dylib node server.js"
