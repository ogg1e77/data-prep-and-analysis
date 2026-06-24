#include "Display.hpp"

Display::Display(const std::string& windowName) : windowName_(windowName) {
    cv::namedWindow(windowName_, cv::WINDOW_AUTOSIZE);
}

void Display::show(const cv::Mat& frame) {
    if (!frame.empty()) {
        cv::imshow(windowName_, frame);
    }
}

void Display::attachBrightnessTrackbar(int* brightnessValue) {
    cv::createTrackbar("Brightness", windowName_, brightnessValue, 100);
}
