Recordings of open string guitar notes (tuned relative to standard) for
testing. Open string notes are used because the main use case of the
tuner is to get the open strings in tune. See comments at '../../../core/note.c:note_freqs'
for more info on the range of notes being tested.

Each file contains digital audio of a single note on the guitar, 
the note being named in the filename. File format is 4 frames (frame length
4096 samples) of raw (headerless), uncompressed, mono, 4000 Hz sample rate, 
signed 16-bit PCM audio.

TODO
- explain why the sample rates / bit width are why they are (bit width
because of the ADC 12-bit of the Pico?)
- note.c:note_freqs inline code with `?
