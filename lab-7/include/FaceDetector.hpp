#pragma once

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <opencv2/dnn.hpp>
#include <opencv2/opencv.hpp>
#include <string>
#include <thread>
#include <vector>

// Виконує детекцію облич у фоновому потоці, не блокуючи відображення відео.
class FaceDetector {
public:
    FaceDetector(const std::string& protoPath,
                 const std::string& weightsPath,
                 float minConfidence = 0.5F);
    ~FaceDetector();

    FaceDetector(const FaceDetector&) = delete;
    FaceDetector& operator=(const FaceDetector&) = delete;

    bool ready() const;
    void pushFrame(const cv::Mat& frame);
    std::vector<cv::Rect> faces() const;

private:
    void run();
    std::vector<cv::Rect> infer(const cv::Mat& frame);

    cv::dnn::Net network_;
    float minConfidence_;

    std::thread thread_;
    mutable std::mutex lock_;
    std::condition_variable signal_;
    std::atomic<bool> active_;
    bool ready_;
    bool frameReady_;

    cv::Mat queuedFrame_;
    std::vector<cv::Rect> result_;
};
