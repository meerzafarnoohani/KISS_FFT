/*
 * MicrophoneFFT.ino
 * KISS FFT — ESP32 I2S Microphone Example
 *
 * Reads audio from an I2S MEMS microphone (e.g. INMP441, SPH0645),
 * computes the FFT every 'FFT_SIZE' samples, and prints the dominant frequency.
 *
 * Wiring (INMP441 example):
 *   INMP441  →  ESP32
 *   VDD      →  3.3V
 *   GND      →  GND
 *   WS       →  GPIO 15
 *   SCK      →  GPIO 14
 *   SD       →  GPIO 32
 *   L/R      →  GND  (select left channel)
 *
 * Open Serial Monitor at 115200 baud.
 */

#include <driver/i2s.h>
#include <KISS_FFT_Arduino.h>

// ── I2S config ───────────────────────────────────────────────────────────────
#define I2S_WS   15
#define I2S_SCK  14
#define I2S_SD   32
#define I2S_PORT I2S_NUM_0

static const int   FFT_SIZE    = 1024;
static const float SAMPLE_RATE = 16000.0f; // 16 kHz

// ── Globals ──────────────────────────────────────────────────────────────────
KissFFT  fft(FFT_SIZE);
int16_t  rawBuf[FFT_SIZE];
float    floatBuf[FFT_SIZE];

// ── I2S initialisation ───────────────────────────────────────────────────────
void i2sInit() {
    i2s_config_t cfg = {
        .mode                 = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate          = (uint32_t)SAMPLE_RATE,
        .bits_per_sample      = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format       = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags     = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count        = 8,
        .dma_buf_len          = 64,
        .use_apll             = false,
    };
    i2s_pin_config_t pins = {
        .bck_io_num   = I2S_SCK,
        .ws_io_num    = I2S_WS,
        .data_out_num = I2S_PIN_NO_CHANGE,
        .data_in_num  = I2S_SD,
    };
    i2s_driver_install(I2S_PORT, &cfg, 0, nullptr);
    i2s_set_pin(I2S_PORT, &pins);
}

// ── Setup ────────────────────────────────────────────────────────────────────
void setup() {
    Serial.begin(115200);
    while (!Serial) delay(10);
    Serial.println(F("KISS FFT — Microphone Example"));
    i2sInit();
}

// ── Loop ─────────────────────────────────────────────────────────────────────
void loop() {
    // 1. Read a full frame from I2S
    size_t bytesRead = 0;
    i2s_read(I2S_PORT, rawBuf, sizeof(rawBuf), &bytesRead, portMAX_DELAY);

    int samplesRead = bytesRead / sizeof(int16_t);
    if (samplesRead < FFT_SIZE) return; // not enough data yet

    // 2. Normalise to [-1, 1]
    for (int i = 0; i < FFT_SIZE; ++i) {
        floatBuf[i] = (float)rawBuf[i] / 32768.0f;
    }

    // 3. Load, window, compute
    fft.setInput(floatBuf, FFT_SIZE);
    fft.applyWindowHann();
    fft.compute();

    // 4. Report
    float dominant = fft.dominantFrequency(SAMPLE_RATE);
    Serial.print(F("Dominant: "));
    Serial.print(dominant, 1);
    Serial.print(F(" Hz   RMS: "));
    Serial.println(fft.rms(), 4);

    delay(100); // ~10 FFT frames per second
}
