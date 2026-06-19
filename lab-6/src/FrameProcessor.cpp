#include "FrameProcessor.hpp"
#include <algorithm>
#include <iostream>
#include <sstream>

FrameProcessor::FrameProcessor()
    : isDrawing_(false),
      drawRectangleMode_(true),
      mouseZoom_(1.0),
      hasPipImage_(false),
      fps_(0.0),
      frameCounter_(0) {
    lastTime_ = std::chrono::steady_clock::now();
}

bool FrameProcessor::loadPipImage(const std::string& path) {
    pipImage_ = cv::imread(path, cv::IMREAD_UNCHANGED);
    hasPipImage_ = !pipImage_.empty();
    if (!hasPipImage_) {
        std::cerr << "[FrameProcessor] Не вдалося завантажити PiP-зображення: "
                  << path << std::endl;
    }
    return hasPipImage_;
}

// ═══════════════════════════════ MOUSE ═══════════════════════════════

void FrameProcessor::mouseCallback(int event, int x, int y, int flags, void* userdata) {
    auto* self = static_cast<FrameProcessor*>(userdata);
    if (self) {
        self->handleMouse(event, x, y, flags);
    }
}

void FrameProcessor::handleMouse(int event, int x, int y, int flags) {
    switch (event) {
        case cv::EVENT_LBUTTONDOWN:
            isDrawing_ = true;
            startPoint_ = cv::Point(x, y);
            currentPoint_ = startPoint_;
            break;

        case cv::EVENT_MOUSEMOVE:
            if (isDrawing_) {
                currentPoint_ = cv::Point(x, y);
            }
            break;

        case cv::EVENT_LBUTTONUP:
            if (isDrawing_) {
                currentPoint_ = cv::Point(x, y);
                shapes_.emplace_back(startPoint_, currentPoint_);
                isDrawing_ = false;
            }
            break;

        case cv::EVENT_RBUTTONDOWN:
            // ПКМ перемикає тип фігури: прямокутник <-> лінія
            drawRectangleMode_ = !drawRectangleMode_;
            break;

        case cv::EVENT_MOUSEWHEEL: {
            int delta = cv::getMouseWheelDelta(flags);
            if (delta > 0) {
                mouseZoom_ = std::min(mouseZoom_ + 0.1, 4.0);
            } else {
                mouseZoom_ = std::max(mouseZoom_ - 0.1, 0.2);
            }
            break;
        }

        default:
            break;
    }
}

void FrameProcessor::drawShapes(cv::Mat& frame) const {
    // Завершені фігури
    for (const auto& shape : shapes_) {
        if (drawRectangleMode_) {
            cv::rectangle(frame, shape.first, shape.second, cv::Scalar(0, 255, 0), 2);
        } else {
            cv::line(frame, shape.first, shape.second, cv::Scalar(0, 255, 255), 2);
        }
    }
    // Фігура, що зараз малюється
    if (isDrawing_) {
        if (drawRectangleMode_) {
            cv::rectangle(frame, startPoint_, currentPoint_, cv::Scalar(0, 200, 0), 1);
        } else {
            cv::line(frame, startPoint_, currentPoint_, cv::Scalar(0, 200, 200), 1);
        }
    }
}

// ═══════════════════════════════ ФІЛЬТРИ ═══════════════════════════════

cv::Mat FrameProcessor::applyInvert(const cv::Mat& frame) const {
    cv::Mat result;
    cv::bitwise_not(frame, result);
    return result;
}

cv::Mat FrameProcessor::applyBlur(const cv::Mat& frame) const {
    cv::Mat result;
    cv::GaussianBlur(frame, result, cv::Size(15, 15), 0);
    return result;
}

cv::Mat FrameProcessor::applyCanny(const cv::Mat& frame) const {
    cv::Mat gray, edges, result;
    cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
    cv::Canny(gray, edges, 80, 160);
    cv::cvtColor(edges, result, cv::COLOR_GRAY2BGR);
    return result;
}

cv::Mat FrameProcessor::applySobel(const cv::Mat& frame) const {
    cv::Mat gray, gradX, gradY, absGradX, absGradY, grad, result;
    cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
    cv::Sobel(gray, gradX, CV_16S, 1, 0);
    cv::Sobel(gray, gradY, CV_16S, 0, 1);
    cv::convertScaleAbs(gradX, absGradX);
    cv::convertScaleAbs(gradY, absGradY);
    cv::addWeighted(absGradX, 0.5, absGradY, 0.5, 0, grad);
    cv::cvtColor(grad, result, cv::COLOR_GRAY2BGR);
    return result;
}

cv::Mat FrameProcessor::applyBinarize(const cv::Mat& frame) const {
    cv::Mat gray, binary, result;
    cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
    cv::threshold(gray, binary, 127, 255, cv::THRESH_BINARY);
    cv::cvtColor(binary, result, cv::COLOR_GRAY2BGR);
    return result;
}

cv::Mat FrameProcessor::applyGlitch(const cv::Mat& frame) const {
    std::vector<cv::Mat> channels;
    cv::split(frame, channels);  // B, G, R

    cv::Mat shiftedB, shiftedR;
    cv::Mat translationB = (cv::Mat_<double>(2, 3) << 1, 0, -15, 0, 1, 0);
    cv::Mat translationR = (cv::Mat_<double>(2, 3) << 1, 0, 15, 0, 1, 0);
    cv::warpAffine(channels[0], shiftedB, translationB, channels[0].size());
    cv::warpAffine(channels[2], shiftedR, translationR, channels[2].size());

    std::vector<cv::Mat> glitchChannels = {shiftedB, channels[1], shiftedR};
    cv::Mat result;
    cv::merge(glitchChannels, result);
    return result;
}

cv::Mat FrameProcessor::applyPip(const cv::Mat& frame, int x, int y) const {
    cv::Mat result = frame.clone();
    if (!hasPipImage_) return result;

    int pipW = std::min(160, frame.cols / 4);
    int pipH = std::min(120, frame.rows / 4);
    cv::Mat resizedPip;
    cv::resize(pipImage_, resizedPip, cv::Size(pipW, pipH));

    int clampedX = std::max(0, std::min(x, frame.cols - pipW));
    int clampedY = std::max(0, std::min(y, frame.rows - pipH));

    cv::Rect roi(clampedX, clampedY, pipW, pipH);

    if (resizedPip.channels() == 4) {
        // Альфа-блендінг для PNG з прозорістю
        for (int row = 0; row < pipH; ++row) {
            for (int col = 0; col < pipW; ++col) {
                cv::Vec4b src = resizedPip.at<cv::Vec4b>(row, col);
                double alpha = src[3] / 255.0;
                cv::Vec3b& dst = result.at<cv::Vec3b>(clampedY + row, clampedX + col);
                for (int c = 0; c < 3; ++c) {
                    dst[c] = static_cast<uchar>(src[c] * alpha + dst[c] * (1 - alpha));
                }
            }
        }
    } else {
        cv::Mat bgrPip;
        if (resizedPip.channels() == 1) {
            cv::cvtColor(resizedPip, bgrPip, cv::COLOR_GRAY2BGR);
        } else {
            bgrPip = resizedPip;
        }
        bgrPip.copyTo(result(roi));
    }

    cv::rectangle(result, roi, cv::Scalar(255, 255, 255), 1);
    return result;
}

cv::Mat FrameProcessor::applyTransform(const cv::Mat& frame, int offsetX, int offsetY,
                                       double zoom, double rotationDeg) const {
    cv::Point2f center(frame.cols / 2.0f, frame.rows / 2.0f);
    cv::Mat rotMatrix = cv::getRotationMatrix2D(center, rotationDeg, zoom * mouseZoom_);

    // Додаємо зсув (translation) до матриці трансформації
    rotMatrix.at<double>(0, 2) += offsetX;
    rotMatrix.at<double>(1, 2) += offsetY;

    cv::Mat result;
    cv::warpAffine(frame, result, rotMatrix, frame.size(),
                    cv::INTER_LINEAR, cv::BORDER_CONSTANT, cv::Scalar(0, 0, 0));
    return result;
}

cv::Mat FrameProcessor::applyBrightness(const cv::Mat& frame) const {
    // brightness: 0..100, 50 = нейтрально
    double beta = (brightness - 50) * 2.0;  // -100..+100
    cv::Mat result;
    frame.convertTo(result, -1, 1.0, beta);
    return result;
}

void FrameProcessor::drawOverlayInfo(cv::Mat& frame, Mode mode) {
    frameCounter_++;

    auto now = std::chrono::steady_clock::now();
    double elapsed = std::chrono::duration<double>(now - lastTime_).count();
    if (elapsed >= 0.5) {
        fps_ = frameCounter_ / elapsed;
        frameCounter_ = 0;
        lastTime_ = now;
    }

    // Середня інтенсивність по каналах
    cv::Scalar meanIntensity = cv::mean(frame);

    static const char* modeNames[] = {
        "NORMAL", "INVERT", "BLUR", "CANNY", "SOBEL", "BINARIZE", "GLITCH", "PIP"
    };

    std::ostringstream oss;
    oss << "Mode: " << modeNames[static_cast<int>(mode)]
        << " | FPS: " << static_cast<int>(fps_);

    std::ostringstream oss2;
    oss2 << "Mean B/G/R: "
         << static_cast<int>(meanIntensity[0]) << "/"
         << static_cast<int>(meanIntensity[1]) << "/"
         << static_cast<int>(meanIntensity[2]);

    cv::putText(frame, oss.str(), cv::Point(10, 25),
                cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0, 255, 0), 2);
    cv::putText(frame, oss2.str(), cv::Point(10, 50),
                cv::FONT_HERSHEY_SIMPLEX, 0.55, cv::Scalar(0, 255, 0), 2);
    cv::putText(frame, "Keys: 0-7 modes | arrows/WASD move | +/- zoom | ,/. rotate | r reset | q quit",
                cv::Point(10, frame.rows - 12),
                cv::FONT_HERSHEY_SIMPLEX, 0.42, cv::Scalar(200, 200, 200), 1);
}

// ═══════════════════════════════ MAIN PROCESS ═══════════════════════════════

cv::Mat FrameProcessor::process(const cv::Mat& input, const KeyProcessor& keys) {
    cv::Mat frame = input.clone();

    // 1. Геометричні трансформації (зум/обертання/зсув) — для всіх режимів окрім PIP
    Mode mode = keys.getMode();
    if (mode != Mode::PIP) {
        frame = applyTransform(frame, keys.getOffsetX(), keys.getOffsetY(),
                                keys.getZoom(), keys.getRotationDeg());
    }

    // 2. Яскравість (керується слайдером)
    frame = applyBrightness(frame);

    // 3. Фільтр відповідно до режиму
    switch (mode) {
        case Mode::NORMAL:   break;
        case Mode::INVERT:   frame = applyInvert(frame);   break;
        case Mode::BLUR:     frame = applyBlur(frame);     break;
        case Mode::CANNY:    frame = applyCanny(frame);    break;
        case Mode::SOBEL:    frame = applySobel(frame);    break;
        case Mode::BINARIZE: frame = applyBinarize(frame); break;
        case Mode::GLITCH:   frame = applyGlitch(frame);   break;
        case Mode::PIP:      frame = applyPip(frame, keys.getPipX(), keys.getPipY()); break;
    }

    // 4. Малювання фігур мишкою (завжди зверху)
    drawShapes(frame);

    // 5. Текстова інформація (режим, FPS, середня інтенсивність)
    drawOverlayInfo(frame, mode);

    return frame;
}
