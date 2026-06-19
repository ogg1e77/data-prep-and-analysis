#include "CameraProvider.hpp"
#include <iostream>

CameraProvider::CameraProvider(int deviceIndex) {
    capture_.open(deviceIndex);
    if (!capture_.isOpened()) {
        std::cerr << "[CameraProvider] Не вдалося відкрити камеру з індексом "
                  << deviceIndex << std::endl;
    }
}

CameraProvider::~CameraProvider() {
    if (capture_.isOpened()) {
        capture_.release();
    }
}

bool CameraProvider::isOpened() const {
    return capture_.isOpened();
}

bool CameraProvider::getFrame(cv::Mat& outFrame) {
    if (!capture_.isOpened()) {
        return false;
    }
    capture_ >> outFrame;
    return !outFrame.empty();
}

int CameraProvider::getWidth() const {
    return static_cast<int>(capture_.get(cv::CAP_PROP_FRAME_WIDTH));
}

int CameraProvider::getHeight() const {
    return static_cast<int>(capture_.get(cv::CAP_PROP_FRAME_HEIGHT));
}

double CameraProvider::getFps() const {
    return capture_.get(cv::CAP_PROP_FPS);
}
