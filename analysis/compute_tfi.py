import pandas as pd
import numpy as np
import csv, os

sources = {
    's2': 'S2 - Interrupt latency',
    's3': 'S3 - SysTick period',
    's4': 'S4 - MMIO read latency',
}

rows = []
for key, label in sources.items():
    path = f'measurements/{key}_comparison.csv'
    if not os.path.exists(path):
        print(f"Missing: {path}"); continue
    df   = pd.read_csv(path)
    hw   = df['hardware'].values
    base = df['baseline'].values
    tfl  = df['tfl'].values
    err_base = abs(np.mean(base) - np.mean(hw))
    err_tfl  = abs(np.mean(tfl)  - np.mean(hw))
    tfi      = 1 - (err_tfl / err_base) if err_base > 0 else 0.0
    rows.append({'Source': label,
                 'HW mean': f'{np.mean(hw):.1f}',
                 'Baseline error': f'{err_base:.1f}',
                 'TFL error': f'{err_tfl:.1f}',
                 'TFI': f'{tfi:.3f}'})
    print(f"{label}: TFI={tfi:.3f} ({tfi*100:.1f}% improvement)")

if rows:
    with open('analysis/tfi_summary.csv', 'w', newline='') as f:
        writer = csv.DictWriter(f, fieldnames=rows[0].keys())
        writer.writeheader()
        writer.writerows(rows)
    print("Summary: analysis/tfi_summary.csv")
