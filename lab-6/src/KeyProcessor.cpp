#include "KeyProcessor.hpp"
#include <algorithm>

KeyProcessor::KeyProcessor()
    : mode_(Mode::NORMAL),
      offsetX_(0), offsetY_(0),
      zoom_(1.0), rotationDeg_(0.0),
      pipX_(20), pipY_(20) {}

void KeyProcessor::resetTransform() {
    offsetX_ = 0;
    offsetY_ = 0;
    zoom_ = 1.0;
    rotationDeg_ = 0.0;
    pipX_ = 20;
    pipY_ = 20;
}

bool KeyProcessor::processKey(int key) {
    switch (key) {
        // ── Вихід ────────────────────────────────────────────
        case 27:   // ESC
        case 'q':
        case 'Q':
            return false;

        // ── Перемикання режимів ────────────────────────────────
        case '0': mode_ = Mode::NORMAL;   break;
        case '1': mode_ = Mode::INVERT;   break;
        case '2': mode_ = Mode::BLUR;     break;
        case '3': mode_ = Mode::CANNY;    break;
        case '4': mode_ = Mode::SOBEL;    break;
        case '5': mode_ = Mode::BINARIZE; break;
        case '6': mode_ = Mode::GLITCH;   break;
        case '7': mode_ = Mode::PIP;      break;

        // ── Стрілочки: зсув зображення (працює в режимі NORMAL/будь-якому) ──
        case 81:  // ліва стрілка (Linux/OpenCV HighGUI код)
        case 2424832:
            if (mode_ == Mode::PIP) pipX_ -= 10; else offsetX_ -= 10;
            break;
        case 83:  // права стрілка
        case 2555904:
            if (mode_ == Mode::PIP) pipX_ += 10; else offsetX_ += 10;
            break;
        case 82:  // верх
        case 2490368:
            if (mode_ == Mode::PIP) pipY_ -= 10; else offsetY_ -= 10;
            break;
        case 84:  // низ
        case 2621440:
            if (mode_ == Mode::PIP) pipY_ += 10; else offsetY_ += 10;
            break;

        // ── Зум (+/-) ────────────────────────────────────────
        case '+':
        case '=':
            zoom_ = std::min(zoom_ + 0.1, 4.0);
            break;
        case '-':
        case '_':
            zoom_ = std::max(zoom_ - 0.1, 0.2);
            break;

        // ── Обертання (,/.) ────────────────────────────────────
        case ',':
            rotationDeg_ -= 5.0;
            break;
        case '.':
            rotationDeg_ += 5.0;
            break;

        // ── Reset трансформацій ─────────────────────────────────
        case 'r':
        case 'R':
            resetTransform();
            break;

        default:
            break;
    }
    return true;
}
