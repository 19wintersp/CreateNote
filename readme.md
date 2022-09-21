# CreateNote

**A CLI tool that can create files from a template.**

Program to create pre-filled files from a template named with the day's date,
created for Replit as part of Task 004.

Licensed under the MIT licence.

## User documentation

`create-note` is a simple program that will copy the given template file to the
outputs, by default with an automatically-generated name based on today's date,
and making subtitutions within the file with this formatted date. `create-note`
is a CLI, and so is controlled by flags appended to the command when run. These
flags have two ways in which they can be used: either with a single-letter flag
(as in `-d`) or a long-form flag (as in `--date`). Multiple short flags can be
combined together, so `-dnO` is equivalent to `-d -n -O`. Some flags also take
values, which can be provided either with or without an equals - note that some
values (such as negative numbers) must be expressed using the equals-based
syntax to distinguish them from flags.

The flags are as follows:
- `-d`, `--date`: takes a date format as a value, and specifies the format for
  the substituted date
- `-h`, `--help`: displays the basic help information, overriding all other
  flags
- `-n`, `--name`: takes a date format as a value, and specifies the format for
  the date used in the file name by default
- `-O`, `--overwrite`: when the specified output file (or default output file)
  already exists, overwrite it; otherwise, the program will exit with an error
  or add an incremented index if the default output is used
- `-t`, `--tz`: when formatting dates, offset the computer timezone by this
  number of hours
- `-v`, `--version`: displays the basic version and licensing information,
  overriding all other flags

### Output

`create-note` has positional arguments to control its main function. The first
non-flag value is used as the name of the template file. It is required, and is
used as the source. Any other positional arguments are interpreted as the output
targets. When no output targets are specified, the default output target is
selected. It is constructed of the current date, formatted according to the `-n`
or `--name` flag (or YYYY-MM-DD if not specified) and with the file extension of
the source file appended. If the default output target already exists, and the
overwrite flag is not given, the first available incrementing index is appended
with a hyphen after the date. When outputs are explicitly given, the default
output target is ignored, and the source is copied to the given outputs instead.
If any of the given outputs already exist, they will not be overwritten and the
program will exit with an error unless the overwrite flag is given. If an output
is a directory, the default output target filename will be used within that
directory.

### Substitution

Whilst copying from the source file, the content of the file is scanned for
special substitution tags. Currently, the only tag is the `{{TODAY}}` tag, which
will be replaced with the current date and time formatted according to the `-d`
or `--date` flag.

### Date format

The date format system directly uses the C library function `strftime` - for
full formatting information, check its documentation.

Simply, date format strings are made up of regular characters as well as certain
special format items, which start with percent (`%`) characters. To insert an
actual percent character without it being interpreted as a format item, use two.
The regular characters that are not parts of format items are simply copied to
the output.

A small set of the format items are as follows:
- `%Y`: Four-digit year
- `%y`: Two-digit year
- `%m`: Two-digit month number
- `%B`: Month name
- `%b`: Short month name
- `%d`: Two-digit date number
- `%A`: Weekday name
- `%a`: Short weekday name
- `%H`: Two-digit 24h hour number
- `%I`: Two-digit 12h hour number
- `%M`: Two-digit minute number
- `%S`: Two-digit second number
- `%P`: AM or PM marker

For example, the format string `Hello at %I:%M%P on %B %e, %Y!` could produce the
output "Hello at 3:21pm on September 21, 2022!".
