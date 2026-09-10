#pragma once

/**
 * Режими обробки зображення, що перемикаються клавішами.
 */
enum class Mode {
    NORMAL,       // 0 — без обробки
    INVERT,       // 1 — інверсія кольорів
    BLUR,         // 2 — Gaussian blur
    CANNY,        // 3 — Canny фільтр
    SOBEL,        // 4 — фільтр Собеля
    BINARIZE,     // 5 — бінаризація
    GLITCH,       // 6 — розсування каналів RGB
    PIP           // 7 — picture-in-picture
};

/**
 * KeyProcessor — обробляє код натиснутої клавіші (результат cv::waitKey)
 * та оновлює поточний режим і допоміжні параметри (зум, зсув, яскравість тощо).
 */
class KeyProcessor {
public:
    KeyProcessor();

    // Обробляє код клавіші, повертає false якщо потрібно завершити програму (ESC/q)
    bool processKey(int key);

    Mode getMode() const { return mode_; }

    // Параметри, що керуються стрілочками / іншими клавішами
    int getOffsetX() const { return offsetX_; }
    int getOffsetY() const { return offsetY_; }
    double getZoom() const { return zoom_; }
    double getRotationDeg() const { return rotationDeg_; }
    int getPipX() const { return pipX_; }
    int getPipY() const { return pipY_; }

    void resetTransform();

private:
    Mode mode_;

    int offsetX_;
    int offsetY_;
    double zoom_;
    double rotationDeg_;

    int pipX_;
    int pipY_;
};
