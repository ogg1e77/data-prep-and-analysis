#pragma once

#include <opencv2/opencv.hpp>
#include "KeyProcessor.hpp"
#include <chrono>
#include <vector>

/**
 * FrameProcessor — здійснює обробку зображення відповідно до поточного
 * режиму (Mode), а також керує мишею (малювання фігур, зум колесом)
 * та слайдером яскравості.
 */
class FrameProcessor {
public:
    FrameProcessor();

    // Основна обробка кадру відповідно до режиму та параметрів трансформації
    cv::Mat process(const cv::Mat& input, const KeyProcessor& keys);

    // ── Обробники миші (підключаються через cv::setMouseCallback) ──
    static void mouseCallback(int event, int x, int y, int flags, void* userdata);
    void handleMouse(int event, int x, int y, int flags);

    // ── Trackbar (слайдер яскравості) ──
    int brightness = 50;   // 0..100, керується через cv::createTrackbar

    // Завантаження PNG-зображення для picture-in-picture
    bool loadPipImage(const std::string& path);

private:
    // ── Методи обробки за режимами ──
    cv::Mat applyInvert(const cv::Mat& frame) const;
    cv::Mat applyBlur(const cv::Mat& frame) const;
    cv::Mat applyCanny(const cv::Mat& frame) const;
    cv::Mat applySobel(const cv::Mat& frame) const;
    cv::Mat applyBinarize(const cv::Mat& frame) const;
    cv::Mat applyGlitch(const cv::Mat& frame) const;
    cv::Mat applyPip(const cv::Mat& frame, int x, int y) const;

    cv::Mat applyTransform(const cv::Mat& frame, int offsetX, int offsetY,
                            double zoom, double rotationDeg) const;
    cv::Mat applyBrightness(const cv::Mat& frame) const;
    void drawOverlayInfo(cv::Mat& frame, Mode mode);
    void drawShapes(cv::Mat& frame) const;

    // ── Стан для малювання мишкою ──
    bool isDrawing_;
    cv::Point startPoint_;
    cv::Point currentPoint_;
    bool drawRectangleMode_;  // true = прямокутник, false = лінія
    std::vector<std::pair<cv::Point, cv::Point>> shapes_;  // завершені фігури

    // ── Зум колесом миші ──
    double mouseZoom_;

    // ── PNG для picture-in-picture ──
    cv::Mat pipImage_;
    bool hasPipImage_;

    // ── FPS-рахунок ──
    std::chrono::steady_clock::time_point lastTime_;
    double fps_;
    long frameCounter_;
};
