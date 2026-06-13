# Temporal Fidelity Layer (TFL) — MSc Thesis Implementation

**Author:** Sandip Kumar Mourya
**Title:** Temporal Accuracy in Rehosted Firmware
**Renode version:** 1.15.3 (pinned)

## Overview

Three-component Renode peripheral extension addressing temporal
inaccuracies in architecture-agnostic firmware rehosting:

| Component | File | Source |
|---|---|---|
| Clock Model | peripherals/TFL_ClockModel.cs | S3: clock drift/jitter |
| Deadline Monitor | peripherals/TFL_DeadlineMonitor.cs | S4: task timing |
| Bus Delay Model | peripherals/TFL_BusDelayModel.cs | S5: propagation delay |

## Reproduce Results

### 1. Install Renode 1.15.3
Download from: https://github.com/renode/renode/releases/tag/v1.15.3
Use the Windows installer: renode-1.15.3.msi

### 2. Run baseline
renode scripts\baseline.resc

### 3. Run TFL enhanced
renode scripts\tfl_full.resc

### 4. Analyse
python analysis\parse_logs.py measurements\hardware\uart.log measurements\baseline_renode\uart.log measurements\tfl_full\uart.log
python analysis\compute_tfi.py
python analysis\plot_distributions.py

## Hardware
- Board: NUCLEO-F103RB
- Logic analyser: Saleae Logic 8
- PA5 (CN10 pin 11) -> analyser CH0
- PA6 (CN10 pin 13) -> analyser CH1
- GND (CN10 pin 20) -> analyser GND
