# Лабораторна робота №2 — Наука про дані: підготовчий етап

## Структура

```
lab-2/
├── part1_vhi.ipynb       # Частина 1: VHI-індекс по регіонах України
├── part2_power.ipynb     # Частина 2: споживання електроенергії
├── requirements.txt      # Залежності
└── vhi_data/             # Створюється автоматично при запуску part1
```

> `vhi_data/` та `household_power_consumption.txt` не зберігаються у репозиторії — завантажуються автоматично при першому запуску ноутбуків.

---

## Запуск

```bash
# 1. Створити та активувати venv
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

Відкрити `part1_vhi.ipynb` або `part2_power.ipynb` і запускати комірки послідовно зверху вниз.  
Дані завантажуються **автоматично** — вручну нічого завантажувати не потрібно.
