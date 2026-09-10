#include <opencv2/opencv.hpp>
#include <iostream>

#include "CameraProvider.hpp"
#include "KeyProcessor.hpp"
#include "FrameProcessor.hpp"
#include "Display.hpp"

int main(int argc, char** argv) {
    const std::string windowName = "Lab 6 — OpenCV";

    CameraProvider camera(0);
    if (!camera.isOpened()) {
        std::cerr << "Не вдалося відкрити камеру. Перевірте підключення пристрою." << std::endl;
        return 1;
    }

    KeyProcessor keyProcessor;
    FrameProcessor frameProcessor;
    Display display(windowName);

    // Підключаємо обробник миші
    cv::setMouseCallback(windowName, FrameProcessor::mouseCallback, &frameProcessor);

    // Підключаємо слайдер яскравості
    display.attachBrightnessTrackbar(&frameProcessor.brightness);

    // Спроба завантажити зображення для picture-in-picture (необов'язково)
    frameProcessor.loadPipImage("assets/overlay.png");

    std::cout << "Програма запущена. Натисніть клавіші 0-7 для перемикання режимів, "
              << "q або ESC для виходу." << std::endl;

    cv::Mat frame, output;
    while (true) {
        if (!camera.getFrame(frame)) {
            std::cerr << "Не вдалося отримати кадр з камери." << std::endl;
            break;
        }

        output = frameProcessor.process(frame, keyProcessor);
        display.show(output);

        int key = cv::waitKey(1);
        if (key != -1) {
            if (!keyProcessor.processKey(key)) {
                break;
            }
        }
    }

    cv::destroyAllWindows();
    return 0;
}
