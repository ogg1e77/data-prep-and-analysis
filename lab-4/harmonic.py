"""
Лабораторна робота №4 — Візуалізація даних 2
Інтерактивний графік гармоніки з шумом та фільтрацією

Інструкція для користувача:
─────────────────────────────────────────────────────────────
 Слайдери:
   • Amplitude        — амплітуда гармоніки (0.1 – 5.0)
   • Frequency        — частота гармоніки   (0.1 – 5.0 Гц)
   • Phase            — фазовий зсув        (0 – 2π)
   • Noise Mean       — середнє шуму        (-1.0 – 1.0)
   • Noise Covariance — дисперсія шуму      (0.01 – 2.0)
   • Cutoff Frequency — частота зрізу фільтра (0.1 – 10.0 Гц)

 Чекбокси:
   • Show Noise     — показувати/приховувати шум
   • Show Filtered  — показувати/приховувати відфільтровану гармоніку

 Кнопка Reset — відновлює початкові значення всіх параметрів

 Примітка:
   • Зміна параметрів гармоніки НЕ перегенеровує шум
   • Зміна параметрів шуму перегенеровує ЛИШЕ шум
─────────────────────────────────────────────────────────────
"""

import numpy as np
import matplotlib.pyplot as plt
import matplotlib.gridspec as gridspec
from matplotlib.widgets import Slider, Button, CheckButtons
from scipy.signal import butter, filtfilt

# ── Початкові параметри ───────────────────────────────────
DEFAULTS = {
    "amplitude":        1.0,
    "frequency":        1.0,
    "phase":            0.0,
    "noise_mean":       0.0,
    "noise_covariance": 0.1,
    "cutoff":           5.0,
}

# ── Часовий вектор ────────────────────────────────────────
FS   = 500          # частота дискретизації (Гц)
T    = 10.0         # тривалість (с)
t    = np.linspace(0, T, int(FS * T), endpoint=False)

# ── Глобальний кеш шуму ───────────────────────────────────
_noise_cache = {
    "mean": DEFAULTS["noise_mean"],
    "cov":  DEFAULTS["noise_covariance"],
    "data": None,
}


def generate_noise(mean: float, cov: float) -> np.ndarray:
    """Генерує та кешує шум. Перегенерує лише якщо параметри змінились."""
    if (_noise_cache["data"] is None
            or _noise_cache["mean"] != mean
            or _noise_cache["cov"]  != cov):
        _noise_cache["mean"] = mean
        _noise_cache["cov"]  = cov
        _noise_cache["data"] = np.random.normal(mean, np.sqrt(abs(cov)), size=len(t))
    return _noise_cache["data"]


def harmonic_with_noise(amplitude: float, frequency: float, phase: float,
                        noise_mean: float, noise_covariance: float,
                        show_noise: bool) -> tuple[np.ndarray, np.ndarray]:
    """
    Повертає (clean, noisy) — чисту та зашумлену гармоніку.
    Якщо show_noise=False, noisy == clean.
    """
    clean = amplitude * np.sin(2 * np.pi * frequency * t + phase)
    noise = generate_noise(noise_mean, noise_covariance)
    noisy = clean + noise if show_noise else clean
    return clean, noisy


def apply_filter(signal: np.ndarray, cutoff: float) -> np.ndarray:
    """Butterworth low-pass фільтр 4-го порядку."""
    nyq    = FS / 2
    cutoff = np.clip(cutoff, 0.1, nyq - 1)
    b, a   = butter(4, cutoff / nyq, btype="low")
    return filtfilt(b, a, signal)


# ── Побудова вікна ────────────────────────────────────────
fig = plt.figure(figsize=(13, 9))
fig.patch.set_facecolor("#1e1e2e")

gs = gridspec.GridSpec(
    2, 1,
    figure=fig,
    top=0.94, bottom=0.42,
    hspace=0.35,
)

ax_main   = fig.add_subplot(gs[0])   # зашумлена + чиста
ax_filter = fig.add_subplot(gs[1])   # відфільтрована vs чиста

for ax in (ax_main, ax_filter):
    ax.set_facecolor("#13131f")
    ax.tick_params(colors="#cdd6f4")
    ax.xaxis.label.set_color("#cdd6f4")
    ax.yaxis.label.set_color("#cdd6f4")
    ax.title.set_color("#cdd6f4")
    for spine in ax.spines.values():
        spine.set_edgecolor("#45475a")

ax_main.set_title("Гармоніка з шумом")
ax_main.set_xlabel("Час (с)")
ax_main.set_ylabel("Амплітуда")

ax_filter.set_title("Відфільтрована гармоніка vs Чиста")
ax_filter.set_xlabel("Час (с)")
ax_filter.set_ylabel("Амплітуда")

# Початкові криві
p = DEFAULTS.copy()
clean0, noisy0 = harmonic_with_noise(
    p["amplitude"], p["frequency"], p["phase"],
    p["noise_mean"], p["noise_covariance"], show_noise=True,
)
filtered0 = apply_filter(noisy0, p["cutoff"])

line_noisy,    = ax_main.plot(t, noisy0,    color="#fab387", lw=1.0,  alpha=0.7, label="Зашумлена")
line_clean,    = ax_main.plot(t, clean0,    color="#89b4fa", lw=2.0,  label="Чиста")
line_filtered, = ax_filter.plot(t, filtered0, color="#a6e3a1", lw=2.0, label="Відфільтрована")
line_clean2,   = ax_filter.plot(t, clean0,    color="#89b4fa", lw=1.5, linestyle="--", label="Чиста (еталон)")

ax_main.legend(facecolor="#313244", labelcolor="#cdd6f4", loc="upper right")
ax_filter.legend(facecolor="#313244", labelcolor="#cdd6f4", loc="upper right")

# ── Слайдери ─────────────────────────────────────────────
slider_color   = "#313244"
slider_style   = {"color": "#89b4fa"}

def make_slider(left, bottom, label, valmin, valmax, valinit, valstep=None):
    ax_s = fig.add_axes([left, bottom, 0.55, 0.022],
                        facecolor=slider_color)
    kw = dict(label=label, valmin=valmin, valmax=valmax, valinit=valinit,
              color="#89b4fa")
    if valstep:
        kw["valstep"] = valstep
    s = Slider(ax_s, **kw)
    s.label.set_color("#cdd6f4")
    s.valtext.set_color("#cdd6f4")
    return s

sliders = {
    "amplitude":        make_slider(0.12, 0.385, "Amplitude",        0.1,  5.0,  DEFAULTS["amplitude"],        0.01),
    "frequency":        make_slider(0.12, 0.355, "Frequency",        0.1,  5.0,  DEFAULTS["frequency"],        0.01),
    "phase":            make_slider(0.12, 0.325, "Phase",            0.0,  2*np.pi, DEFAULTS["phase"],         0.01),
    "noise_mean":       make_slider(0.12, 0.295, "Noise Mean",      -1.0,  1.0,  DEFAULTS["noise_mean"],       0.01),
    "noise_covariance": make_slider(0.12, 0.265, "Noise Covariance", 0.01, 2.0,  DEFAULTS["noise_covariance"], 0.01),
    "cutoff":           make_slider(0.12, 0.235, "Cutoff Frequency", 0.1, 10.0,  DEFAULTS["cutoff"],           0.1),
}

# ── Чекбокси ─────────────────────────────────────────────
ax_check = fig.add_axes([0.76, 0.225, 0.18, 0.09], facecolor=slider_color)
check = CheckButtons(ax_check, ["Show Noise", "Show Filtered"], [True, True])
for txt in check.labels:
    txt.set_color("#cdd6f4")
for rect in check.rectangles:
    rect.set_edgecolor("#cdd6f4")

# ── Кнопка Reset ─────────────────────────────────────────
ax_btn = fig.add_axes([0.12, 0.185, 0.10, 0.033], facecolor="#f38ba8")
btn_reset = Button(ax_btn, "Reset", color="#f38ba8", hovercolor="#eba0ac")
btn_reset.label.set_color("#1e1e2e")
btn_reset.label.set_fontweight("bold")

# ── Інструкція ────────────────────────────────────────────
fig.text(0.12, 0.165,
         "Слайдери: Amplitude, Frequency, Phase — параметри гармоніки  |  "
         "Noise Mean/Covariance — параметри шуму  |  "
         "Cutoff — частота зрізу фільтра  |  Reset — скинути все",
         color="#6c7086", fontsize=8, wrap=True)

# ── Стан чекбоксів ────────────────────────────────────────
state = {"show_noise": True, "show_filtered": True}


# ── Оновлення графіка ─────────────────────────────────────
def update(_=None):
    amp   = sliders["amplitude"].val
    freq  = sliders["frequency"].val
    phase = sliders["phase"].val
    n_mean = sliders["noise_mean"].val
    n_cov  = sliders["noise_covariance"].val
    cutoff = sliders["cutoff"].val

    show_noise     = state["show_noise"]
    show_filtered  = state["show_filtered"]

    clean, noisy = harmonic_with_noise(amp, freq, phase, n_mean, n_cov, show_noise)
    filtered     = apply_filter(noisy, cutoff)

    line_clean.set_ydata(clean)
    line_noisy.set_ydata(noisy)
    line_noisy.set_visible(show_noise)

    line_clean2.set_ydata(clean)
    line_filtered.set_ydata(filtered)
    line_filtered.set_visible(show_filtered)

    for ax in (ax_main, ax_filter):
        ax.relim()
        ax.autoscale_view()

    fig.canvas.draw_idle()


def on_check(label):
    if label == "Show Noise":
        state["show_noise"] = not state["show_noise"]
    elif label == "Show Filtered":
        state["show_filtered"] = not state["show_filtered"]
    update()


def on_reset(_):
    # Скидаємо кеш шуму
    _noise_cache["data"] = None
    for key, slider in sliders.items():
        slider.set_val(DEFAULTS[key])
    update()


# Підключаємо обробники
for slider in sliders.values():
    slider.on_changed(update)

check.on_clicked(on_check)
btn_reset.on_clicked(on_reset)

plt.suptitle("Лабораторна робота №4 — Інтерактивна гармоніка",
             color="#cdd6f4", fontsize=13, fontweight="bold", y=0.99)

plt.show()
