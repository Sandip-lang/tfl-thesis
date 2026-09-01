import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import os

os.makedirs('analysis/figures', exist_ok=True)

sources = {
    's2': 'S2 - Interrupt latency (cycles)',
    's3': 'S3 - SysTick period (cycles)',
    's4': 'S4 - MMIO read latency (cycles)',
}

fig, axes = plt.subplots(1, 3, figsize=(15, 4))
fig.suptitle('TFL: Timing Distribution Comparison', fontsize=13,
             fontweight='bold')

for ax, (key, label) in zip(axes, sources.items()):
    path = f'measurements/{key}_comparison.csv'
    if not os.path.exists(path):
        ax.text(0.5, 0.5, f'No data', ha='center', va='center',
                transform=ax.transAxes)
        ax.set_title(label); continue
    df   = pd.read_csv(path)
    hw   = df['hardware'].dropna()
    base = df['baseline'].dropna()
    tfl  = df['tfl'].dropna()
    err_base = abs(base.mean() - hw.mean())
    err_tfl  = abs(tfl.mean()  - hw.mean())
    tfi      = 1 - (err_tfl / err_base) if err_base > 0 else 0.0
    ax.hist(hw,   bins=40, alpha=0.65, color='steelblue',
            label=f'Hardware mu={hw.mean():.0f}')
    ax.hist(base, bins=40, alpha=0.65, color='darkorange',
            label=f'Baseline err={err_base:.0f}')
    ax.hist(tfl,  bins=40, alpha=0.65, color='seagreen',
            label=f'TFL err={err_tfl:.0f}')
    ax.set_title(f'{label}\nTFI={tfi:.2f}', fontsize=9)
    ax.set_xlabel('Cycles')
    ax.set_ylabel('Count')
    ax.legend(fontsize=7)

plt.tight_layout()
plt.savefig('analysis/figures/tfl_comparison.pdf', dpi=150,
            bbox_inches='tight')
plt.savefig('analysis/figures/tfl_comparison.png', dpi=150,
            bbox_inches='tight')
print("Saved: analysis/figures/tfl_comparison.pdf")
