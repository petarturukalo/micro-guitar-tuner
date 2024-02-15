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
-- TODO also list exact versions of software?
- can list theory books if needed for reader's further knowledge. 
-- ref music and computers to learn more about the theory behind it
-- ref digital audio with java book as example for application of theory 
(what to do with the output of FFT, just confirming understanding)
- basically give all the info needed for the reader to build their own pico guitar
tuner using my source
- should link to pico docs at all? or reader expected to be familiar? 
- mention how to run tests (emulation, graph outputs, etc.)
- mention implementation and limitations of FFT, dynamic frame size to help, etc.
