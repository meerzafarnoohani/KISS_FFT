#ifndef KISS_FFT_ARDUINO_H
#define KISS_FFT_ARDUINO_H

/*
 * KISS_FFT_Arduino.h
 * High-level Arduino wrapper for the KISS FFT library.
 *
 * Simplifies common tasks:
 *   - Running a forward FFT on real samples (microphone, ADC, etc.)
 *   - Computing magnitude spectrum
 *   - Finding dominant frequency
 *   - Running an inverse FFT
 *
 * Typical usage:
 *   KissFFT fft(1024);
 *   fft.setInput(samples, 1024);   // float array of real samples
 *   fft.compute();
 *   float* mag = fft.getMagnitude();
 *   float  freq = fft.dominantFrequency(sampleRateHz);
 */

#include "kiss_fft.h"

class KissFFT {
public:
    /**
     * @param fftSize  Number of samples.  Best as power of 2 (e.g. 256, 512, 1024).
     */
    explicit KissFFT(int fftSize)
        : _n(fftSize), _cfgFwd(nullptr), _cfgInv(nullptr),
          _in(nullptr), _out(nullptr), _mag(nullptr)
    {
        _cfgFwd = kiss_fft_alloc(_n, 0, nullptr, nullptr);
        _cfgInv = kiss_fft_alloc(_n, 1, nullptr, nullptr);
        _in     = new kiss_fft_cpx[_n];
        _out    = new kiss_fft_cpx[_n];
        _mag    = new float[_n / 2 + 1]; // only positive frequencies needed
        memset(_in,  0, sizeof(kiss_fft_cpx) * _n);
        memset(_out, 0, sizeof(kiss_fft_cpx) * _n);
    }

    ~KissFFT() {
        kiss_fft_free(_cfgFwd);
        kiss_fft_free(_cfgInv);
        delete[] _in;
        delete[] _out;
        delete[] _mag;
    }

    // ── Input loading ─────────────────────────────────────────────────────

    /**
     * Load real-valued samples (e.g. from analogRead / I2S / microphone).
     * Imaginary parts are set to 0.
     *
     * @param samples   Pointer to float array
     * @param count     Number of samples (must be <= fftSize)
     */
    void setInput(const float* samples, int count) {
        int len = min(count, _n);
        for (int i = 0; i < len; ++i) {
            _in[i].r = samples[i];
            _in[i].i = 0.0f;
        }
        for (int i = len; i < _n; ++i) {
            _in[i].r = _in[i].i = 0.0f;
        }
    }

    /**
     * Load integer samples (e.g. raw 12-bit ADC values).
     * Values are converted to float; you can pass a scale factor.
     *
     * @param samples    Pointer to int16_t array
     * @param count      Number of samples
     * @param scale      Divide each sample by this (e.g. 2048.0f to normalise 12-bit)
     */
    void setInput(const int16_t* samples, int count, float scale = 1.0f) {
        int len = min(count, _n);
        for (int i = 0; i < len; ++i) {
            _in[i].r = (float)samples[i] / scale;
            _in[i].i = 0.0f;
        }
        for (int i = len; i < _n; ++i) {
            _in[i].r = _in[i].i = 0.0f;
        }
    }

    /**
     * Load complex-valued input directly (advanced use).
     */
    void setInputComplex(const kiss_fft_cpx* cpx, int count) {
        int len = min(count, _n);
        memcpy(_in, cpx, sizeof(kiss_fft_cpx) * len);
        for (int i = len; i < _n; ++i) {
            _in[i].r = _in[i].i = 0.0f;
        }
    }

    // ── Window functions ──────────────────────────────────────────────────
    // Apply before compute() to reduce spectral leakage.

    /** Hann (Hanning) window — good general-purpose choice */
    void applyWindowHann() {
        for (int i = 0; i < _n; ++i) {
            float w = 0.5f * (1.0f - cosf(2.0f * (float)M_PI * i / (_n - 1)));
            _in[i].r *= w;
        }
    }

    /** Hamming window */
    void applyWindowHamming() {
        for (int i = 0; i < _n; ++i) {
            float w = 0.54f - 0.46f * cosf(2.0f * (float)M_PI * i / (_n - 1));
            _in[i].r *= w;
        }
    }

    /** Rectangular window (no windowing — fastest, most leakage) */
    void applyWindowRect() { /* nothing to do */ }

    // ── Compute ───────────────────────────────────────────────────────────

    /** Run the forward FFT.  Call setInput() first. */
    void compute() {
        kiss_fft(_cfgFwd, _in, _out);
        _updateMagnitude();
    }

    /** Run the inverse FFT.  Input must be set via setInputComplex(). */
    void computeInverse() {
        kiss_fft(_cfgInv, _in, _out);
        // Normalise: KISS FFT does NOT normalise the inverse
        float inv_n = 1.0f / _n;
        for (int i = 0; i < _n; ++i) {
            _out[i].r *= inv_n;
            _out[i].i *= inv_n;
        }
    }

    // ── Output access ─────────────────────────────────────────────────────

    /**
     * Magnitude spectrum (after compute()).
     * Returns array of (_n/2 + 1) values — index 0 = DC, index k = k * fs/N.
     */
    float* getMagnitude() { return _mag; }

    /** Raw complex FFT output array (size = fftSize). */
    kiss_fft_cpx* getOutput() { return _out; }

    /** Raw complex FFT input array (size = fftSize). */
    kiss_fft_cpx* getInput() { return _in; }

    /** FFT size passed to constructor. */
    int size() const { return _n; }

    // ── Convenience ───────────────────────────────────────────────────────

    /**
     * Find the bin index with the largest magnitude (ignoring DC bin 0).
     */
    int dominantBin() const {
        int peak = 1;
        for (int i = 2; i <= _n / 2; ++i) {
            if (_mag[i] > _mag[peak]) peak = i;
        }
        return peak;
    }

    /**
     * Convert a bin index to frequency in Hz.
     * @param bin         Bin index (0 … N/2)
     * @param sampleRate  Sample rate in Hz (e.g. 44100)
     */
    float binToFrequency(int bin, float sampleRate) const {
        return (float)bin * sampleRate / (float)_n;
    }

    /**
     * Return the dominant frequency in Hz.
     * @param sampleRate  Sample rate in Hz
     */
    float dominantFrequency(float sampleRate) const {
        return binToFrequency(dominantBin(), sampleRate);
    }

    /**
     * Power spectral density (magnitude squared) of a bin.
     */
    float power(int bin) const {
        if (bin < 0 || bin > _n / 2) return 0.0f;
        return _mag[bin] * _mag[bin];
    }

    /**
     * RMS amplitude of the reconstructed signal (Parseval's theorem).
     */
    float rms() const {
        float sum = 0.0f;
        for (int i = 0; i <= _n / 2; ++i) sum += power(i);
        return sqrtf(sum) / _n;
    }

private:
    int           _n;
    kiss_fft_cfg  _cfgFwd;
    kiss_fft_cfg  _cfgInv;
    kiss_fft_cpx* _in;
    kiss_fft_cpx* _out;
    float*        _mag;

    void _updateMagnitude() {
        for (int i = 0; i <= _n / 2; ++i) {
            _mag[i] = sqrtf(_out[i].r * _out[i].r + _out[i].i * _out[i].i);
        }
    }

    // Non-copyable
    KissFFT(const KissFFT&);
    KissFFT& operator=(const KissFFT&);
};

#endif // KISS_FFT_ARDUINO_H
