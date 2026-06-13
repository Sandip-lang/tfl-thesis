// TFL_BusDelayModel.cs
// Temporal Fidelity Layer - Bus Delay Model (Source S5)
// Renode version: 1.15.3
// Author: Sandip Kumar Mourya

using System;
using System.Collections.Generic;
using Antmicro.Renode.Core;
using Antmicro.Renode.Core.Structure.Registers;
using Antmicro.Renode.Logging;
using Antmicro.Renode.Peripherals.Bus;
using Antmicro.Renode.Time;
using Antmicro.Renode.Utilities;

namespace Antmicro.Renode.Peripherals.Miscellaneous
{
    public class TFL_BusDelayModel : BasicDoubleWordPeripheral, IKnownSize
    {
        public ulong  PropagationDelayNs  { get; set; } = 86_806;
        public double ArbitrationJitterNs { get; set; } = 0.0;
        public int    MaxQueueDepth       { get; set; } = 64;
        public long   Size                => 0x20;

        private struct PendingMessage
        {
            public ulong        Value;
            public TimeInterval DeliveryTime;
        }

        private readonly Queue<PendingMessage>       queue = new Queue<PendingMessage>();
        private readonly PseudorandomNumberGenerator rng;

        private ulong lastDelivered  = 0;
        private ulong totalSent      = 0;
        private ulong totalDelivered = 0;
        private ulong totalDropped   = 0;

        private enum Registers : long
        {
            SEND       = 0x00,
            RECV       = 0x04,
            STATUS     = 0x08,
            DELAY_CFG  = 0x0C,
            STATS_SENT = 0x10,
            STATS_DLVR = 0x14,
            STATS_DROP = 0x18
        }

        public TFL_BusDelayModel(IMachine machine) : base(machine)
        {
            rng = EmulationManager.Instance.CurrentEmulation.RandomGenerator;
            DefineRegisters();
            this.Log(LogLevel.Info,
                "TFL_BusDelayModel init: delay={0}ns jitter={1}ns",
                PropagationDelayNs, ArbitrationJitterNs);
        }

        private void DefineRegisters()
        {
            Registers.SEND.Define(this)
                .WithValueField(0, 32, FieldMode.Write,
                    writeCallback: (_, v) => Enqueue((ulong)v),
                    name: "SEND");

            Registers.RECV.Define(this)
                .WithValueField(0, 32, FieldMode.Read,
                    valueProviderCallback: _ => (uint)DequeueIfReady(),
                    name: "RECV");

            Registers.STATUS.Define(this)
                .WithFlag(0, FieldMode.Read,
                    valueProviderCallback: _ => queue.Count > 0,
                    name: "PENDING")
                .WithValueField(1, 31, FieldMode.Read,
                    valueProviderCallback: _ => (uint)queue.Count,
                    name: "DEPTH");

            Registers.DELAY_CFG.Define(this)
                .WithValueField(0, 32,
                    writeCallback:
                        (_, v) => { PropagationDelayNs = (ulong)v; },
                    valueProviderCallback:
                        _ => (uint)PropagationDelayNs,
                    name: "DELAY_CFG");

            Registers.STATS_SENT.Define(this)
                .WithValueField(0, 32, FieldMode.Read,
                    valueProviderCallback: _ => (uint)totalSent,
                    name: "SENT");
            Registers.STATS_DLVR.Define(this)
                .WithValueField(0, 32, FieldMode.Read,
                    valueProviderCallback: _ => (uint)totalDelivered,
                    name: "DLVR");
            Registers.STATS_DROP.Define(this)
                .WithValueField(0, 32, FieldMode.Read,
                    valueProviderCallback: _ => (uint)totalDropped,
                    name: "DROP");
        }

        private void Enqueue(ulong value)
        {
            if (queue.Count >= MaxQueueDepth)
            {
                totalDropped++;
                this.Log(LogLevel.Warning,
                    "TFL_BusDelayModel: queue full, dropping message.");
                return;
            }

            double jitterNs = 0.0;
            if (ArbitrationJitterNs > 0.0)
            {
                double u1 = Math.Max(1e-10, rng.NextDouble());
                double u2 = rng.NextDouble();
                double z  = Math.Sqrt(-2.0 * Math.Log(u1))
                            * Math.Cos(2.0 * Math.PI * u2);
                jitterNs = Math.Abs(z * ArbitrationJitterNs);
            }

            ulong totalNs    = PropagationDelayNs + (ulong)jitterNs;
            var   delivery   = ((Machine)machine).ClockSource.CurrentValue
                             + TimeInterval.FromNanoseconds(totalNs);

            queue.Enqueue(new PendingMessage
                { Value = value, DeliveryTime = delivery });
            totalSent++;

            this.Log(LogLevel.Debug,
                "TFL_BusDelayModel: queued val={0} delay={1}ns",
                value, totalNs);
        }

        private ulong DequeueIfReady()
        {
            var now = ((Machine)machine).ClockSource.CurrentValue;
            while (queue.Count > 0 && now >= queue.Peek().DeliveryTime)
            {
                lastDelivered = queue.Dequeue().Value;
                totalDelivered++;
                this.Log(LogLevel.Debug,
                    "TFL_BusDelayModel: delivered val={0}", lastDelivered);
            }
            return lastDelivered;
        }

        public override void Reset()
        {
            base.Reset();
            this.Log(LogLevel.Info,
                "TFL_BusDelayModel reset: sent={0} delivered={1} dropped={2}",
                totalSent, totalDelivered, totalDropped);
            queue.Clear();
            lastDelivered  = 0;
            totalSent      = 0;
            totalDelivered = 0;
            totalDropped   = 0;
        }
    }
}
