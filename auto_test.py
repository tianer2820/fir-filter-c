#!/usr/bin/env python3

import numpy as np
import subprocess
import random
import matplotlib.pyplot as plt
from scipy import signal
import os

# Compile the C code if needed
if not os.path.exists('auto_test'):
    print("Compiling C code...")
    subprocess.run(['make', 'clean'], check=True)
    subprocess.run(['make'], check=True)

def run_c_firwin(numtaps, fs, window_type, cutoffs):
    """Run the C implementation and return the coefficients."""
    # Prepare the command line arguments
    args = ['./auto_test', str(numtaps), str(fs), str(window_type)] + [str(c) for c in cutoffs]
    
    # Run the command and capture output
    result = subprocess.run(args, capture_output=True, text=True)
    
    if result.returncode != 0:
        print("Error running C implementation:")
        print(result.stderr)
        return None
    
    # Parse the output to get coefficients
    coefficients = []
    in_coeffs = False
    
    for line in result.stdout.split('\n'):
        line = line.strip()
        if line.startswith('#'):
            if 'Coefficients:' in line:
                in_coeffs = True
            continue
        if in_coeffs and line:
            try:
                coeff = float(line)
                coefficients.append(coeff)
            except ValueError:
                pass
    
    return np.array(coefficients)

def run_scipy_firwin(numtaps, fs, window_type, cutoffs):
    """Run the SciPy implementation and return the coefficients."""
    # Map window type to SciPy window names and parameters
    window_info = {
        0: ('boxcar', {}),                # Rectangular
        1: ('hamming', {}),               # Hamming
        2: ('blackman', {}),              # Blackman
        3: ('triang', {}),                # Triangular
        4: ('boxcar', {}),                # Parzen (not in scipy, using boxcar as fallback)
        5: ('bohman', {}),                # Bohman
        6: ('nuttall', {}),               # Nuttall
        7: ('blackmanharris', {}),        # Blackman-Harris
        8: ('flattop', {}),               # Flat-top
        9: ('bartlett', {}),              # Bartlett
        10: ('hann', {}),                 # Hann
        11: ('cosine', {})                # Cosine (sine)
    }
    
    
    # Handle the case where the first cutoff is 0 (lowpass/bandpass)
    pass_zero = cutoffs[0] == 0
    pass_nyquist = cutoffs[-1] >= fs/2
    if pass_zero:
        cutoffs = cutoffs[1:]
    if pass_nyquist:
        cutoffs = cutoffs[:-1]
    
    # Get window type and parameters
    window_name, window_params = window_info[window_type]
    
    # Call SciPy's firwin with appropriate parameters

    coeffs = signal.firwin(
        numtaps,
        cutoffs,
        window=window_name,
        pass_zero=pass_zero,
        fs=fs,
        **window_params
    )
    
    # For cosine window, we need to use a different approach as scipy doesn't have it directly
    if window_type == 11:  # Cosine window
        n = np.arange(numtaps)
        coeffs = np.sin(np.pi * (n + 0.5) / numtaps)
    
    return coeffs



if __name__ == "__main__":
    PLOT_RESULTS = False
    TEST_NUM = 1000
    PRINT_DEBUG = False

    # Test each window type with a simple low-pass filter
    window_names = [
        'Rectangular', 'Hamming', 'Blackman', 'Triangular',
        'Parzen', 'Bohman', 'Nuttall', 'Blackman-Harris',
        'Flat-top', 'Bartlett', 'Hann', 'Cosine'
    ]
    for window_type in range(12):
        print(f"\nTesting {window_names[window_type]} window")
        print("=" * 50)

        test_results = []

        for test_idx in range(TEST_NUM):
            # generate random params
            fs = random.randint(1000, 5000)
            numtaps = random.randint(5, 100)
            num_bands = random.randint(1, 10)
            cutoffs = [random.random() * (fs//2-2) + 1 for _ in range(num_bands * 2)]
            cutoffs.sort()
            pass_zero = random.choice([True, False])
            pass_nyquist = random.choice([True, False])
            if numtaps % 2 == 0:
                pass_nyquist = False
            
            if num_bands == 1 and pass_zero and pass_nyquist:
                if random.choice([True, False]):
                    pass_zero = False
                else:
                    pass_nyquist = False

            if pass_zero:
                cutoffs[0] = 0
            if pass_nyquist:
                cutoffs[-1] = fs/2

            if PRINT_DEBUG:
                print(f"Test {test_idx + 1}: fs={fs}, numtaps={numtaps}, num_bands={num_bands}, cutoffs={cutoffs}, pass_zero={pass_zero}, pass_nyquist={pass_nyquist}")

            # Run both implementations
            c_coeffs = run_c_firwin(numtaps, fs, window_type, cutoffs)
            py_coeffs = run_scipy_firwin(numtaps, fs, window_type, cutoffs)
        
            if PLOT_RESULTS:
                # Plot results
                plt.figure(figsize=(12, 6))
                
                # Time domain
                plt.subplot(1, 2, 1)
                plt.plot(c_coeffs, 'b-', linewidth=2, label='C Implementation')
                plt.plot(py_coeffs, 'r--', linewidth=1.5, label='SciPy')
                plt.title(f'{window_names[window_type]} Window - Time Domain')
                plt.xlabel('Sample')
                plt.ylabel('Amplitude')
                plt.legend()
                plt.grid(True)
                
                # Frequency domain
                plt.subplot(1, 2, 2)
                for coeffs, color, label in [(c_coeffs, 'b-', 'C'), (py_coeffs, 'r--', 'SciPy')]:
                    w, h = signal.freqz(coeffs, fs=fs)
                    plt.semilogx(w, 20 * np.log10(np.maximum(np.abs(h), 1e-10)), 
                                color, linewidth=2, label=f'{label} Response')
                
                plt.title(f'{window_names[window_type]} Window - Frequency Response')
                plt.xlabel('Frequency [Hz]')
                plt.ylabel('Magnitude [dB]')
                plt.ylim(-100, 10)
                plt.xlim(1, fs/2)
                for cutoff in cutoffs:
                    plt.axvline(cutoff, color='k', linestyle='--', alpha=0.7)
                plt.axhline(-3, color='k', linestyle='--', alpha=0.7)
                plt.legend()
                plt.grid(True, which='both', axis='both')
                
                plt.tight_layout()
                plt.show()

                break

            else:
                # check if the two arrays are equal
                if np.allclose(c_coeffs, py_coeffs, rtol=1e-4, atol=1e-4):
                    test_results.append(1)
                    if PRINT_DEBUG:
                        print(f"Test {test_idx + 1} passed")
                else:
                    test_results.append(0)
                    if PRINT_DEBUG:
                        print(f"Test {test_idx + 1} failed")
            
        print(f"Pass rate: {sum(test_results) / len(test_results)}")
                    
