#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define ARRAY_LEN(arr) (sizeof(arr) / sizeof(arr[0]))

#define PROGRAM_VERSION "0.2.0"
#ifndef PROGRAM_NAME
#define PROGRAM_NAME    "create-note"
#endif

#define DATE_TAG "{{TODAY}}"

// pre-definitions

typedef struct {
	char* template;
	char** outputs;

	char* date_fmt;
	char* name_fmt;
	bool overwrite;
	int time_offset;
} Options;
typedef char* Arg[2];

void usage(const char* argv0, const char* error);
void run(Options opts);

char* argv0;

// functions

int main(int argc, char* argv[]) {
	if (argc >= 1) argv0 = argv[0];

	// decode argument vector

	int length = 0;
	Arg* args = malloc(8 * sizeof(Arg));

	bool no_more_args = false;

	for (int i = 1; i < argc; i++) {
		if (length % 8 == 7)
			args = realloc(args, (length + 9) * sizeof(Arg));
		
		if (no_more_args || argv[i][0] != '-') {
			char* value = malloc(strlen(argv[i]));
			strcpy(value, argv[i]);

			args[length][0] = NULL;
			args[length][1] = value;

			length++;

			continue;
		}

		if (argv[i][1] == '-') {
			if (argv[i][2] == 0) {
				no_more_args = true;
				continue;
			}

			size_t arg_len = strcspn(argv[i] + 2, "=");

			char* arg = malloc(arg_len + 1);
			strncpy(arg, argv[i] + 2, arg_len);
			arg[arg_len] = 0;

			char* value = NULL;

			if (argv[i][2 + arg_len] == '=') {
				value = malloc(strlen(argv[i] + 3 + arg_len) + 1);
				strcpy(value, argv[i] + 3 + arg_len);
			}

			args[length][0] = arg;
			args[length][1] = value;

			length++;
		} else {
			size_t arg_len = strcspn(argv[i] + 1, "=");

			for (size_t j = 0; j < arg_len; j++) {
				char* arg = malloc(2);
				arg[0] = argv[i][j + 1];
				arg[1] = 0;

				char* value = NULL;

				if (argv[i][j + 2] == '=') {
					value = malloc(strlen(argv[i] + j + 3) + 1);
					strcpy(value, argv[i] + j + 3);
				}

				args[length][0] = arg;
				args[length][1] = value;

				length++;
			}
		}
	}

	args[length][0] = NULL;
	args[length][1] = NULL;
	
	// parse arguments

	char* template = NULL;
	char** outputs = malloc(8 * sizeof(char*));
	int n_outputs = 0;

	bool help = false;
	bool version = false;
	Options opts = {
		.template = NULL,
		.outputs = NULL,
		.date_fmt = "%x",
		.name_fmt = "%Y-%m-%d",
		.overwrite = false,
		.time_offset = 0
	};

	struct {
		char flag_short;
		char* flag_long;
		bool value;
		bool present;
	} options[] = {
		{ 'h', "help", false },
		{ 'v', "version", false },
		{ 'd', "date", true },
		{ 'n', "name", true },
		{ 'O', "overwrite", false },
		{ 't', "tz", true }
	};

	for (Arg* arg = args; (*arg)[0] || (*arg)[1]; arg++) {
		if ((*arg)[0] == NULL) {
			if (template == NULL) {
				template = (*arg)[1];
			} else {
				outputs[n_outputs++] = (*arg)[1];
				if (n_outputs % 8 == 0)
					outputs = realloc(outputs, (n_outputs + 8) * sizeof(char*));
				outputs[n_outputs] = NULL;
			}
		} else {
			for (size_t i = 0; i < ARRAY_LEN(options); i++) {
				if (
					(
						(*arg)[0][1] == 0 &&
						(*arg)[0][0] == options[i].flag_short
					) ||
					strcmp((*arg)[0], options[i].flag_long) == 0
				) {
					if ((*arg)[1] != NULL && !options[i].value) {
						usage(argv0, "unexpected argument value");
						exit(EXIT_FAILURE);
					}

					bool consumed = false;

					if ((*arg)[1] == NULL && options[i].value) {
						if ((*(arg + 1))[0] == NULL && (*(arg + 1))[1] != NULL) {
							(*arg)[1] = (*(arg + 1))[1];
							consumed = true;
						} else {
							usage(argv0, "argument value missing");
							exit(EXIT_FAILURE);
						}
					}

					if (options[i].present) {
						usage(argv0, "duplicate argument");
						exit(EXIT_FAILURE);
					}

					options[i].present = true;

					switch (options[i].flag_short) {
						case 'h':
							help = true;
							break;
						case 'v':
							version = true;
							break;
						case 'd':
							opts.date_fmt = (*arg)[1];
							break;
						case 'n':
							opts.name_fmt = (*arg)[1];
							break;
						case 'O':
							opts.overwrite = true;
							break;
						case 't':
							opts.time_offset = (int)(3600.0 * strtof((*arg)[1], NULL));
							break;
						default:
							abort(); // unreachable
					}

					if (consumed) arg++;

					goto next;
				}
			}

			usage(argv0, "unknown argument");
			exit(EXIT_FAILURE);

			next: {}
		}
	}

	opts.template = template;
	opts.outputs = outputs;

	// main functionality

	if (help) {
		usage(argv0, NULL);
	} else if (version) {
		fprintf(stderr, "%s v%s\n", PROGRAM_NAME, PROGRAM_VERSION);

		fputs("\n", stderr);

		fputs("Written by 19wintersp for Replit task 004.\n", stderr);
		fputs("Licensed under the MIT licence.\n", stderr);
	} else if (template != NULL) {
		run(opts);
	} else {
		usage(argv0, "no template provided");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

void usage(const char* argv0, const char* error) {
	// print error or help with usage
	
	if (argv0 == NULL)
		argv0 = PROGRAM_NAME;

	if (error != NULL)
		fprintf(stderr, "%s: %s\n", argv0, error);

	fprintf(stderr, "Usage: %s [OPTION]... <TEMPLATE> [OUTPUT]...\n", argv0);

	if (error != NULL) {
		fprintf(stderr, "Try '%s --help' for more information\n", argv0);
	} else {
		fputs("Create a copy of TEMPLATE with a name given by OUTPUT.\n", stderr);

		fputs("\n", stderr);

		fputs("If OUTPUT is not specified, today's date is used.\n", stderr);
		fputs("If OUTPUT is a folder, the default name is used within.\n", stderr);

		fputs("\n", stderr);

		fputs("You can modify the program with these options:\n", stderr);
		fputs("  -d, --date=FMT   Specify date format (or local)\n", stderr);
		fputs("  -h, --help       Print this help text\n", stderr);
		fputs("  -n, --name=FMT   Specify name format (or YYYY-MM-DD)\n", stderr);
		fputs("  -O, --overwrite  Overwrite if the target exists\n", stderr);
		fputs("  -t, --tz=HOURS   Time offset from UTC\n", stderr);
		fputs("  -v, --version    Print version information\n", stderr);

		fputs("\n", stderr);

		fputs("The date format uses a few special symbols:\n", stderr);
		fputs("  %Y  Four-digit year\n", stderr);
		fputs("  %y  Two-digit year\n", stderr);
		fputs("  %m  Two-digit month number\n", stderr);
		fputs("  %B  Month name\n", stderr);
		fputs("  %b  Short month name\n", stderr);
		fputs("  %d  Two-digit date number\n", stderr);
		fputs("  %A  Weekday name\n", stderr);
		fputs("  %a  Short weekday name\n", stderr);
		fputs("  %H  Two-digit 24h hour number\n", stderr);
		fputs("  %I  Two-digit 12h hour number\n", stderr);
		fputs("  %M  Two-digit minute number\n", stderr);
		fputs("  %S  Two-digit second number\n", stderr);
		fputs("  %P  AM or PM marker\n", stderr);
		fputs("Non-symbol characters are just copied over.\n", stderr);

		fputs("\n", stderr);

		fprintf(
			stderr,
			"The program exits with %d on success, or %d on failure.\n",
			EXIT_SUCCESS, EXIT_FAILURE
		);
	}
}

const char* file_name(const char* path) {
	// return file name part of path

	char* file = strrchr(path, '/');
	return file == NULL ? path : file + 1;
}

const char* file_extension(const char* path) {
	// return extension part of path with "."

	const char* file = file_name(path);
	char* ext = strrchr(file, '.');
	return ext == file ? NULL : ext;
}

void copy(const char* src_path, const char* dest_path, const char* date) {
	struct stat file;
	stat(src_path, &file);

	int src = open(src_path, O_RDONLY);
	int dest = open(dest_path, O_WRONLY | O_CREAT, file.st_mode);

	if (src == -1 || dest == -1) {
		usage(argv0, strerror(errno));
		exit(EXIT_FAILURE);
	}

	// loading the whole file into memory is painfully inefficient (I would prefer
	// to use a streaming replacement) but it's easier and faster, and the memory
	// usage of a program of this sort isn't a great concern.

	char* buf = malloc(file.st_size + 1);
	buf[file.st_size] = 0;

	ssize_t n, sum = 0;

	while ((n = read(src, buf + sum, 4096)) > 0) sum += n;
	if (n == -1) goto copy_error;

	char* ptr = buf;

	for (;;) {
		char* tag = strstr(ptr, DATE_TAG);
		if (tag == NULL) break;

		if (write(dest, ptr, tag - ptr) == -1) goto copy_error;
		if (write(dest, date, strlen(date)) == -1) goto copy_error;

		ptr = tag + strlen(DATE_TAG);
	}

	n = write(dest, ptr, strlen(ptr));

	if (n == -1) {
copy_error:
		usage(argv0, strerror(errno));
		exit(EXIT_FAILURE);
	}

	close(src);
	close(dest);
}

void run(Options opts) {
	struct stat file;

	time_t c_time = time(NULL) + opts.time_offset;
	const struct tm* c_tm = gmtime(&c_time);

	size_t date_len = strftime(NULL, -1, opts.date_fmt, c_tm); // ub?
	char* date = calloc(date_len + 1, sizeof(char));
	strftime(date, date_len + 1, opts.date_fmt, c_tm);

	size_t time_len = strftime(NULL, -1, opts.name_fmt, c_tm); // ub?
	char* time = calloc(time_len + 1, sizeof(char));
	strftime(time, time_len + 1, opts.name_fmt, c_tm);

	const char* ext = file_extension(opts.template);
	if (ext == NULL) ext = "";
	size_t ext_len = strlen(ext);

	if (ext_len > 1023) {
		usage(argv0, "file extension is too long");
		exit(EXIT_FAILURE);
	}

	char* default_name = calloc(time_len + ext_len + 1, sizeof(char));
	snprintf(default_name, time_len + ext_len + 1, "%s%s", time, ext);

	if (stat(opts.template, &file) == -1) {
		usage(argv0, strerror(errno));
		exit(EXIT_FAILURE);
	}

	char** output = opts.outputs;
	for (; *output != NULL; output++) {
		if (stat(*output, &file) == 0) {
			if (S_ISDIR(file.st_mode)) {
				size_t dest_len = strlen(*output) + 1 + strlen(default_name);
				char* dest = calloc(dest_len + 1, sizeof(char));

				char* slash = (*output)[strlen(*output) - 1] != '/' ? "/" : "";
				snprintf(dest, dest_len + 1, "%s%s%s", *output, slash, default_name);

				copy(opts.template, dest, date);
			} else if (opts.overwrite) {
				copy(opts.template, *output, date);
			} else {
				usage(argv0, "target already exists");
				exit(EXIT_FAILURE);
			}
		} else if (errno == ENOENT) {
			copy(opts.template, *output, date);
		} else {
			usage(argv0, strerror(errno));
			exit(EXIT_FAILURE);
		}
	}

	if (output == opts.outputs) {
		if (stat(default_name, &file) == 0 && !opts.overwrite) {
			char* incremented_name;
			unsigned int i = 0;

			do {
				i++;

				if (incremented_name != NULL) free(incremented_name);

				size_t length = snprintf(NULL, 0, "%s-%u%s", time, i, ext);
				incremented_name = calloc(length + 1, sizeof(char));
				snprintf(incremented_name, length + 1, "%s-%u%s", time, i, ext);
			} while (stat(incremented_name, &file) == 0);

			copy(opts.template, incremented_name, date);
		} else {
			copy(opts.template, default_name, date);
		}
	}
}
