#!/bin/bash
set -e

# Colors
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m'

echo -e "${GREEN}=== Advanced Debug & Performance Monitor ===${NC}"

# 1. Compilation
echo -e "${YELLOW}Step 1: Compiling with Debug Flags (-g) and Warnings (-Wall -Wextra)...${NC}"
# We modified binding.gyp to include these flags.
# Rebuild in Debug mode
npx node-gyp rebuild --debug

# 2. AddressSanitizer Setup (Replaces Valgrind on macOS for better compat)
echo -e "${YELLOW}Step 2: Configuring Memory Protection (AddressSanitizer)...${NC}"
# Try to find the ASan library
CLANG_RES=$(clang --print-resource-dir 2>/dev/null || echo "")
ASAN_LIB=""

if [ -n "$CLANG_RES" ]; then
    ASAN_LIB="$CLANG_RES/lib/darwin/libclang_rt.asan_osx_dynamic.dylib"
fi

# Fallback paths if not found
if [ ! -f "$ASAN_LIB" ]; then
    ASAN_LIB=$(find /usr/lib/clang /Library/Developer/CommandLineTools -name "libclang_rt.asan_osx_dynamic.dylib" 2>/dev/null | head -n 1)
fi

if [ -f "$ASAN_LIB" ]; then
    echo -e "${GREEN}✔ AddressSanitizer found: $ASAN_LIB${NC}"
    export DYLD_INSERT_LIBRARIES="$ASAN_LIB"
    echo "  -> Detection Enabled: Dangling pointers, Buffer overflows, Use-after-free."
else
    echo -e "${RED}✘ AddressSanitizer not found automatically.${NC}"
    echo "  -> Continuing with standard Debug build."
fi

# 3. Execution
echo -e "${YELLOW}Step 3: Starting Real-Time Monitor...${NC}"
echo "   - This will start the server and run benchmarks."
echo "   - Watch the 'RAM' and 'Status' indicators."
echo "   - Any Segmentation Faults will be caught and printed."
echo ""

node scripts/monitor.js
