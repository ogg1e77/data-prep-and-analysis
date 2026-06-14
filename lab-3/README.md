# Лабораторна робота №3 — Візуалізація даних

## Опис завдання

Візуалізація датасету **Heart Disease (UCI)** — медичні показники 303 пацієнтів для діагностики серцевих захворювань.

**Характеристики датасету:**
- Dataset Characteristics: Multivariate
- Attribute Characteristics: Categorical, Integer, Real
- Number of Attributes: 13 + target
- Has Missing Values: Yes

## Графіки (8 штук)

| # | Тип | Що показує |
|---|-----|-----------|
| 1 | Scatter + trend line | Залежність ЧСС від віку |
| 2 | Histogram | Розподіл холестерину по 5 діапазонах |
| 3 | Violin Plot | Розподіл віку за статтю та захворюванням |
| 4 | Heatmap | Кореляційна матриця атрибутів |
| 5 | Pair Plot | Матриця парних залежностей |
| 6 | Box Plot | Тиск за типом болю у грудях |
| 7 | Stacked Bar | Частка захворювання за віком та статтю |
| 8 | Line Plot | Тренди ЧСС та холестерину за віком |

## Вимоги до системи

- **Python:** 3.10+
- **ОС:** Windows / macOS / Linux

## Встановлення та запуск

```bash
# 1. Створити віртуальне середовище
python -m venv venv

# Windows
venv\Scripts\activate
# macOS / Linux
source venv/bin/activate

# 2. Встановити залежності
pip install -r requirements.txt

# 3. Запустити Jupyter
jupyter notebook
```

Відкрити `lab3_visualization.ipynb` → **Kernel → Restart & Run All**

## Отримання датасету

Датасет завантажується **автоматично** з UCI Repository при першому запуску ноутбука. Вручну нічого робити не потрібно.
