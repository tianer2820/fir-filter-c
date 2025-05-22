#include "fir_filter.h"
#include <math.h>
#include <stdlib.h>

// Sinc function
static float sinc(float x) {
    if (x == 0.0f) {
        return 1.0f;
    }
    return sinf(M_PI * x) / (M_PI * x);
}

// Apply the specified window to the data
static void apply_window(float* data, int n, enum fir_filter_window_type window) {
    if (n <= 0) return;
    
    switch (window) {
        case RECTANGULAR:
            // Rectangular window is all ones - no need to do anything
            break;
            
        case HAMMING:
        {
            const float a0 = 0.54f;
            const float a1 = 1.0f - a0;
            
            for (int i = 0; i < n; i++) {
                float win = a0 - a1 * cosf(2.0f * M_PI * i / (n - 1));
                data[i] *= win;
            }
            break;
        }
        
        case BLACKMAN:
        {
            const float a0 = 0.42f;
            const float a1 = 0.5f;
            const float a2 = 0.08f;
            
            for (int i = 0; i < n; i++) {
                float x = 2.0f * M_PI * i / (n - 1);
                float win = a0 - a1 * cosf(x) + a2 * cosf(2.0f * x);
                data[i] *= win;
            }
            break;
        }
        
        case TRIANGULAR:
        {
            for (int i = 0; i < n; i++) {
                float win = 1.0f - fabsf((i - (n - 1) / 2.0f) / (n / 2.0f));
                data[i] *= win;
            }
            break;
        }
        
        case PARZEN:
        {
            int N = n - 1;
            float half_N = N / 2.0f;
            for (int i = 0; i < n; i++) {
                float x = fabsf((i - half_N) / half_N);
                float win;
                if (x <= 0.5f) {
                    win = 1.0f - 6.0f * x * x * (1.0f - x);
                } else {
                    float temp = 1.0f - x;
                    win = 2.0f * temp * temp * temp;
                }
                data[i] *= win;
            }
            break;
        }
        
        case BOHMAN:
        {
            float N = (float)(n - 1);
            for (int i = 0; i < n; i++) {
                float x = fabsf(2.0f * i / N - 1.0f);
                float win = (1.0f - x) * cosf(M_PI * x) + sinf(M_PI * x) / M_PI;
                data[i] *= win;
            }
            break;
        }
        
        case NUTTALL:
        {
            const float a0 = 0.3635819f;
            const float a1 = 0.4891775f;
            const float a2 = 0.1365995f;
            const float a3 = 0.0106411f;
            
            for (int i = 0; i < n; i++) {
                float x = 2.0f * M_PI * i / (n - 1);
                float win = a0 - a1 * cosf(x) + a2 * cosf(2.0f * x) - a3 * cosf(3.0f * x);
                data[i] *= win;
            }
            break;
        }
        
        case BLACKMANHARRIS:
        {
            const float a0 = 0.35875f;
            const float a1 = 0.48829f;
            const float a2 = 0.14128f;
            const float a3 = 0.01168f;
            
            for (int i = 0; i < n; i++) {
                float x = 2.0f * M_PI * i / (n - 1);
                float win = a0 - a1 * cosf(x) + a2 * cosf(2.0f * x) - a3 * cosf(3.0f * x);
                data[i] *= win;
            }
            break;
        }
        
        case FLATTOP:
        {
            const float a0 = 0.21557895f;
            const float a1 = 0.41663158f;
            const float a2 = 0.277263158f;
            const float a3 = 0.083578947f;
            const float a4 = 0.006947368f;
            
            for (int i = 0; i < n; i++) {
                float x = 2.0f * M_PI * i / (n - 1);
                float win = a0 - a1 * cosf(x) + a2 * cosf(2.0f * x) - 
                           a3 * cosf(3.0f * x) + a4 * cosf(4.0f * x);
                data[i] *= win;
            }
            break;
        }
        
        case BARTLETT:
        {
            float N = (float)(n - 1);
            for (int i = 0; i < n; i++) {
                float win = 1.0f - fabsf(2.0f * i / N - 1.0f);
                data[i] *= win;
            }
            break;
        }
        
        case HANN:
        {
            for (int i = 0; i < n; i++) {
                float win = 0.5f * (1.0f - cosf(2.0f * M_PI * i / (n - 1)));
                data[i] *= win;
            }
            break;
        }
        
        case COSINE:
        {
            for (int i = 0; i < n; i++) {
                float win = sinf(M_PI * (i + 0.5f) / n);
                data[i] *= win;
            }
            break;
        }
    }
}

// Create a FIR filter using the window method
int firwin(int numtaps, int cutoff_count, const float* cutoffs, float fs, 
           enum fir_filter_window_type window, float* out) {
    // Validate inputs
    if (numtaps <= 0 || cutoff_count <= 0 || !cutoffs || !out) {
        return -1;
    }
    
    if (cutoff_count % 2 != 0) {
        // Number of cutoffs must be even
        return -1;
    }
    
    // Check if cutoffs are in increasing order
    for (int i = 1; i < cutoff_count; i++) {
        if (cutoffs[i] <= cutoffs[i-1]) {
            return -1;
        }
    }
    
    // Check if cutoffs are within valid range [0, fs/2]
    float nyquist = fs / 2.0f;
    for (int i = 0; i < cutoff_count; i++) {
        if (cutoffs[i] < 0 || cutoffs[i] > nyquist) {
            return -1;
        }
    }
    
    // Check if last cutoff is at Nyquist with even number of taps
    if (cutoffs[cutoff_count-1] == nyquist && numtaps % 2 == 0) {
        return -1;  // Even number of taps can't have response at Nyquist
    }
    
    // Allocate temporary array for the filter coefficients
    float* h = (float*)calloc(numtaps, sizeof(float));
    if (!h) {
        return -1;
    }
    
    // Compute the ideal filter response
    float alpha = 0.5f * (numtaps - 1);
    
    // Process each pair of cutoffs as a passband
    for (int i = 0; i < cutoff_count; i += 2) {
        float left = cutoffs[i] / nyquist;
        float right = cutoffs[i+1] / nyquist;
        
        for (int n = 0; n < numtaps; n++) {
            float m = n - alpha;
            h[n] += right * sinc(right * m) - left * sinc(left * m);
        }
    }
    
    // Apply the window
    apply_window(h, numtaps, window);
    
    // Normalize the filter coefficients
    // Find the frequency at which to normalize (middle of first passband)
    float scale_freq;
    if (cutoffs[0] == 0.0f) {
        scale_freq = 0.0f;
    } else if (cutoffs[1] == nyquist) {
        scale_freq = 1.0f;
    } else {
        scale_freq = 0.5f * (cutoffs[0] + cutoffs[1]) / nyquist;
    }
    
    // Calculate the scaling factor
    float scale = 0.0f;
    for (int n = 0; n < numtaps; n++) {
        float m = n - alpha;
        scale += h[n] * cosf(M_PI * m * scale_freq);
    }
    
    // Avoid division by zero
    if (fabsf(scale) < 1e-10f) {
        scale = 1.0f;
    }
    
    // Apply scaling and copy to output
    for (int n = 0; n < numtaps; n++) {
        out[n] = h[n] / scale;
    }
    
    free(h);
    return 0;
}
