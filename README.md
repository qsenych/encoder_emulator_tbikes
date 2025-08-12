# Encoder Emulator for ThunderBikes

A quick-turn solution to emulate Hall A/B/C encoder signals from a sin/cos encoder, built when our motor controller only accepted Hall signals, but our motor didn’t have them installed.

---

## Background & Purpose

We had a tight competition deadline and couldn't modify the motor (warranty and rules constraints), and couldn’t afford a new motor controller at the time. Instead, I built this emulator to trick the motor controller into calibrating using signals derived from our existing sin/cos encoder, letting the motor at least spin and get tested before proper hardware arrived.

---

## Hardware & Firmware Overview

- **Platform**: STM32 “Bluepill” development board mounted on a protoboard.
- **Signal Conditioning**: Sin/cos input routed through a 1.6 kΩ / 5.5 kΩ voltage divider, filtered with 20 nF capacitance.
- **Output Stage**: Open-collector FET circuit interfaced with 12 V to mimic expected hardware behavior for Hall signals.
- **Signal Logic**: A six-state, hardcoded lookup table generates Hall A/B/C outputs based on rotor position — repeating every 360 electrical degrees (i.e., every 36 mechanical degrees for our 10 pole-pair motor).

