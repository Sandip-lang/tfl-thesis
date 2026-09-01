import sys, re, csv, os
import numpy as np

def extract_samples(filepath, section):
    samples = []
    in_block = False
    try:
        with open(filepath) as f:
            for line in f:
                line = line.strip()
                if f"{section}_SAMPLES_START" in line:
                    in_block = True; continue
                if f"{section}_SAMPLES_END" in line:
                    in_block = False; continue
                if in_block:
                    try: samples.append(int(line))
                    except ValueError: pass
    except FileNotFoundError:
        print(f"WARNING: {filepath} not found")
    return np.array(samples)

def process(hw_log, base_log, tfl_log):
    sources = ['S2', 'S3', 'S4']
    os.makedirs('measurements', exist_ok=True)
    for src in sources:
        hw   = extract_samples(hw_log,   src)
        base = extract_samples(base_log, src)
        tfl  = extract_samples(tfl_log,  src)
        n = min(len(hw), len(base), len(tfl))
        if n == 0:
            print(f"{src}: no samples found"); continue
        with open(f'measurements/{src.lower()}_comparison.csv', 'w',
                  newline='') as f:
            writer = csv.writer(f)
            writer.writerow(['idx', 'hardware', 'baseline', 'tfl'])
            for i in range(n):
                writer.writerow([i, hw[i], base[i], tfl[i]])
        print(f"{src}: {n} samples written")

if __name__ == '__main__':
    if len(sys.argv) != 4:
        print("Usage: parse_logs.py <hw_log> <baseline_log> <tfl_log>")
        sys.exit(1)
    process(sys.argv[1], sys.argv[2], sys.argv[3])
