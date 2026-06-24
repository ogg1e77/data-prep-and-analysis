#!/usr/bin/env bash
# build.sh — компіляція проєкту за допомогою CMake + make.
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

echo "=== Білд Lab 6 (OpenCV) ==="

mkdir -p build
cd build

cmake -DCMAKE_BUILD_TYPE=Release ..
make -j"$(nproc)"

echo "=== Білд завершено успішно ==="
echo "Виконуваний файл: build/lab6_opencv"
