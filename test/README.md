There are two test programs, `gen-freq-mag-plots` and TODO ...

TODO 
- write up
- what the test programs are for, how to build and use them
(cross compiling and emulation, why it's for cortex-a?, 32-bit/64-bit arm gcc/emu?)
- split this up into sections: intro, generate-plots info,
other test info, how to build and run either
- generate-plots:
-- explain that it takes all notes files in blah and spits out plots in plot (mention frames too)
-- give warning on how much space this will use 
-- info: Xtics are spaced at `plot.c:XTICS_INCR` Hz: to see more accurately the frequency of a peak 
	open the svg file in a browser and then left click the mouse to bring up the coordinate 
	that the cursor is currently over. (TODO and then show image of it as an example)
-- explain the contents of the notes and what you're expected to see. peaks at integer multiples of
	the fundamental frequency (Fourier analysis), and that the HPS version then turns the fundamental
	frequency into the max peak (show before and after example of one where fundamental isn't max in before)

