#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <linux/input.h>

#include "dump.h"

#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS 0
#endif

#ifndef EXIT_FAILURE
#define EXIT_FAILURE 1
#endif

#define KEYCODE_END KEY_F10   // Keycode to terminate
#define DUMP_DEFAULT dump_raw // Default dump function

#define LOGFILE_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH) // Mode for the log file

const char banner[] =
"kbdump [options] [device|dump]\n"
"	Dump keys from keyboard devices\n\n"

"	-h              help\n"
"	-s              enable stopping the dump when F10 is pressed\n"
"	-o [file]       dump to [file] instead of stdout\n"
"	-t [secs]       print timestamps on ascii dumps after [secs] seconds of idleness\n"
"	-f [ascii|log]  format events in ascii or log lines\n";

int main(int argc, char **argv)
{
	int opt;
	int ret = 0;
	int end = -1;
	int ts_interval = -1;
	int kbd = 0, log = STDOUT_FILENO;

	ssize_t (*dump)(int, int , int, int) = DUMP_DEFAULT;

	while ((opt = getopt(argc, argv, ":hsot:f:")) != -1) {
		switch (opt) {
			case 'h':
				ret = EXIT_SUCCESS;
				fputs(banner, stderr);

				goto fd_close;

			case 's':
				end = KEYCODE_END;
				break;

			case 'o':
				if ((log = open(optarg, O_CREAT | O_WRONLY, LOGFILE_MODE)) < 0) {
					fprintf(stderr, "[err] Failed to open '%s", optarg);
					perror("'");

					ret = EXIT_FAILURE;
					goto fd_close;
				}

				break;

			case 't':
				if ((ts_interval = strtol(optarg, NULL, 10)) < 0) {
					fprintf(stderr, "Invalid timestamp interval: '%s'\n", optarg);

					ret = EXIT_FAILURE;
					goto fd_close;
				}

				break;

			case 'f':
				if (! strcmp("ascii", optarg))
					dump = dump_ascii;
				else
				if (! strcmp("log", optarg))
					dump = dump_log;
				else {
					fputs(banner, stderr);

					ret = EXIT_FAILURE;
					goto fd_close;
				}

				break;

			case ':':
				fprintf(
					stderr,
					"[err] Option '%c' requires an argument\n", optopt
				);

				ret = EXIT_FAILURE;
				goto fd_close;

			case '?':
				fprintf(stderr, "[err] Invalid option: '%c'\n", optopt);

				ret = EXIT_FAILURE;
				goto fd_close;
		}
	}

	if (optind == argc)
		kbd = STDIN_FILENO;
	else
	if (optind == argc - 1) {
		if ((kbd = open(argv[optind], O_RDONLY)) < 0) {
			fprintf(stderr, "Failed to open '%s", argv[optind]);
			perror("'");

			ret = EXIT_FAILURE;
			goto fd_close;
		}
	}
	else {
		fputs(banner, stderr);

		ret = EXIT_FAILURE;
		goto fd_close;
	}

	dump(kbd, log, end, ts_interval);

fd_close:
	// Redundant but consistent
	if (kbd != STDIN_FILENO && kbd > 0)
		close(kbd);

	if (log != STDOUT_FILENO && log > 0)
		close(log);

	return ret;
}
