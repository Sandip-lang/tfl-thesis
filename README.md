# Temporal Fidelity Layer (TFL) — MSc Thesis Implementation

**Author:** Sandip Kumar Mourya
**Title:** Temporal Accuracy in Rehosted Firmware
**Institution:** Saarland University
**Renode version:** 1.15.3 (pinned — see VERSIONS.txt)

---

## Repository Structure
tfl-thesis/
├── peripherals/          # TFL C# Renode peripheral extensions
│   ├── TFL_ClockModel.cs         # S3: clock drift and jitter
│   ├── TFL_DeadlineMonitor.cs    # S4: task wakeup deviation
│   └── TFL_BusDelayModel.cs      # S5: inter-node propagation delay
├── platforms/            # Renode .repl platform descriptions
│   ├── nucleo_f103rb_base.repl   # Unmodified baseline
│   └── nucleo_f103rb_tfl.repl    # TFL-enhanced platform
├── scripts/              # Renode .resc execution scripts
│   ├── baseline.resc
│   ├── tfl_full.resc
│   ├── tfl_clock_only.resc
│   ├── tfl_deadline_only.resc
│   └── tfl_bus_only.resc
├── firmware/
│   ├── timing_probe/     # Zephyr RTOS measurement firmware
│   │   ├── CMakeLists.txt
│   │   ├── prj.conf
│   │   ├── nucleo_f103rb.overlay
│   │   └── src/
│   │       ├── main.c            # Entry point, test sequencer
│   │       ├── gpio_markers.c/h  # GPIO timing marker module
│   │       └── timing_tests.c/h  # Ten test scenario implementations
│   └── binaries/         # Built ELF (git-tracked for reproducibility)
├── renode/
│   ├── run_timing_test.resc      # Renode session script
│   └── extract_gpio_timing.py    # GPIO log parser
├── captures/
│   ├── physical/         # Saleae Logic 8 CSV exports (raw data)
│   └── virtual/          # Renode GPIO timing CSVs
├── analysis/
│   ├── full_analysis.py          # Complete pipeline: parse+stats+plot
│   ├── parse_logs.py             # UART log parser (TFL evaluation)
│   ├── compute_tfi.py            # TFI metric computation
│   ├── plot_distributions.py     # TFL comparison plots
│   ├── requirements.txt          # Python dependencies
│   └── figures/                  # Generated PDF/PNG figures
└── docs/                 # Thesis chapters
## TFL Components

| Component | File | Addresses |
|---|---|---|
| Clock Model | peripherals/TFL_ClockModel.cs | S3: clock drift/jitter |
| Deadline Monitor | peripherals/TFL_DeadlineMonitor.cs | S4: task timing |
| Bus Delay Model | peripherals/TFL_BusDelayModel.cs | S5: propagation delay |

## Reproduce Results

### 1. Install tools (Windows)

`powershell
# Renode 1.15.3 installer from:
# https://github.com/renode/renode/releases/tag/v1.15.3

# Python dependencies
pip install -r analysis/requirements.txt
`

### 2. Build firmware (requires Zephyr SDK 0.16.8)

`ash
cd firmware/timing_probe
west build -p always -b nucleo_f103rb . -- -DDTC_OVERLAY_FILE=nucleo_f103rb.overlay
cp build/zephyr/zephyr.elf ../../firmware/binaries/timing_probe_nucleo_f103rb.elf
`

### 3. Flash to hardware

`ash
west flash --build-dir build
`

### 4. Run baseline Renode
renode scripts\baseline.resc

### 5. Run TFL-enhanced Renode
renode scripts\tfl_full.resc

### 6. Run experimental analysis

`powershell
python analysis\full_analysis.py
`

### 7. Run TFI evaluation

`powershell
python analysis\parse_logs.py measurements\hardware\uart.log measurements\baseline_renode\uart.log measurements\tfl_full\uart.log
python analysis\compute_tfi.py
python analysis\plot_distributions.py
`

## Hardware Setup

- Board: NUCLEO-F103RB (STM32F103RB, Cortex-M3 @ 72 MHz)
- Logic analyser: Saleae Logic 8 (100 MS/s, 4 channels)

| Signal | Nucleo Pin | Analyser CH |
|---|---|---|
| Marker 0 (primary) | PA0 (CN7 pin 28) | CH0 |
| Marker 1 (envelope) | PA1 (CN7 pin 30) | CH1 |
| Marker 2 (sub-test) | PA4 (CN7 pin 32) | CH2 |
| Marker 3 (load) | PA8 (CN10 pin 23) | CH3 |
| GND | CN7 pin 20 | GND |

## License

MIT
