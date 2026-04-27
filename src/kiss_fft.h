#ifndef KISS_FFT_H
#define KISS_FFT_H

/*
 * KISS FFT - Arduino/ESP32 Port
 * Originally by Mark Borgerding
 * Ported and adapted for Arduino/ESP32 embedded use
 *
 * KISS = Keep It Simple, Stupid
 * A mixed-radix FFT with support for power-of-2 sizes.
 */

#include <Arduino.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

// ── Configuration ──────────────────────────────────────────────────────────
// Use float (32-bit) for ESP32 — it has an FPU.
// Change to double only if you need extra precision (2x slower).
#ifndef KISS_FFT_SCALAR
  #define KISS_FFT_SCALAR float
#endif

typedef struct {
    KISS_FFT_SCALAR r;
    KISS_FFT_SCALAR i;
} kiss_fft_cpx;

// ── Internal state ──────────────────────────────────────────────────────────
struct kiss_fft_state;
typedef struct kiss_fft_state* kiss_fft_cfg;

// ── Public API ──────────────────────────────────────────────────────────────

/**
 * Allocate and initialise an FFT plan.
 *
 * @param nfft       Number of samples. Best performance when power-of-2.
 * @param inverse_fft  0 = forward FFT,  1 = inverse FFT
 * @param mem        Optional pre-allocated buffer (pass NULL to use heap)
 * @param lenmem     If mem!=NULL: size of buffer; on return: bytes actually used
 * @return           Opaque config handle, or NULL on failure
 */
kiss_fft_cfg kiss_fft_alloc(int nfft, int inverse_fft, void* mem, size_t* lenmem);

/**
 * Run the FFT.
 *
 * @param cfg   Plan from kiss_fft_alloc()
 * @param fin   Input array  (nfft complex samples)
 * @param fout  Output array (nfft complex samples) — may equal fin for in-place
 */
void kiss_fft(kiss_fft_cfg cfg, const kiss_fft_cpx* fin, kiss_fft_cpx* fout);

/**
 * Free a plan allocated on the heap.
 * Safe to call with NULL.
 */
void kiss_fft_free(kiss_fft_cfg cfg);

/**
 * Stride version — useful for processing interleaved data or sub-arrays.
 */
void kiss_fft_stride(kiss_fft_cfg st, const kiss_fft_cpx* fin,
                     kiss_fft_cpx* fout, int fin_stride);

/**
 * Utility: next fast FFT size >= n (i.e. smallest power-of-2 >= n).
 */
int kiss_fft_next_fast_size(int n);

// ── Convenience macro ───────────────────────────────────────────────────────
#define KISS_FFT_TMP_ALLOC(nbytes)   (malloc(nbytes))
#define KISS_FFT_TMP_FREE(ptr)       (free(ptr))

#endif // KISS_FFT_H
