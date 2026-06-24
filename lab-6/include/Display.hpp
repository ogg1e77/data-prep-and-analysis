#pragma once

#include <opencv2/opencv.hpp>
#include <string>

/**
 * Display — відповідає за відображення кадру у вікні (cv::imshow)
 * та керування слайдером (trackbar) яскравості.
 */
class Display {
public:
    explicit Display(const std::string& windowName);

    void show(const cv::Mat& frame);
    const std::string& getWindowName() const { return windowName_; }

    // Прив'язує trackbar яскравості до зовнішньої змінної
    void attachBrightnessTrackbar(int* brightnessValue);

private:
    std::string windowName_;
};
