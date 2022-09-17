#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARRAY_LEN(arr) (sizeof(arr) / sizeof(arr[0]))

// pre-definitions

typedef struct {
	char* template;
	char** outputs;

	char* date_fmt;
	bool force_file;
	bool overwrite;
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
	Options opts = { 0 };

	struct {
		char flag_short;
		char* flag_long;
		bool value;
		bool present;
	} options[] = {
		{ 'h', "help", false },
		{ 'd', "date", true },
		{ 'f', "file", false },
		{ 'o', "overwrite", false }
	};

	for (Arg* arg = args; (*arg)[0] || (*arg)[1]; arg++) {
		if ((*arg)[0] == NULL) {
			if (template == NULL) {
				template = (*arg)[0];
			} else {
				outputs[n_outputs++] = (*arg)[0];
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
					if (((*arg)[1] == NULL) == options[i].value) {
						usage(argv0, "argument value mismatch");
						exit(EXIT_FAILURE);
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
						case 'd':
							opts.date_fmt = (*arg)[1];
							break;
						case 'f':
							opts.force_file = true;
							break;
						case 'o':
							opts.overwrite = true;
							break;
						default:
							abort(); // unreachable
					}

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
		argv0 = "create-note";

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
		fputs("If OUTPUT has no extension, it is created as a folder.\n", stderr);

		fputs("\n", stderr);

		fputs("You can modify the program with these options:\n", stderr);
		fputs("  -d, --date=FMT   Specify date format (or YYYY-MM-DD)\n", stderr);
		fputs("  -f, --file       Never assume OUTPUT is a folder\n", stderr);
		fputs("  -o, --overwrite  Overwrite if the target exists\n", stderr);

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
	}
}
