#!/bin/bash
echo "=== GDB/LLDB Wrapper ==="
echo "Note: On macOS, 'lldb' is the native debugger used instead of 'gdb'."
echo "Starting Node.js server under LLDB..."
echo "Type 'run' to start the program once LLDB loads."
echo "Type 'bt' to see backtrace if it crashes."
echo "========================"

lldb -- node server.js
