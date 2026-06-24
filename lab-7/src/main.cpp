#include <cstdlib>
#include <iostream>
#include <opencv2/opencv.hpp>

#include "CameraProvider.hpp"
#include "Display.hpp"
#include "FaceDetector.hpp"
#include "FrameProcessor.hpp"
#include "KeyProcessor.hpp"
#include "MouseHandler.hpp"

int main(int argc, char** argv) {
    const std::string windowName = "Lab 7";

    // Індекс камери можна передати першим аргументом командного рядка.
    // Це зручно для віртуальних камер на кшталт DroidCam, які часто
    // не є /dev/video0. За замовчуванням 0. Приклад: ./run.sh 2
    int deviceIndex = (argc > 1) ? std::atoi(argv[1]) : 0;

    CameraProvider camera(deviceIndex);
    if (!camera.isOpened()) {
        std::cerr << "Error: camera with index " << deviceIndex
                  << " was not opened.\n"
                  << "Check that a webcam (or DroidCam) is connected and available.\n"
                  << "List available devices with: v4l2-ctl --list-devices\n"
                  << "Then run, for example: ./run.sh 2\n";
        return 1;
    }

    Display display(windowName);
    KeyProcessor keyProcessor;
    FrameProcessor frameProcessor;
    MouseHandler mouseHandler;
    FaceDetector faceDetector("assets/deploy.prototxt",
                              "assets/res10_300x300_ssd_iter_140000.caffemodel");

    if (!faceDetector.ready()) {
        std::cerr << "Warning: face detector model was not loaded. Run ./preinstall.sh first.\n";
    }

    int brightnessValue = frameProcessor.getBrightness();
    cv::createTrackbar("Brightness", windowName, &brightnessValue, 200,
                       FrameProcessor::onBrightnessTrackbar, &frameProcessor);
    cv::setMouseCallback(windowName, MouseHandler::callback, &mouseHandler);

    std::cout << "Controls:\n"
              << "  0 Normal\n"
              << "  1 Invert colors\n"
              << "  2 Gaussian blur\n"
              << "  3 Canny edge detector\n"
              << "  4 Sobel filter\n"
              << "  5 Binary threshold\n"
              << "  6 Quantization\n"
              << "  7 RGB glitch\n"
              << "  8 Picture in picture\n"
              << "  9 Draw rectangles with mouse\n"
              << "  F Face detection with OpenCV DNN in background thread\n"
              << "  A/D rotate, +/- zoom, mouse wheel zoom, I/J/K/L move frame, R reset\n"
              << "  Q or ESC quit\n";

    while (true) {
        cv::Mat frame = camera.getFrame();
        if (frame.empty()) {
            std::cerr << "Warning: empty frame received.\n";
            break;
        }

        const int wheelDelta = mouseHandler.consumeWheelDelta();
        if (wheelDelta > 0) {
            keyProcessor.increaseZoom(0.1);
        } else if (wheelDelta < 0) {
            keyProcessor.decreaseZoom(0.1);
        }

        const bool faceMode = (keyProcessor.getMode() == KeyProcessor::Mode::Face);

        if (faceMode) {
            faceDetector.pushFrame(frame);
        }

        cv::Mat processed = frameProcessor.process(frame, keyProcessor.getMode(),
                                                   keyProcessor, mouseHandler);

        if (faceMode) {
            frameProcessor.drawFaces(processed, faceDetector.faces());
        }

        display.show(processed);

        const int key = cv::waitKey(1);

        if (key >= 0 && ((key & 0xFF) == 'r' || (key & 0xFF) == 'R')) {
            mouseHandler.clear();
        }

        if (!keyProcessor.processKey(key)) {
            break;
        }
    }

    cv::destroyAllWindows();
    return 0;
}
