#!/usr/bin/env bash
# run.sh — запуск зібраної програми.
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

if [ ! -f "build/lab6_opencv" ]; then
    echo "Виконуваний файл не знайдено. Спочатку запустіть ./build.sh"
    exit 1
fi

./build/lab6_opencv
