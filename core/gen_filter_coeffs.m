% Generate coefficients for a brick wall band-pass filter for use by the 
% CMSIS DSP arm_fir_init_*() functions. A plot of the filter is opened in 
% a new window and the coefficients printed to standard out as single precision
% 32-bit floats.
pkg load signal

oversampling_rate = 4096;
% So it begins to slope at ~16 Hz, just a bit below the lowest note C0.
highpass_cutoff_freq = 13;
% So it begins to slope just above the highest supported note, G#6.
lowpass_cutoff_freq = 1700;
% The higher the order the steeper the cutoff slope (and the closer it is
% to a brick wall filter), but also the more coefficients and memory required 
% to store them and time spent to process them. 
order = 1999;

coeffs = fir1(order, [highpass_cutoff_freq/(oversampling_rate/2), 
                      lowpass_cutoff_freq/(oversampling_rate/2)], "bandpass");
% Convert to 32-bit float.
coeffs = single(coeffs);
% Plot filter.
freqz(coeffs, 1, 512, oversampling_rate)
drawnow()
% Reverse coefficients because arm_fir_init_*() requires them in time reversed order. 
coeffs = fliplr(coeffs);
% Print coefficients. 
% The 9 is FLT_DECIMAL_DIG from float.h. See its documentation for more info.
printf("%.9g, %.9g, %.9g, %.9g, %.9g,\n", coeffs)
% Wait for plot to be closed before exiting.
uiwait()

% TODO
% - share variables like oversampling_rate with their .c definitions?
% -- same goes for repeated documentation?
% - ref somewhere else for note limits?
