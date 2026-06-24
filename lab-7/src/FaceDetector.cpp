#include "FaceDetector.hpp"

#include <algorithm>
#include <chrono>
#include <iostream>

FaceDetector::FaceDetector(const std::string& protoPath,
                           const std::string& weightsPath,
                           float minConfidence)
    : minConfidence_(minConfidence),
      active_(true),
      ready_(false),
      frameReady_(false) {
    try {
        network_ = cv::dnn::readNetFromCaffe(protoPath, weightsPath);
        ready_ = !network_.empty();
    } catch (const cv::Exception& error) {
        std::cerr << "Could not load face detection model: " << error.what() << '\n';
        ready_ = false;
    }

    if (ready_) {
        thread_ = std::thread(&FaceDetector::run, this);
    }
}

FaceDetector::~FaceDetector() {
    active_ = false;
    signal_.notify_all();

    if (thread_.joinable()) {
        thread_.join();
    }
}

bool FaceDetector::ready() const {
    return ready_;
}

void FaceDetector::pushFrame(const cv::Mat& frame) {
    if (!ready_ || frame.empty()) {
        return;
    }

    {
        std::lock_guard<std::mutex> guard(lock_);
        queuedFrame_ = frame.clone();
        frameReady_ = true;
    }
    signal_.notify_one();
}

std::vector<cv::Rect> FaceDetector::faces() const {
    std::lock_guard<std::mutex> guard(lock_);
    return result_;
}

void FaceDetector::run() {
    while (active_) {
        cv::Mat current;

        {
            std::unique_lock<std::mutex> guard(lock_);
            signal_.wait(guard, [this] { return frameReady_ || !active_; });

            if (!active_) {
                break;
            }

            current = queuedFrame_.clone();
            frameReady_ = false;
        }

        // Щоб на відео показати різницю з однопотоковим режимом,
        // можна штучно сповільнити інференс, розкоментувавши рядок.
        // Відео при цьому залишається плавним.
        // std::this_thread::sleep_for(std::chrono::milliseconds(500));

        auto found = infer(current);

        {
            std::lock_guard<std::mutex> guard(lock_);
            result_ = std::move(found);
        }
    }
}

std::vector<cv::Rect> FaceDetector::infer(const cv::Mat& frame) {
    std::vector<cv::Rect> boxes;

    if (frame.empty()) {
        return boxes;
    }

    cv::Mat blob = cv::dnn::blobFromImage(frame, 1.0, cv::Size(300, 300),
                                          cv::Scalar(104.0, 177.0, 123.0));
    network_.setInput(blob);

    cv::Mat output = network_.forward();
    cv::Mat data(output.size[2], output.size[3], CV_32F, output.ptr<float>());

    const int width = frame.cols;
    const int height = frame.rows;

    for (int row = 0; row < data.rows; ++row) {
        const float score = data.at<float>(row, 2);

        if (score < minConfidence_) {
            continue;
        }

        int left = static_cast<int>(data.at<float>(row, 3) * width);
        int top = static_cast<int>(data.at<float>(row, 4) * height);
        int right = static_cast<int>(data.at<float>(row, 5) * width);
        int bottom = static_cast<int>(data.at<float>(row, 6) * height);

        left = std::clamp(left, 0, width - 1);
        top = std::clamp(top, 0, height - 1);
        right = std::clamp(right, 0, width - 1);
        bottom = std::clamp(bottom, 0, height - 1);

        if (right > left && bottom > top) {
            boxes.emplace_back(cv::Point(left, top), cv::Point(right, bottom));
        }
    }

    return boxes;
}
