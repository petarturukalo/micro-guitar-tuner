TODO
- description: A Raspberry Pi Pico microcontroller guitar tuner (TODO
if use RTOS then mention that too, etc.)
-- put demo vid here first thing (try and embed it in the output)
-- maybe even name project to mcu (or microcontroller) guitar tuner
- add motivation section for why it's pico (refer first paragraph of doc/overview.txt):
- list and link to dependent hardware (including the pico itself), their use and how
it relates/fulfills the motivation if possible (e.g. coin cell battery, so needed Pico)
-- show how the pin/wire component connections, maybe with fritzing? 
- list dependent software: SDK, RTOS if applicable, etc., and why software architecture
is the way it is (because wanted Pico)
-- CMSIS DSP/core
-- gnu octave (optional)
-- gnuplot? (optional)
-- TODO also list exact versions of software?
- can list theory books if needed for reader's further knowledge. 
-- ref music and computers to learn more about the theory behind it
-- ref digital audio with java book as supplementary info for whatever i used it for
(digital filters, FFT, cents)
-- ref HPS stuff (which? Noll because it has the formula, any others that helped
in understanding Noll? "efficient pitch detection for interactive music" for outlining
the variables used in Noll's HPS equation better?)
from it: digital filters, fft, high-level guitar tuner example (just to confirm
understanding/assumptions)
- basically give all the info needed for the reader to build their own pico guitar
tuner using my source
- should link to pico docs at all (pico sdk?)? or reader expected to be familiar? can ref pico sdk
or other material to remind them how to build it, or give instructions on making it with cmake
- mention how to run tests (emulation, graph outputs, etc.). or put in test/README.md and
tell reader to go look there
- mention parameters (sampling rate, etc.), implementation and limitations of FFT, 
dynamic frame size to help, etc.
- explain directory structure: core is for src of core lib. include is its API.
test is for tests for testing core. mcu (if make a folder for it?) is for mcu 
specific usage. allows to separate and test the majority of the functionality
separately to the mcu code (rationale?) ...
