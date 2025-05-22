
#include "fir_filter.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void print_usage(const char* prog_name) {
    printf("Usage: %s <numtaps> <fs> <window_type> <cutoff1> [cutoff2 ...]\n", prog_name);
    printf("  numtaps:    Number of filter taps (must be odd)\n");
    printf("  fs:         Sampling frequency in Hz\n");
    printf("  window_type: Window type (number or name):\n");
    printf("               0=Rectangular, 1=Hamming, 2=Blackman, 3=Triangular\n");
    printf("               4=Parzen, 5=Bohman, 6=Nuttall, 7=Blackman-Harris\n");
    printf("               8=Flattop, 9=Bartlett, 10=Hann, 11=Cosine\n");
    printf("  cutoffs:    List of cutoff frequencies in Hz (must be even number of cutoffs)\n");
    printf("\nExample: %s 51 1000.0 1 200.0 300.0\n", prog_name);
    printf("Example: %s 101 44100.0 hann 500.0 1000.0 3000.0 4000.0\n", prog_name);
}

int parse_window_type(const char* str) {
    if (strcmp(str, "0") == 0 || strcasecmp(str, "rectangular") == 0 || strcasecmp(str, "boxcar") == 0) return RECTANGULAR;
    if (strcmp(str, "1") == 0 || strcasecmp(str, "hamming") == 0) return HAMMING;
    if (strcmp(str, "2") == 0 || strcasecmp(str, "blackman") == 0) return BLACKMAN;
    if (strcmp(str, "3") == 0 || strcasecmp(str, "triangular") == 0) return TRIANGULAR;
    if (strcmp(str, "4") == 0 || strcasecmp(str, "parzen") == 0) return PARZEN;
    if (strcmp(str, "5") == 0 || strcasecmp(str, "bohman") == 0) return BOHMAN;
    if (strcmp(str, "6") == 0 || strcasecmp(str, "nuttall") == 0) return NUTTALL;
    if (strcmp(str, "7") == 0 || strcasecmp(str, "blackmanharris") == 0 || strcasecmp(str, "blackman-harris") == 0) return BLACKMANHARRIS;
    if (strcmp(str, "8") == 0 || strcasecmp(str, "flattop") == 0) return FLATTOP;
    if (strcmp(str, "9") == 0 || strcasecmp(str, "bartlett") == 0) return BARTLETT;
    if (strcasecmp(str, "10") == 0 || strcasecmp(str, "hann") == 0) return HANN;
    if (strcasecmp(str, "11") == 0 || strcasecmp(str, "cosine") == 0) return COSINE;
    return -1;
}

const char* get_window_name(enum fir_filter_window_type window) {
    switch (window) {
        case RECTANGULAR:   return "Rectangular (boxcar)";
        case HAMMING:       return "Hamming";
        case BLACKMAN:      return "Blackman";
        case TRIANGULAR:    return "Triangular";
        case PARZEN:        return "Parzen";
        case BOHMAN:        return "Bohman";
        case NUTTALL:       return "Nuttall";
        case BLACKMANHARRIS:return "Blackman-Harris";
        case FLATTOP:       return "Flat-top";
        case BARTLETT:      return "Bartlett";
        case HANN:          return "Hann";
        case COSINE:        return "Cosine (sine)";
        default:            return "Unknown";
    }
}

int main(int argc, char* argv[]) {
    // Check minimum number of arguments
    if (argc < 5) {
        print_usage(argv[0]);
        return 1;
    }
    
    // Parse command line arguments
    int numtaps = atoi(argv[1]);
    float fs = (float)atof(argv[2]);
    int window_type = parse_window_type(argv[3]);
    
    if (window_type == -1) {
        fprintf(stderr, "Error: Invalid window type. Must be 0, 1, or 2.\n");
        return 1;
    }
    
    int num_cutoffs = argc - 4;
    if (num_cutoffs % 2 != 0) {
        fprintf(stderr, "Error: Number of cutoff frequencies must be even.\n");
        return 1;
    }
    
    float* cutoffs = (float*)malloc(num_cutoffs * sizeof(float));
    if (!cutoffs) {
        fprintf(stderr, "Error: Memory allocation failed.\n");
        return 1;
    }
    
    for (int i = 0; i < num_cutoffs; i++) {
        cutoffs[i] = (float)atof(argv[4 + i]);
    }
    
    // Allocate array for coefficients
    float* h = (float*)malloc(numtaps * sizeof(float));
    if (!h) {
        fprintf(stderr, "Error: Memory allocation failed.\n");
        free(cutoffs);
        return 1;
    }
    
    // Design the filter
    printf("# FIR Filter Design\n");
    printf("# Taps: %d\n", numtaps);
    printf("# Sampling frequency: %.1f Hz\n", fs);
    printf("# Window: %s\n", get_window_name(window_type));
    printf("# Cutoffs: ");
    for (int i = 0; i < num_cutoffs; i++) {
        printf("%.1f ", cutoffs[i]);
    }
    printf("Hz\n\n");
    
    int result = firwin(numtaps, num_cutoffs, cutoffs, fs, window_type, h);
    if (result != 0) {
        fprintf(stderr, "Error designing filter.\n");
        free(cutoffs);
        free(h);
        return 1;
    }
    
    // Print all coefficients
    printf("# Coefficients:\n");
    for (int i = 0; i < numtaps; i++) {
        printf("%.15g\n", h[i]);
    }
    
    // Calculate and print the sum of coefficients
    float sum = 0.0f;
    for (int i = 0; i < numtaps; i++) {
        sum += h[i];
    }
    printf("\n# Sum of coefficients: %.15g\n", sum);
    
    free(cutoffs);
    free(h);
    return 0;
}

