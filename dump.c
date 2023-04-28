/*
	Note that dumping ascii "hardcodedly" like this is very suboptimal as it depends on the
	keyboard mapping of each system. The ascii dumpers below are optimized for Brazilian
	keyboards.
*/

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <linux/input.h>

#include "dump.h"

#define MAX_LINE 64
#define FMT_LINE "[%zu.%zu] type %u, value %i, code %u, ascii '%c'\n"

#define KEY_RELEASED 0
#define KEY_PRESSED  1
#define KEY_REPEATED 2

#define IS_LOWER(c)  (c >= 0x61 && c <= 0x7a)
#define IS_UPPER(c)  (c >= 0x41 && c <= 0x5a)
#define IS_DIGIT(c)  (c >= 0x30 && c <= 0x39)

#define UPPERCASE(c) (c - 32)
#define LOWERCASE(c) (c + 32)


static int _keycode_ascii(int kc)
{
	switch (kc) {
		case KEY_1:            return '1';  case KEY_2:            return '2';
		case KEY_3:            return '3';  case KEY_4:            return '4';
		case KEY_5:            return '5';  case KEY_6:            return '6';
		case KEY_7:            return '7';  case KEY_8:            return '8';
		case KEY_9:            return '9';  case KEY_0:            return '0';
		case KEY_MINUS:        return '-';  case KEY_EQUAL:        return '=';
		case KEY_BACKSPACE:    return '\b'; case KEY_TAB:          return '\t';
		case KEY_Q:            return 'q';  case KEY_W:            return 'w';
		case KEY_E:            return 'e';  case KEY_R:            return 'r';
		case KEY_T:            return 't';  case KEY_Y:            return 'y';
		case KEY_U:            return 'u';  case KEY_I:            return 'i';
		case KEY_O:            return 'o';  case KEY_P:            return 'p';
		case KEY_LEFTBRACE:    return '[';  case KEY_RIGHTBRACE:   return ']';
		case KEY_ENTER:        return '\n'; case KEY_A:            return 'a';
		case KEY_S:            return 's';  case KEY_D:            return 'd';
		case KEY_F:            return 'f';  case KEY_G:            return 'g';
		case KEY_H:            return 'h';  case KEY_J:            return 'j';
		case KEY_K:            return 'k';  case KEY_L:            return 'l';
		case KEY_SEMICOLON:    return ';';  case KEY_APOSTROPHE:   return '\'';
		case KEY_GRAVE:        return '`';  case KEY_BACKSLASH:    return '\\';
		case KEY_Z:            return 'z';  case KEY_X:            return 'x';
		case KEY_C:            return 'c';  case KEY_V:            return 'v';
		case KEY_B:            return 'b';  case KEY_N:            return 'n';
		case KEY_M:            return 'm';  case KEY_COMMA:        return ',';
		case KEY_DOT:          return '.';  case KEY_SLASH:        return '/';
		case KEY_KPASTERISK:   return '*';  case KEY_SPACE:        return ' ';
		case KEY_KP7:          return '7';  case KEY_KP8:          return '8';
		case KEY_KP9:          return '9';  case KEY_KPMINUS:      return '-';
		case KEY_KP4:          return '4';  case KEY_KP5:          return '5';
		case KEY_KP6:          return '6';  case KEY_KPPLUS:       return '+';
		case KEY_KP1:          return '1';  case KEY_KP2:          return '2';
		case KEY_KP3:          return '3';  case KEY_KP0:          return '0';
		case KEY_KPDOT:        return '.';  case KEY_KPENTER:      return '\n';
		case KEY_KPSLASH:      return '/';  case KEY_KPEQUAL:      return '=';
		case KEY_KPCOMMA:      return ',';  case KEY_KPLEFTPAREN:  return '(';
		case KEY_KPRIGHTPAREN: return ')';
	}

	return -1;
}

static int _keycode_ascii_shift(int kc)
{
	switch (kc) {
		case KEY_1:            return '!';  case KEY_2:            return '@';
		case KEY_3:            return '#';  case KEY_4:            return '$';
		case KEY_5:            return '%';  case KEY_6:            return '6';
		case KEY_7:            return '&';  case KEY_8:            return '*';
		case KEY_9:            return '(';  case KEY_0:            return ')';
		case KEY_MINUS:        return '_';  case KEY_EQUAL:        return '+';
		case KEY_BACKSPACE:    return '\b'; case KEY_TAB:          return '\t';
		case KEY_Q:            return 'Q';  case KEY_W:            return 'W';
		case KEY_E:            return 'E';  case KEY_R:            return 'E';
		case KEY_T:            return 'T';  case KEY_Y:            return 'Y';
		case KEY_U:            return 'U';  case KEY_I:            return 'I';
		case KEY_O:            return 'O';  case KEY_P:            return 'P';
		case KEY_LEFTBRACE:    return '{';  case KEY_RIGHTBRACE:   return '}';
		case KEY_ENTER:        return '\n'; case KEY_A:            return 'A';
		case KEY_S:            return 'S';  case KEY_D:            return 'D';
		case KEY_F:            return 'F';  case KEY_G:            return 'G';
		case KEY_H:            return 'H';  case KEY_J:            return 'J';
		case KEY_K:            return 'K';  case KEY_L:            return 'L';
		case KEY_SEMICOLON:    return ':';  case KEY_APOSTROPHE:   return '"';
		case KEY_GRAVE:        return '`';  case KEY_BACKSLASH:    return '|';
		case KEY_Z:            return 'Z';  case KEY_X:            return 'X';
		case KEY_C:            return 'C';  case KEY_V:            return 'V';
		case KEY_B:            return 'B';  case KEY_N:            return 'N';
		case KEY_M:            return 'M';  case KEY_COMMA:        return '<';
		case KEY_DOT:          return '>';  case KEY_SLASH:        return '?';
		case KEY_KPASTERISK:   return '*';  case KEY_SPACE:        return ' ';
		case KEY_KP7:          return '7';  case KEY_KP8:          return '8';
		case KEY_KP9:          return '9';  case KEY_KPMINUS:      return '-';
		case KEY_KP4:          return '4';  case KEY_KP5:          return '5';
		case KEY_KP6:          return '6';  case KEY_KPPLUS:       return '+';
		case KEY_KP1:          return '1';  case KEY_KP2:          return '2';
		case KEY_KP3:          return '3';  case KEY_KP0:          return '0';
		case KEY_KPDOT:        return '.';  case KEY_KPENTER:      return '\n';
		case KEY_KPSLASH:      return '/';  case KEY_KPEQUAL:      return '=';
		case KEY_KPCOMMA:      return ',';  case KEY_KPLEFTPAREN:  return '(';
		case KEY_KPRIGHTPAREN: return ')';
	}

	return -1;
}

ssize_t dump_raw(int kbd, int out, int end)
{
	ssize_t ret;
	struct input_event ie = {.code = 0};

	for (ret = 0; ie.code != end && read(kbd, &ie, sizeof(ie)) == sizeof(ie); ++ret) {
		// Ignore non-key events
		if (ie.type != EV_KEY)
			continue;

		// Ignore key releases except for shifts
		if (ie.value == KEY_RELEASED && ie.code != KEY_LEFTSHIFT && ie.code != KEY_RIGHTSHIFT)
			continue;

		write(out, &ie, sizeof(ie));
	}

	return ret;
}

ssize_t dump_ascii(int kbd, int out, int end)
{
	ssize_t ret;

	char key;
	char capslock = 0, shift = 0;

	struct input_event ie = {.code = 0};

	int (*dumper)(int);

	for (ret = 0; ie.code != end && read(kbd, &ie, sizeof(ie)) == sizeof(ie); ++ret) {
		// Ignore non-key events
		if (ie.type != EV_KEY)
			continue;

		// Handle capitalization
		if (ie.code == KEY_CAPSLOCK && ie.value == KEY_PRESSED)
			capslock ^= 1;
		else
		if (ie.code == KEY_LEFTSHIFT || ie.code == KEY_RIGHTSHIFT) {
			if (ie.value == KEY_PRESSED)
				shift = 1;
			else
			if (ie.value == KEY_RELEASED)
				shift = 0;
		}

		// Ignore releases of non-modifier keys
		if (ie.value == KEY_RELEASED)
			continue;

		dumper = (shift) ? _keycode_ascii_shift : _keycode_ascii;

		if ((key = dumper(ie.code)) >= 0) {
			if (capslock) {
				key = (
					IS_LOWER(key) ? UPPERCASE(key) :
					IS_UPPER(key) ? LOWERCASE(key) : key
				);
			}

			write(out, &key, 1);
		}
	}

	return ret;
}

ssize_t dump_log(int kbd, int out, int end)
{
	ssize_t ret;

	char key;
	char cl = 0, ls = 0, rs = 0;

	char line[MAX_LINE];

	struct input_event ie = {.code = 0};

	for (ret = 0; ie.code != end && read(kbd, &ie, sizeof(ie)) == sizeof(ie); ++ret) {
		// Ignore non-key events
		if (ie.type != EV_KEY)
			continue;

		// Handle capitalization
		if (ie.code == KEY_CAPSLOCK && ie.value == KEY_PRESSED)
			cl ^= 1;
		else
		if (ie.code == KEY_LEFTSHIFT) {
			if (ie.value == KEY_PRESSED)
				rs = 1;
			else
			if (ie.value == KEY_RELEASED)
				rs = 0;
		}
		else
		if (ie.code == KEY_RIGHTSHIFT) {
			if (ie.value == KEY_PRESSED)
				rs = 1;
			else
			if (ie.value == KEY_RELEASED)
				rs = 0;
		}

		// Ignore key releases
		if (ie.value == KEY_RELEASED)
			continue;

		// Dump log line
		if ((key = _keycode_ascii(ie.code)) >= 0) {
			if (cl || ls || rs)
				key = UPPERCASE(key);

			snprintf(
				line, MAX_LINE, FMT_LINE,
				ie.time.tv_sec, ie.time.tv_usec, ie.type, ie.value, ie.code, key
			);

			write(out, line, strlen(line));
		}
	}

	return ret;
}
