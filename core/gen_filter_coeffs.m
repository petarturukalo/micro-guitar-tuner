% Copyright (C) 2024 Petar Turukalo
% SPDX-License-Identifier: GPL-2.0
%
% Generate coefficients for a brick wall band-pass filter for use by the 
% CMSIS DSP arm_fir_init_*() functions. 

pkg load signal

if (nargin != 2)
	error("Expected 2 args oversampling rate and number of taps/coefficients")
endif

oversampling_rate = str2num(argv{1});
% Filter out some of the low frequency noise and frequencies less than
% the lowest note C0. Ideally want a very sharp slope at ~16 Hz but the 
% order required for it is too high and makes the performance unusable
% (see also comment at nr_taps variable in dsp_params.mk).
highpass_cutoff_freq = 13;
% So it begins to slope just about the highest supported note, G#6.
% See also comments at 'note.c:note_freqs'.
lowpass_cutoff_freq = 1700;
% The higher the order the steeper the cutoff slope (and the closer it is
% to a brick wall filter), but also the more coefficients and memory required 
% to store them and time spent to process them. 
order = str2num(argv{2})-1;

coeffs = fir1(order, [highpass_cutoff_freq/(oversampling_rate/2), 
                      lowpass_cutoff_freq/(oversampling_rate/2)], "bandpass");
% Convert to 32-bit float.
coeffs = single(coeffs);

