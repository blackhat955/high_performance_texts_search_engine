#!/bin/bash
set -e

echo "Installing dependencies..."
npm install

echo "Building C++ Addon (Release)..."
npx node-gyp rebuild

echo "Build complete."
