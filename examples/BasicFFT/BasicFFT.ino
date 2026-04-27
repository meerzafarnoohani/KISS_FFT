/*
 * BasicFFT.ino
 * KISS FFT — Basic Example for ESP32 / Arduino
 *
 * Demonstrates:
 *   1. Generating a synthetic test signal (440 Hz sine + 1 kHz sine)
 *   2. Applying a Hann window to reduce spectral leakage
 *   3. Running the FFT
 *   4. Printing the magnitude spectrum over Serial
 *   5. Reporting the dominant frequency
 *
 * Open Serial Monitor at 115200 baud.
 *
 * Hardware:  Any ESP32 board (or Arduino with enough RAM — Uno has too little).
 *            For a real microphone, replace generateTestSignal() with ADC reads.
 */

#include <KISS_FFT_Arduino.h>

// ── Parameters ──────────────────────────────────────────────────────────────
static const int   FFT_SIZE    = 1024;   // Must be power of 2
static const float SAMPLE_RATE = 8000.0f; // Hz — match your actual ADC rate

// ── Globals ─────────────────────────────────────────────────────────────────
KissFFT fft(FFT_SIZE);
float   samples[FFT_SIZE];

// ── Helpers ─────────────────────────────────────────────────────────────────

/**
 * Fill 'samples' with two sine waves to verify the FFT.
 * Replace this function with real ADC / I2S reads in your project.
 */
void generateTestSignal() {
    const float f1 = 440.0f;   // A4 note
    const float f2 = 1000.0f;  // 1 kHz tone
    for (int i = 0; i < FFT_SIZE; ++i) {
        float t = (float)i / SAMPLE_RATE;
        samples[i] = 0.7f * sinf(2.0f * PI * f1 * t)
                   + 0.3f * sinf(2.0f * PI * f2 * t);
    }
}

void printSpectrum(float* mag, int bins, float sampleRate, int nfft) {
    Serial.println(F("Bin\tFrequency(Hz)\tMagnitude"));
    for (int i = 1; i <= bins; ++i) {          // skip DC (bin 0)
        if (mag[i] > 5.0f) {                   // only print significant bins
            float freq = (float)i * sampleRate / nfft;
            Serial.print(i);
            Serial.print('\t');
            Serial.print(freq, 1);
            Serial.print('\t');
            Serial.println(mag[i], 2);
        }
    }
}

// ── Setup ────────────────────────────────────────────────────────────────────
void setup() {
    Serial.begin(115200);
    while (!Serial) delay(10);

    Serial.println(F("\n=== KISS FFT — Basic Example ==="));
    Serial.print(F("FFT size:    ")); Serial.println(FFT_SIZE);
    Serial.print(F("Sample rate: ")); Serial.print(SAMPLE_RATE); Serial.println(F(" Hz"));
    Serial.print(F("Resolution:  "));
    Serial.print(SAMPLE_RATE / FFT_SIZE, 2);
    Serial.println(F(" Hz/bin\n"));

    // 1. Fill input buffer
    generateTestSignal();

    // 2. Load samples into FFT object
    fft.setInput(samples, FFT_SIZE);

    // 3. Apply Hann window (reduces spectral leakage)
    fft.applyWindowHann();

    // 4. Compute FFT
    unsigned long t0 = micros();
    fft.compute();
    unsigned long elapsed = micros() - t0;

    Serial.print(F("FFT computed in ")); Serial.print(elapsed); Serial.println(F(" µs"));

    // 5. Print spectrum
    printSpectrum(fft.getMagnitude(), FFT_SIZE / 2, SAMPLE_RATE, FFT_SIZE);

    // 6. Dominant frequency
    Serial.print(F("\nDominant frequency: "));
    Serial.print(fft.dominantFrequency(SAMPLE_RATE), 1);
    Serial.println(F(" Hz"));

    Serial.print(F("Signal RMS: "));
    Serial.println(fft.rms(), 4);
}

void loop() {
    // Nothing — results are printed once in setup().
    // In a real application, continuously re-sample and re-compute here.
    delay(5000);
}
