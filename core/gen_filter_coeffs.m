% Generate coefficients for a brick wall band-pass filter for use by the 
% CMSIS DSP arm_fir_init_*() functions. 
pkg load signal

if (nargin != 1)
	error("Expected 1 arg number of taps/coefficients")
endif

oversampling_rate = 4096;
% So it begins to slope at ~16 Hz, just a bit below the lowest note C0.
highpass_cutoff_freq = 13;
% So it begins to slope just above the highest supported note, G#6.
% See also comments at 'note.c:note_freqs'.
lowpass_cutoff_freq = 1700;
% The higher the order the steeper the cutoff slope (and the closer it is
% to a brick wall filter), but also the more coefficients and memory required 
% to store them and time spent to process them. 
order = str2num(argv{1})-1;

coeffs = fir1(order, [highpass_cutoff_freq/(oversampling_rate/2), 
                      lowpass_cutoff_freq/(oversampling_rate/2)], "bandpass");
% Convert to 32-bit float.
coeffs = single(coeffs);

% TODO
% - share oversampling_rate with def in dsp.h? (struggled to cleanly)
% -- same goes for repeated documentation?
% - ref somewhere else for note limits?
