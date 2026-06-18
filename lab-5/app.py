"""
Лабораторна робота №5 — Наука про дані: обмін результатами та початковий аналіз

Веб-додаток на Streamlit для аналізу VHI/VCI/TCI-даних по регіонах України.
Запуск: streamlit run app.py
"""

import os
import re
import urllib.request
from datetime import datetime

import numpy as np
import pandas as pd
import streamlit as st
import matplotlib.pyplot as plt

# ═══════════════════════════════════════════════════════════════
# Конфігурація сторінки
# ═══════════════════════════════════════════════════════════════
st.set_page_config(
    page_title="VHI / VCI / TCI — аналіз даних",
    page_icon="🌾",
    layout="wide",
)

DATA_DIR = "vhi_data"
BASE_URL = (
    "https://www.star.nesdis.noaa.gov/smcd/emb/vci/VH/get_TS_admin.php"
    "?country=UKR&provinceID={pid}&year1=1981&year2=2024&type=Mean"
)

NOAA_ID_TO_NAME = {
    1: "Cherkasy", 2: "Chernihiv", 3: "Chernivtsi", 4: "Crimea",
    5: "Dnipropetrovsk", 6: "Donetsk", 7: "Ivano-Frankivsk", 8: "Kharkiv",
    9: "Kherson", 10: "Khmelnytskyi", 11: "Kirovohrad", 12: "Kyiv",
    13: "Kyiv City", 14: "Luhansk", 15: "Lviv", 16: "Mykolaiv",
    17: "Odessa", 18: "Poltava", 19: "Rivne", 20: "Sumy",
    21: "Ternopil", 22: "Vinnytsia", 23: "Volyn", 24: "Zakarpattia",
    25: "Zaporizhzhia", 26: "Zhytomyr", 27: "Sevastopol",
}

UKR_ALPHA_ORDER = [
    (1, "Вінницька", 22), (2, "Волинська", 23), (3, "Дніпропетровська", 5),
    (4, "Донецька", 6), (5, "Житомирська", 26), (6, "Закарпатська", 24),
    (7, "Запорізька", 25), (8, "Івано-Франківська", 7), (9, "Київська", 12),
    (10, "Кіровоградська", 11), (11, "Луганська", 14), (12, "Львівська", 15),
    (13, "Миколаївська", 16), (14, "Одеська", 17), (15, "Полтавська", 18),
    (16, "Рівненська", 19), (17, "Сумська", 20), (18, "Тернопільська", 21),
    (19, "Харківська", 8), (20, "Херсонська", 9), (21, "Хмельницька", 10),
    (22, "Черкаська", 1), (23, "Чернівецька", 3), (24, "Чернігівська", 2),
    (25, "Крим", 4), (26, "м. Київ", 13), (27, "м. Севастополь", 27),
]
NOAA_TO_UKR_IDX = {noaa: ukr for ukr, _, noaa in UKR_ALPHA_ORDER}
NOAA_TO_UKR_NAME = {noaa: name for _, name, noaa in UKR_ALPHA_ORDER}
UKR_NAME_TO_ID = {name: ukr for ukr, name, _ in UKR_ALPHA_ORDER}

INDEX_OPTIONS = ["VCI", "TCI", "VHI"]


# ═══════════════════════════════════════════════════════════════
# Завантаження та обробка даних (з кешуванням)
# ═══════════════════════════════════════════════════════════════
def already_downloaded(province_id: int) -> bool:
    if not os.path.exists(DATA_DIR):
        return False
    pattern = re.compile(rf"^vhi_province_{province_id}_\d{{8}}_\d{{6}}\.csv$")
    return any(pattern.match(f) for f in os.listdir(DATA_DIR))


def download_vhi(province_id: int) -> None:
    if already_downloaded(province_id):
        return
    os.makedirs(DATA_DIR, exist_ok=True)
    url = BASE_URL.format(pid=province_id)
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    filename = f"vhi_province_{province_id}_{timestamp}.csv"
    filepath = os.path.join(DATA_DIR, filename)
    req = urllib.request.Request(url, headers={"User-Agent": "Mozilla/5.0"})
    try:
        with urllib.request.urlopen(req, timeout=15) as response:
            with open(filepath, "wb") as f:
                f.write(response.read())
    except Exception as e:
        st.warning(f"Не вдалося завантажити область {province_id}: {e}")


def read_vhi_file(filepath: str, noaa_id: int) -> pd.DataFrame:
    with open(filepath, "r", encoding="utf-8", errors="ignore") as f:
        raw = f.read()
    lines = [l.strip() for l in raw.splitlines() if l.strip() and not l.strip().startswith("<")]
    from io import StringIO
    df = pd.read_csv(StringIO("\n".join(lines)), header=0,
                     skipinitialspace=True, on_bad_lines="skip")
    df.columns = [c.strip().lower().replace(" ", "_") for c in df.columns]
    if "vhi" in df.columns:
        df = df[df["vhi"] != -1]
    num_cols = df.select_dtypes(include="number").columns
    df[num_cols] = df[num_cols].fillna(df[num_cols].median())
    df["noaa_province_id"] = noaa_id
    return df.reset_index(drop=True)


@st.cache_data(show_spinner="Завантаження та підготовка даних...")
def load_data() -> pd.DataFrame:
    for pid in range(1, 28):
        download_vhi(pid)

    dfs = []
    pattern = re.compile(r"vhi_province_(\d+)_")
    if os.path.exists(DATA_DIR):
        for fname in sorted(os.listdir(DATA_DIR)):
            m = pattern.match(fname)
            if not m:
                continue
            noaa_id = int(m.group(1))
            try:
                dfs.append(read_vhi_file(os.path.join(DATA_DIR, fname), noaa_id))
            except Exception:
                continue

    if not dfs:
        # Запасний варіант: синтетичні дані, якщо NOAA недоступний
        return generate_fallback_data()

    df = pd.concat(dfs, ignore_index=True)
    df["ukr_province_id"] = df["noaa_province_id"].map(NOAA_TO_UKR_IDX)
    df["province_name_ukr"] = df["noaa_province_id"].map(NOAA_TO_UKR_NAME)

    required = {"year", "week", "vci", "tci", "vhi"}
    if not required.issubset(df.columns):
        return generate_fallback_data()

    return df


def generate_fallback_data() -> pd.DataFrame:
    """Синтетичні дані на випадок, якщо NOAA недоступний (наприклад, з хмарного сервера)."""
    rng = np.random.default_rng(42)
    rows = []
    for ukr_id, name, noaa_id in UKR_ALPHA_ORDER:
        for year in range(1981, 2025):
            for week in range(1, 53):
                vci = round(float(np.clip(rng.normal(45, 20), 0, 100)), 2)
                tci = round(float(np.clip(rng.normal(50, 18), 0, 100)), 2)
                vhi = round((vci + tci) / 2, 2)
                rows.append({
                    "year": year, "week": week,
                    "vci": vci, "tci": tci, "vhi": vhi,
                    "noaa_province_id": noaa_id,
                    "ukr_province_id": ukr_id,
                    "province_name_ukr": name,
                })
    return pd.DataFrame(rows)


# ═══════════════════════════════════════════════════════════════
# Завантаження даних
# ═══════════════════════════════════════════════════════════════
df = load_data()

MIN_YEAR, MAX_YEAR = int(df["year"].min()), int(df["year"].max())
MIN_WEEK, MAX_WEEK = int(df["week"].min()), int(df["week"].max())
PROVINCE_NAMES = sorted(df["province_name_ukr"].dropna().unique().tolist())

DEFAULTS = {
    "index": "VHI",
    "province": PROVINCE_NAMES[0] if PROVINCE_NAMES else "Київська",
    "week_range": (MIN_WEEK, MAX_WEEK),
    "year_range": (MIN_YEAR, MAX_YEAR),
    "sort_asc": False,
    "sort_desc": False,
}

# ── Ініціалізація session_state ───────────────────────────────
for key, val in DEFAULTS.items():
    if key not in st.session_state:
        st.session_state[key] = val


def reset_filters():
    for key, val in DEFAULTS.items():
        st.session_state[key] = val


# ═══════════════════════════════════════════════════════════════
# Layout: дві колонки
# ═══════════════════════════════════════════════════════════════
st.title("🌾 Аналіз VCI / TCI / VHI по регіонах України")

col_controls, col_content = st.columns([1, 3], gap="large")

# ── Колонка з інтерактивними елементами ────────────────────────
with col_controls:
    st.subheader("Фільтри")

    selected_index = st.selectbox(
        "Часовий ряд",
        options=INDEX_OPTIONS,
        key="index",
    )

    selected_province = st.selectbox(
        "Область",
        options=PROVINCE_NAMES,
        key="province",
    )

    week_range = st.slider(
        "Інтервал тижнів",
        min_value=MIN_WEEK, max_value=MAX_WEEK,
        key="week_range",
    )

    year_range = st.slider(
        "Інтервал років",
        min_value=MIN_YEAR, max_value=MAX_YEAR,
        key="year_range",
    )

    st.divider()
    st.caption("Сортування за обраним індексом")
    sort_asc = st.checkbox("Сортувати за зростанням", key="sort_asc")
    sort_desc = st.checkbox("Сортувати за спаданням", key="sort_desc")

    if sort_asc and sort_desc:
        st.warning("Обрано обидва сортування одночасно — застосовано сортування за зростанням (пріоритет).")

    st.divider()
    st.button("🔄 Reset", on_click=reset_filters, use_container_width=True)

# ── Фільтрація даних ────────────────────────────────────────────
index_col = selected_index.lower()

filtered = df[
    (df["province_name_ukr"] == selected_province)
    & df["week"].between(week_range[0], week_range[1])
    & df["year"].between(year_range[0], year_range[1])
].copy()

filtered = filtered.sort_values(["year", "week"]).reset_index(drop=True)

if sort_asc and not sort_desc:
    filtered = filtered.sort_values(index_col, ascending=True).reset_index(drop=True)
elif sort_desc and not sort_asc:
    filtered = filtered.sort_values(index_col, ascending=False).reset_index(drop=True)
elif sort_asc and sort_desc:
    filtered = filtered.sort_values(index_col, ascending=True).reset_index(drop=True)

# ── Колонка з вкладками (таблиця + графіки) ────────────────────
with col_content:
    tab_table, tab_plot, tab_compare = st.tabs(
        ["📋 Таблиця", "📈 Графік", "🌍 Порівняння областей"]
    )

    # ── Вкладка 1: Таблиця ──────────────────────────────────────
    with tab_table:
        st.subheader(f"Відфільтровані дані — {selected_province}")
        st.caption(
            f"Роки: {year_range[0]}–{year_range[1]} | "
            f"Тижні: {week_range[0]}–{week_range[1]} | "
            f"Записів: {len(filtered)}"
        )
        display_cols = ["year", "week", "vci", "tci", "vhi"]
        st.dataframe(filtered[display_cols], use_container_width=True, height=480)

    # ── Вкладка 2: Графік відфільтрованих даних ─────────────────
    with tab_plot:
        st.subheader(f"{selected_index} — {selected_province}")
        if filtered.empty:
            st.info("Немає даних для обраних фільтрів.")
        else:
            fig, ax = plt.subplots(figsize=(10, 5))
            plot_data = filtered.sort_values(["year", "week"])
            x_labels = plot_data["year"].astype(str) + "-W" + plot_data["week"].astype(str)
            ax.plot(range(len(plot_data)), plot_data[index_col], color="#2E7D32", linewidth=1.3)
            ax.set_title(f"{selected_index} для області «{selected_province}»")
            ax.set_xlabel("Тиждень (послідовно у вибраному діапазоні)")
            ax.set_ylabel(selected_index)
            ax.grid(alpha=0.3)

            # Підписи осі X — рідше, щоб не злипались
            step = max(len(plot_data) // 12, 1)
            ax.set_xticks(range(0, len(plot_data), step))
            ax.set_xticklabels(x_labels.iloc[::step], rotation=45, ha="right")

            plt.tight_layout()
            st.pyplot(fig)

    # ── Вкладка 3: Порівняння з іншими областями ────────────────
    with tab_compare:
        st.subheader(f"Порівняння середнього {selected_index} між областями")
        st.caption(f"Період: {year_range[0]}–{year_range[1]}, тижні {week_range[0]}–{week_range[1]}")

        compare_subset = df[
            df["week"].between(week_range[0], week_range[1])
            & df["year"].between(year_range[0], year_range[1])
        ]

        province_means = (
            compare_subset.groupby("province_name_ukr")[index_col]
            .mean()
            .sort_values(ascending=False)
        )

        if province_means.empty:
            st.info("Немає даних для побудови порівняння.")
        else:
            colors = [
                "#D32F2F" if name == selected_province else "#90A4AE"
                for name in province_means.index
            ]
            fig2, ax2 = plt.subplots(figsize=(10, 9))
            ax2.barh(province_means.index, province_means.values, color=colors)
            ax2.set_xlabel(f"Середнє значення {selected_index}")
            ax2.set_title(f"Порівняння областей за {selected_index} (обрана область — червоним)")
            ax2.invert_yaxis()
            ax2.grid(axis="x", alpha=0.3)
            plt.tight_layout()
            st.pyplot(fig2)

st.divider()
st.caption(
    "Лабораторна робота №5 — Наука про дані: обмін результатами та початковий аналіз. "
    "Дані: NOAA STAR VHI (VCI / TCI / VHI), область даних — лабораторна робота №2."
)
