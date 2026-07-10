#!/usr/bin/env python3
"""
extract_gpio_timing.py
Parses Renode log output and extracts GPIO state change events
with virtual timestamps into a structured CSV format.

Usage: python3 extract_gpio_timing.py <renode_log> <output_csv>
Author: Sandip Kumar Mourya
"""

import re
import csv
import sys
import os

class RenodeGPIOExtractor:

    GPIO_PATTERN = re.compile(
        r'(\d{2}:\d{2}:\d{2}\.\d+)\s+\[.*\]\s+gpioPortA:\s+Pin\s+(\d+)\s+'
        r'(?:set to|changed to)\s+(True|False|High|Low|1|0)',
        re.IGNORECASE
    )

    def __init__(self, log_path, output_path):
        self.log_path    = log_path
        self.output_path = output_path
        self.events      = []
        self.start_time  = None

    def parse_timestamp(self, s):
        h, m, sec = s.split(':')
        return int(h) * 3600 + int(m) * 60 + float(sec)

    def parse_state(self, s):
        return 1 if s.lower() in ('true', 'high', '1') else 0

    def extract(self):
        with open(self.log_path, 'r') as f:
            for line in f:
                m = self.GPIO_PATTERN.search(line)
                if not m:
                    continue
                t     = self.parse_timestamp(m.group(1))
                pin   = int(m.group(2))
                state = self.parse_state(m.group(3))

                if self.start_time is None:
                    self.start_time = t

                rel = t - self.start_time
                self.events.append({
                    'time_s':  rel,
                    'time_ns': int(rel * 1e9),
                    'pin':     pin,
                    'state':   state,
                })

        print(f"Extracted {len(self.events)} GPIO events from {self.log_path}")
        return self.events

    def save_csv(self):
        if not self.events:
            self.extract()

        os.makedirs(os.path.dirname(self.output_path) or '.', exist_ok=True)

        with open(self.output_path, 'w', newline='') as f:
            writer = csv.writer(f)
            writer.writerow(['time_s', 'time_ns', 'pin', 'state'])
            for e in self.events:
                writer.writerow([
                    f"{e['time_s']:.12f}",
                    e['time_ns'],
                    e['pin'],
                    e['state'],
                ])

        print(f"Saved {len(self.events)} events to {self.output_path}")


if __name__ == '__main__':
    if len(sys.argv) != 3:
        print(f"Usage: {sys.argv[0]} <renode_log> <output_csv>")
        sys.exit(1)

    ex = RenodeGPIOExtractor(sys.argv[1], sys.argv[2])
    ex.extract()
    ex.save_csv()
