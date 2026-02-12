#!/bin/bash
echo "=== Valgrind Wrapper ==="
echo "Starting Node.js server under Valgrind..."
echo "Note: Valgrind support on macOS (especially ARM64) is limited."
echo "If this fails, use 'scripts/advanced_debug.sh' which uses AddressSanitizer."
echo "========================"

valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes node server.js
