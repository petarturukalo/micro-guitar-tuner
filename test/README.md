# Intro

There are two test programs, `assert-tests` to programmatically test
the core library, and `gen-freq-mag-plots` to generate plots to visualise 
aspects of its DSP.

# Usage 

Although these are user space applications, cross compilation is still required
to build them because CMSIS DSP is for ARM Cortex-A and Cortex-M processors.
Note these binaries depend on and build a Cortex-A version of the core library,
whereas the guitar tuner proper binary in `../mcu/` depends on and builds a Cortex-M
version. Compile both binaries by running `make`, which uses `arm-linux-gnueabihf-gcc` 
by default. 

Because these are ARM binaries, you'll likely need to use an emulator such as QEMU 
to run them, e.g. 

```
qemu-arm -L /usr/arm-linux-gnueabihf gen-freq-mag-plots
```

WARNING the generated plots take up a fair amount of disk space, ~150 MB.

# Generate Plots

The `gen-freq-mag-plots` binary takes as input samples from all note audio file 
sources in the `test/data/note/` dir and outputs plots to the `plot/` dir. Plots
are generated using [gnuplot](http://www.gnuplot.info/), and so must be installed. 
See the README in [`test/data/note/`](test/data/note) for an explanation of the file 
format of the note file sources. Produced for each frame of samples is two plots,
both showing the magnitude of the frequencies of the note in the frequency domain,
one before HPS processing, and one after (see TODO top-level README DSP section for more info). 

Xtics are spaced at `plot.c:XTICS_INCR` Hz: to see more accurately the frequency of 
a peak, open the svg plot file in a browser (TODO test which browsers work) and then
left click the mouse to bring up the coordinate that the cursor is currently over,
as in the following image.

	TODO show image of G3 with coordinate over fundamental frequency peak

# Assertion Tests

The `assert-tests` binary simply runs assertion tests on core library API functions
and outputs whether all tests succeeded. If a test fails it will output where in the
source the test failed, along with some simple diagnostics. The assertion tests also
use the note audio file sources in `test/data/note/` as test data, along with the 
sine wave file sources in `test/data/sine/`.

