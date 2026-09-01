// TFL_DeadlineMonitor.cs
// Temporal Fidelity Layer - Deadline Monitor (Source S4)
// Renode version: 1.15.3
// Author: Sandip Kumar Mourya

using System;
using System.Collections.Generic;
using System.IO;
using Antmicro.Renode.Core;
using Antmicro.Renode.Core.Structure.Registers;
using Antmicro.Renode.Logging;
using Antmicro.Renode.Peripherals.Bus;
using Antmicro.Renode.Time;

namespace Antmicro.Renode.Peripherals.Miscellaneous
{
    public class TFL_DeadlineMonitor : BasicDoubleWordPeripheral, IKnownSize
    {
        public ulong  ExpectedPeriodNs { get; set; } = 1_000_000;
        public string OutputCsvPath    { get; set; } = @"C:\temp\tfl_deadlines.csv";
        public int    MaxSamples       { get; set; } = 10_000;
        public long   Size             => 0x10;

        private readonly List<long> samples = new List<long>();
        private TimeInterval lastMarkTime;
        private bool         firstMark       = true;
        private long         lastDeviationNs = 0;
        private ulong        markCount       = 0;

        private enum Registers : long
        {
            MARK        = 0x00,
            STATUS      = 0x04,
            LAST_DEV_HI = 0x08,
            LAST_DEV_LO = 0x0C
        }

        public TFL_DeadlineMonitor(IMachine machine) : base(machine)
        {
            DefineRegisters();
            this.Log(LogLevel.Info,
                "TFL_DeadlineMonitor init: period={0}ns csv={1}",
                ExpectedPeriodNs, OutputCsvPath);
        }

        private void DefineRegisters()
        {
            Registers.MARK.Define(this)
                .WithValueField(0, 32,
                    FieldMode.Write,
                    writeCallback: (_, __) => RecordWakeup(),
                    name: "MARK");

            Registers.STATUS.Define(this)
                .WithValueField(0, 32,
                    FieldMode.Read,
                    valueProviderCallback: _ => (uint)samples.Count,
                    name: "STATUS");

            Registers.LAST_DEV_HI.Define(this)
                .WithValueField(0, 16,
                    FieldMode.Read,
                    valueProviderCallback: _ =>
                        (uint)((lastDeviationNs >> 16) & 0xFFFF),
                    name: "LAST_DEV_HI");

            Registers.LAST_DEV_LO.Define(this)
                .WithValueField(0, 16,
                    FieldMode.Read,
                    valueProviderCallback: _ =>
                        (uint)(lastDeviationNs & 0xFFFF),
                    name: "LAST_DEV_LO");
        }

        private void RecordWakeup()
        {
            markCount++;
            if (MaxSamples > 0 && samples.Count >= MaxSamples) return;

            var now = ((Machine)machine).ClockSource.CurrentValue;

            if (firstMark)
            {
                lastMarkTime = now;
                firstMark    = false;
                return;
            }

            var actualNs    = (long)(now - lastMarkTime).TotalNanoseconds;
            lastDeviationNs = actualNs - (long)ExpectedPeriodNs;
            samples.Add(lastDeviationNs);
            lastMarkTime = now;

            this.Log(LogLevel.Noisy,
                "TFL_DeadlineMonitor mark #{0}: actual={1}ns dev={2}ns",
                markCount, actualNs, lastDeviationNs);
        }

        private void FlushResults()
        {
            int n = samples.Count;
            if (n == 0) return;

            double mean = 0; long min = long.MaxValue, max = long.MinValue;
            double variance = 0;

            foreach (var s in samples)
            {
                mean += s;
                if (s < min) min = s;
                if (s > max) max = s;
            }
            mean /= n;
            foreach (var s in samples)
                variance += (s - mean) * (s - mean);
            double std = Math.Sqrt(variance / n);

            var sorted = new List<long>(samples); sorted.Sort();
            long p99 = sorted[(int)(n * 0.99)];

            this.Log(LogLevel.Info,
                "TFL_DeadlineMonitor ({0} samples): " +
                "mean={1:F1}ns std={2:F1}ns min={3}ns max={4}ns p99={5}ns",
                n, mean, std, min, max, p99);

            try
            {
                Directory.CreateDirectory(
                    Path.GetDirectoryName(OutputCsvPath));
                using var w = new StreamWriter(OutputCsvPath, append: false);
                w.WriteLine("sample_index,deviation_ns");
                for (int i = 0; i < samples.Count; i++)
                    w.WriteLine($"{i},{samples[i]}");
                this.Log(LogLevel.Info,
                    "TFL_DeadlineMonitor: wrote {0} samples to {1}",
                    n, OutputCsvPath);
            }
            catch (Exception ex)
            {
                this.Log(LogLevel.Error,
                    "TFL_DeadlineMonitor CSV write failed: {0}", ex.Message);
            }
        }

        public override void Reset()
        {
            if (samples.Count > 0) FlushResults();
            base.Reset();
            samples.Clear();
            firstMark       = true;
            lastDeviationNs = 0;
            markCount       = 0;
            this.Log(LogLevel.Info, "TFL_DeadlineMonitor reset.");
        }
    }
}
