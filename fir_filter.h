#ifndef FIR_FILTER_H
#define FIR_FILTER_H

// Window types
enum fir_filter_window_type {
    RECTANGULAR,   // Rectangular (boxcar) window
    HAMMING,       // Hamming window
    BLACKMAN,      // Blackman window
    TRIANGULAR,    // Triangular window
    PARZEN,        // Parzen window
    BOHMAN,        // Bohman window
    NUTTALL,       // Nuttall window (minimum 4-term Blackman-Harris)
    BLACKMANHARRIS,// Minimum 4-term Blackman-Harris window
    FLATTOP,       // Flat top window
    BARTLETT,      // Bartlett window (triangular with zero endpoints)
    HANN,          // Hann window (raised cosine)
    COSINE         // Cosine (sine) window
};

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/**
 * @brief Create a fir filter.
 * 
 * @param numtaps Number of taps (must be odd)
 * @param cutoff_count Number of cutoffs (must be even and at least 2)
 * @param cutoffs Array of cutoff frequencies in Hz. Each pair of cutoffs defines a passband, use 0 for the first cutoff to define a lowpass filter, use fs/2 for the last cutoff to define a highpass filter.
 * @param fs Sampling frequency in Hz
 * @param window Window type (RECTANGULAR, HAMMING, or BLACKMAN)
 * @param out Output array (must be pre-allocated with size numtaps)
 * @return 0 on success, -1 on error
 */
int firwin(int numtaps, int cutoff_count, const float* cutoffs, float fs, 
           enum fir_filter_window_type window, float* out);


#endif