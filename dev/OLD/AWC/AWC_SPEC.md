# AWC (Analog WAV Converter) Board Technical Specification

## 1. General Overview
- **System Purpose**: High-fidelity analog signal buffering and impedance conversion for 24-bit audio recorders.
- **Target Recorder**: ZOOM H6 (Supported Sampling Rate: Up to 192kHz / 24-bit)
- **Target Sensor**: Analog Accelerometer (5V drive, 0-5V output format)

## 2. Analog Input Specifications
- **Number of Channels**: 4 Channels (Ch1~Ch3 for 3-axis, Ch4 for AUX)
- **Input Voltage Range**: 0.0V ~ 5.0V (Rail-to-Rail Input)
- **Input Impedance**: Ultra-High Impedance (CMOS Input)
- **Input Bias Current**: 81 pA (Typ.)
- **Sensor Power Supply**: 5.0V DC provided at the input terminal block

## 3. Analog Output Specifications
- **Number of Channels**: 4 Channels
- **Output Format**: Unbalanced Mono (Tip: Signal, Ring/Sleeve: GND)
- **Output Voltage Range**: 0.0V ~ 5.0V (Rail-to-Rail Output)
- **Output Drive Current**: Max 50 mA per channel
- **Phantom Power Protection**: Hardware Grounding (Ring and Sleeve shorted to GND to safely sink 48V phantom power)

## 4. Signal Characteristics (Buffer Stage)
- **Core Amplifier**: TI TLV9064IPWR (Quad, RRIO, CMOS Op-Amp)
- **Circuit Topology**: Voltage Follower (Gain = 1 V/V)
- **Gain Bandwidth Product (GBW)**: 10 MHz
- **Slew Rate**: 6.5 V/µs
- **Input Voltage Noise Density**: 16 nV/√Hz @ 1kHz
- **Common Mode Rejection Ratio (CMRR)**: 87 dB

## 5. Power Supply Specifications
- **Primary Power Source**: 9V DC (006P Battery)
- **Power Switch**: SPDT Slide Switch (Hardware cut-off)
- **Internal Operating Voltage**: 5.0V DC (Regulated)
- **Internal LDO Regulator**: AMS1117-5.0 (Max 1.0A capacity)
- **System Power Indicator**: Red LED (0603 size, approx. 0.9mA current draw for ultra-low power consumption)

## 6. Mechanical & Interface
- **Input Terminal (CN11)**: 3.5mm Pitch, 16-Pin (2x8) Terminal Block
- **Output Terminal (CN12)**: 3.5mm Pitch, 16-Pin (2x8) Terminal Block
- **Mounting Holes**: 4x NPTH (Non-Plated Through Hole) for chassis isolation