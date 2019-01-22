#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ESP8266TrueRandom.h>

#define PW_DIGITS	0x0001	/* At least one digit */
#define PW_UPPERS	0x0002	/* At least one upper letter */
#define PW_SYMBOLS	0x0004
#define PW_AMBIGUOUS	0x0008
#define PW_NO_VOWELS	0x0010

const char *pw_digits = "0123456789";
const char *pw_uppers = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
const char *pw_lowers = "abcdefghijklmnopqrstuvwxyz";
const char *pw_symbols = "!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~";
const char *pw_ambiguous = "B8G6I1l0OQDS5Z2";
const char *pw_vowels = "01aeiouyAEIOUY";

int pw_random_number(int max_num) {
    return ESP8266TrueRandom.random(max_num);
}

bool pw_rand(char *buf, int size, int pw_flags)
{
	char		ch, *chars, *wchars;
	int		i, len, feature_flags;

	len = 0;
	if (pw_flags & PW_DIGITS) {
		len += strlen(pw_digits);
	}
	if (pw_flags & PW_UPPERS) {
		len += strlen(pw_uppers);
	}
	len += strlen(pw_lowers);
	if (pw_flags & PW_SYMBOLS) {
		len += strlen(pw_symbols);
	}
    chars = (char*) malloc(len+1);
    if (!chars) {
		Serial.printf("Couldn't malloc pw_rand buffer.\n");
        return false;
	}
	wchars = chars;
	if (pw_flags & PW_DIGITS) {
		strcpy(wchars, pw_digits);
		wchars += strlen(pw_digits);
	}
	if (pw_flags & PW_UPPERS) {
		strcpy(wchars, pw_uppers);
		wchars += strlen(pw_uppers);
	}
	strcpy(wchars, pw_lowers);
	wchars += strlen(pw_lowers);
	if (pw_flags & PW_SYMBOLS) {
		strcpy(wchars, pw_symbols);
	}
try_again:
	len = strlen(chars);
	feature_flags = pw_flags;
	i = 0;
	while (i < size) {
		ch = chars[pw_random_number(len)];
		if ((pw_flags & PW_AMBIGUOUS) && strchr(pw_ambiguous,ch))
			continue;
		if ((pw_flags & PW_NO_VOWELS) && strchr(pw_vowels, ch))
			continue;
		buf[i++] = ch;
		if (strchr(pw_digits, ch))
			feature_flags &= ~PW_DIGITS;
		if (strchr(pw_uppers, ch))
			feature_flags &= ~PW_UPPERS;
		if (strchr(pw_symbols, ch))
			feature_flags &= ~PW_SYMBOLS;
	}
	if (feature_flags & (PW_UPPERS | PW_DIGITS | PW_SYMBOLS))
		goto try_again;
	buf[size] = 0;
	free(chars);
	return true;
}

bool generate_password(char *buffer) {
    return pw_rand(buffer, 10, PW_DIGITS | PW_UPPERS);
}