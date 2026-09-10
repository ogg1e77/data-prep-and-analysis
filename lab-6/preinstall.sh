#!/usr/bin/env bash
# preinstall.sh — встановлення всіх залежностей, необхідних для білда і запуску.
set -e

echo "=== Встановлення залежностей для Lab 6 (OpenCV) ==="

sudo apt update
sudo apt install -y libopencv-dev cmake build-essential pkg-config

echo "=== Встановлення завершено ==="
echo "Версія OpenCV:"
pkg-config --modversion opencv4 || pkg-config --modversion opencv || true
