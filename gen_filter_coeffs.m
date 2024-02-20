% Generate coefficients for a brick wall band-pass filter for use by the 
% CMSIS DSP arm_fir_init_*() functions. A plot of the filter is opened in 
% a new window and the coefficients printed to standard out as single precision
% 32-bit floats.
% TODO see src where arm_fir_init_*() is called after it's written
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
% - when/if variables like oversampling_rate, etc., get redefined elsewhere 
%	(.C files), try to put them all in one place?
% -- same goes for repeated documentation?
% - explain somewhere the lowpass_cutoff_freq is also Nyquist frequency, aliasing, original
%	sampling rate is 2 times it, and oversampling rate is even a bit more for 
%	FFT frequency resolution reasons and to reduce quantization noise.
% - move this file elsewhere later on?
% - ref somewhere else for note limits?
% - print coefficients in the format for the data type used with the arm_fir_* functions: f32 / q notation?
