#include "note.h"
#include <stdbool.h>

/* TODO if use this struct outside of note_freqs table separate the two. */
struct note_freq {
	const char *note_name;
	float32_t frequency;
} note_freqs[] = {
	/* TODO rm unused? */
	{ "A1",  55 },
	{ "A#1", 58.27 },
	{ "B1",  61.735 },

	{ "C2",  65.406 },
	{ "C#2", 69.296 },
	{ "D2",  73.416 },
	{ "D#2", 77.782 },
	{ "E2",  82.407 },
	{ "F2",  87.307 },
	{ "F#2", 92.499 },
	{ "G2",  97.999 },
	{ "G#2", 103.826 },
	{ "A2",  110 },
	{ "A#2", 116.541 },
	{ "B2",  123.471 },

	{ "C3",  130.813 },
	{ "C#3", 138.591 },
	{ "D3",  146.832 },
	{ "D#3", 155.563 },
	{ "E3",  164.814 },
	{ "F3",  174.614 },
	{ "F#3", 184.997 },
	{ "G3",  195.998 },
	{ "G#3", 207.652 },
	{ "A3",  220 },
	{ "A#3", 233.082 },
	{ "B3",  246.942 },

	{ "C4",  261.626 },
	{ "C#4", 277.183 },
	{ "D4",  293.665 },
	{ "D#4", 311.127 },
	{ "E4",  329.628 },
	{ "F4",  349.228 },
	{ "F#4", 369.994 },
	{ "G4",  391.995 },
	{ "G#4", 415.305 },
	{ "A4",  440 },
	{ "A#4", 466.164 },
	{ "B4",  493.883 },

	{ "C5",  523.251 },
	{ "C#5", 554.365 },
	{ "D5",  587.33 },
	{ "D#5", 622.254 },
	{ "E5",  659.255 },
	{ "F5",  698.456 },
	{ "F#5", 739.989 },
	{ "G5",  783.991 },
	{ "G#5", 830.609 },
	{ "A5",  880 },
	{ "A#5", 932.328 },
	{ "B5",  987.767 },
					 
	{ "C6",  1046.502 },
	{ 0 }
};

/* TODO don't reinvent wheel? take all of this out of core if only used by test? */
static int Strlen(const char *s)
{
	const char *start = s;
	while (*s)
		++s;
	return s-start;
}

static bool Strcmp(const char *s, const char *t)
{
	int n = Strlen(s);
	int m = Strlen(t);

	if (n != m)
		return false;
	for (int i = 0; i < n; ++i) {
		if (s[i] != t[i])
			return false;
	}
	return true;
}

float32_t note_frequency(const char *note_name)
{
	struct note_freq *nf = note_freqs;
	
	for (; nf->note_name; ++nf) {
		if (Strcmp(note_name, nf->note_name)) {
			return nf->frequency;
		}
	}
	return 0;
}
