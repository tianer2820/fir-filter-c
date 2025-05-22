# FIR Filter C

This is a C implementation of the firwin function from the scipy.signal module.

This repo only implements the firwin function, if you need IIR filters such as butterworth and chebyshev in C, take a look at https://github.com/adis300/filter-c.

## Usage
Copy the firwin.c and firwin.h files to your project, and add them to your Makefile, CMakeLists, Scons, or whatever you are using. There is only one function in the library, called `firwin`, which takes the same parameters as the scipy.signal.firwin function.

**Warning**: Not all window types are supported, and some are not correctly implemented at the time of writing. The table below shows the pass rate of 1000 random tests for each window type. Window types with a pass rate of 0.999 or 1.0 are probably safe to use.

| Window type | Pass rate |
|-------------|----------|
| Rectangular | 0.999    |
| Hamming     | 0.999    |
| Blackman    | 1.0      |
| Triangular  | 0.517    |
| Parzen      | 0.0      |
| Bohman      | 1.0      |
| Nuttall     | 1.0      |
| Blackman-Harris | 1.0      |
| Flat-top    | 1.0      |
| Bartlett    | 1.0      |
| Hann        | 0.999    |
| Cosine      | 0.0      |

## Autotesting
The autotest.py script can be used to run the test described above. It will run 1000 random tests for each window type, compare the results with the scipy implementation, and print the pass rate. It can also plot the frequency response if you set the `PLOT_RESULTS` variable to `True`.

To run the autotest, simply run `python3 autotest.py` (you need to have scipy installed).
