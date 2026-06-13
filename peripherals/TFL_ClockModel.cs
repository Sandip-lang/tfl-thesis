// TFL_ClockModel.cs
// Temporal Fidelity Layer - Clock Model (Source S3)
// Renode version: 1.15.3
// Author: Sandip Kumar Mourya

using System;
using Antmicro.Renode.Core;
using Antmicro.Renode.Core.Structure.Registers;
using Antmicro.Renode.Logging;
using Antmicro.Renode.Peripherals.Bus;
using Antmicro.Renode.Time;
using Antmicro.Renode.Utilities;

namespace Antmicro.Renode.Peripherals.Timers
{
    public class TFL_ClockModel : BasicDoubleWordPeripheral, IKnownSize
    {
        public double DriftPpm { get; set; } = 0.0;
        public double JitterNs { get; set; } = 0.0;
        public long   Size     => 0x20;

        private readonly ulong cpuFrequencyHz;
        private readonly PseudorandomNumberGenerator rng;
        private uint  ctrlShadow = 0;
        private uint  loadShadow = 0;
        private ulong readCount  = 0;
        private const ulong LogEveryNReads = 10000;

        private enum Registers : long
        {
            CTRL    = 0x00,
            LOAD    = 0x04,
            CURRENT = 0x08,
            CALIB   = 0x0C
        }

        public TFL_ClockModel(IMachine machine, ulong frequency = 72_000_000)
            : base(machine)
        {
            cpuFrequencyHz = frequency;
            rng = EmulationManager.Instance.CurrentEmulation.RandomGenerator;
            DefineRegisters();
            this.Log(LogLevel.Info,
                "TFL_ClockModel init: freq={0}Hz DriftPpm={1} JitterNs={2}",
                cpuFrequencyHz, DriftPpm, JitterNs);
        }

        private void DefineRegisters()
        {
            Registers.CTRL.Define(this)
                .WithValueField(0, 32,
                    writeCallback:         (_, v) => { ctrlShadow = (uint)v; },
                    valueProviderCallback: _ => ctrlShadow,
                    name: "CTRL");

            Registers.LOAD.Define(this)
                .WithValueField(0, 32,
                    writeCallback:         (_, v) => { loadShadow = (uint)v; },
                    valueProviderCallback: _ => loadShadow,
                    name: "LOAD");

            Registers.CURRENT.Define(this)
                .WithValueField(0, 32,
                    FieldMode.Read,
                    valueProviderCallback: _ => (uint)GetAdjustedCounterValue(),
                    name: "CURRENT");

            Registers.CALIB.Define(this)
                .WithValueField(0, 32,
                    FieldMode.Read,
                    valueProviderCallback: _ => (uint)(cpuFrequencyHz / 100),
                    name: "CALIB");
        }

        private ulong GetAdjustedCounterValue()
        {
            var virtualTimeNs = ((Machine)machine).ClockSource.CurrentValue
                                                  .TotalNanoseconds;
            var cTrue = (ulong)(virtualTimeNs
                                * (double)cpuFrequencyHz
                                / 1_000_000_000.0);

            var driftCycles = (long)(cTrue * DriftPpm / 1_000_000.0);

            long jitterCycles = 0;
            if (JitterNs > 0.0)
            {
                double u1 = Math.Max(1e-10, rng.NextDouble());
                double u2 = rng.NextDouble();
                double z  = Math.Sqrt(-2.0 * Math.Log(u1))
                            * Math.Cos(2.0 * Math.PI * u2);
                jitterCycles = (long)(z * JitterNs
                                        * (double)cpuFrequencyHz
                                        / 1_000_000_000.0);
            }

            long adjusted = (long)cTrue + driftCycles + jitterCycles;
            if (adjusted < 0) adjusted = 0;

            ulong result = (ulong)adjusted;
            if (loadShadow > 0)
                result = result % ((ulong)loadShadow + 1);

            readCount++;
            if (readCount % LogEveryNReads == 0)
                this.Log(LogLevel.Debug,
                    "TFL_ClockModel #{0}: true={1} drift={2} jitter={3} result={4}",
                    readCount, cTrue, driftCycles, jitterCycles, result);

            return result;
        }

        public override void Reset()
        {
            base.Reset();
            ctrlShadow = 0;
            loadShadow = 0;
            readCount  = 0;
            this.Log(LogLevel.Info, "TFL_ClockModel reset.");
        }
    }
}
