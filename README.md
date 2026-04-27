# KISS FFT — Arduino & ESP32 Library

> **K**eep **I**t **S**imple, **S**tupid FFT — a lightweight Fast Fourier Transform for Arduino and ESP32.

[![Arduino Library](https://img.shields.io/badge/Arduino-Library-blue?logo=arduino)](https://github.com/meerzafarnoohani/KISS_FFT)
[![License: MIT](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/platform-ESP32%20%7C%20Arduino-orange)](https://github.com/meerzafarnoohani/KISS_FFT)

---

## What is KISS FFT?

KISS FFT converts a time-domain signal (a series of samples from a microphone, ADC, accelerometer, etc.) into the **frequency domain** — revealing which frequencies are present and how strong each one is.

**Practical uses on ESP32 / Arduino:**
- 🎵 Audio spectrum visualization
- 🎤 Pitch / dominant frequency detection
- 📳 Vibration analysis from accelerometers
- 🔊 Noise filtering and signal processing
- 📡 Wireless signal analysis

This is an Arduino/ESP32 port of the original [KISS FFT](https://github.com/mborgerding/kissfft) C library by Mark Borgerding, wrapped with a clean C++ class designed for embedded use.

---

## Installation

### Via Arduino IDE (recommended)
1. Download the latest `KISS_FFT.zip` from the [Releases](https://github.com/meerzafarnoohani/KISS_FFT/releases) page.
2. Open Arduino IDE → **Sketch → Include Library → Add .ZIP Library...**
3. Select the downloaded zip file.

### Via Arduino Library Manager
Search for **KISS_FFT** in **Tools → Manage Libraries** and click Install.

### Manual
Clone this repository into your Arduino libraries folder:
```bash
cd ~/Arduino/libraries
git clone https://github.com/meerzafarnoohani/KISS_FFT.git
```

---

## Quick Start

```cpp
#include <KISS_FFT_Arduino.h>

const int   FFT_SIZE    = 1024;
const float SAMPLE_RATE = 8000.0f; // Hz

KissFFT fft(FFT_SIZE);
float   samples[FFT_SIZE];

void loop() {
    // Fill samples[] with your ADC / microphone data, then:
    fft.setInput(samples, FFT_SIZE);
    fft.applyWindowHann();          // reduces spectral leakage
    fft.compute();                  // run the FFT

    float* mag  = fft.getMagnitude();            // magnitude per bin
    float  freq = fft.dominantFrequency(SAMPLE_RATE); // dominant Hz
    float  rms  = fft.rms();                     // signal RMS

    Serial.print("Dominant frequency: ");
    Serial.print(freq);
    Serial.println(" Hz");
}
```

---

## API Reference

### Constructor
```cpp
KissFFT fft(int fftSize);
```
Creates an FFT object. Best performance with power-of-2 sizes: `256`, `512`, `1024`, `2048`.

### Loading Input

| Method | Description |
|--------|-------------|
| `setInput(float* samples, int count)` | Load real float samples (normalized, e.g. −1.0 to 1.0) |
| `setInput(int16_t* samples, int count, float scale)` | Load raw ADC integers; divide by `scale` |
| `setInputComplex(kiss_fft_cpx* cpx, int count)` | Load complex input directly |

### Window Functions
Apply **before** `compute()` to reduce spectral leakage.

| Method | Description |
|--------|-------------|
| `applyWindowHann()` | Hann window — best general-purpose choice |
| `applyWindowHamming()` | Hamming window |
| `applyWindowRect()` | No windowing (rectangular) |

### Compute

| Method | Description |
|--------|-------------|
| `compute()` | Run forward FFT |
| `computeInverse()` | Run inverse FFT (auto-normalized) |

### Output & Analysis

| Method | Returns | Description |
|--------|---------|-------------|
| `getMagnitude()` | `float*` | Magnitude spectrum, N/2+1 bins |
| `getOutput()` | `kiss_fft_cpx*` | Raw complex FFT output |
| `dominantFrequency(float sampleRate)` | `float` | Strongest frequency in Hz |
| `dominantBin()` | `int` | Bin index of strongest frequency |
| `binToFrequency(int bin, float sampleRate)` | `float` | Convert bin index → Hz |
| `power(int bin)` | `float` | Power (magnitude²) of a bin |
| `rms()` | `float` | RMS amplitude of the signal |
| `size()` | `int` | FFT size |

---

## Examples

| Example | Description |
|---------|-------------|
| `BasicFFT` | Synthetic 440 Hz + 1 kHz signal — verify the library works without hardware |
| `MicrophoneFFT` | Real-time FFT from an I2S MEMS microphone (INMP441 / SPH0645) on ESP32 |

---

## Memory Usage (ESP32)

| FFT Size | RAM (approx.) |
|----------|--------------|
| 256      | ~6 KB        |
| 512      | ~12 KB       |
| 1024     | ~24 KB       |
| 2048     | ~48 KB       |

ESP32 has ~320 KB of RAM — 1024 is a comfortable default.  
For AVR (Uno/Nano) use 64 or 128 max due to the 2 KB RAM limit.

---

## Supported Platforms

| Platform | Status |
|----------|--------|
| ESP32 | ✅ Fully tested |
| ESP8266 | ✅ Supported |
| Arduino Uno / Nano | ⚠️ Use FFT size ≤ 128 |
| Arduino Mega | ✅ Up to 512 |
| Arduino MKR / SAMD | ✅ Supported |
| Raspberry Pi Pico (RP2040) | ✅ Supported |

---

## Contributing

Contributions are welcome! Here's how:

1. **Fork** this repository
2. Create a feature branch: `git checkout -b feature/your-feature`
3. Commit your changes: `git commit -m "Add your feature"`
4. Push to the branch: `git push origin feature/your-feature`
5. Open a **Pull Request**

**Ideas for contributions:**
- Additional window functions (Blackman, Flat-top)
- Real-FFT optimisation (half the computation for real-only signals)
- New examples (ADC sampling, accelerometer, wireless)
- AVR-specific optimisations

Please open an issue first for major changes so we can discuss the approach.

---

## Credits

- Original KISS FFT by **Mark Borgerding** — [github.com/mborgerding/kissfft](https://github.com/mborgerding/kissfft) (BSD 3-Clause)
- Arduino/ESP32 port and wrapper by **Meer Zafarullah Noohani** — [github.com/meerzafarnoohani](https://github.com/meerzafarnoohani)

---

## License

MIT License — see [LICENSE](LICENSE) for full text.  
The underlying KISS FFT engine retains its original BSD 3-Clause license (credited in source).
