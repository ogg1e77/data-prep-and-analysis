#pragma once

#include <opencv2/opencv.hpp>
#include <string>

/**
 * CameraProvider — обгортка над cv::VideoCapture.
 * Відповідає лише за отримання кадрів з камери.
 */
class CameraProvider {
public:
    explicit CameraProvider(int deviceIndex = 0);
    ~CameraProvider();

    bool isOpened() const;
    bool getFrame(cv::Mat& outFrame);

    int getWidth() const;
    int getHeight() const;
    double getFps() const;

private:
    cv::VideoCapture capture_;
};
