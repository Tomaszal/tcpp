#include <argp.h>
#include <error.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include "args.h"

const char *argp_program_version =
        "tcpp 0.2";
const char *argp_program_bug_address =
        "<mrtomaszal@gmail.com>";

/**
 * Program documentation.
 *
 * Passed into the DOC field of the ARGP structure.
 */
static char doc[] =
        "Tomaszal's C preprocessor (TCPP) -- a program for preprocessing C computer programming language.";

/**
 * An array of accepted ARGP_OPTION's.
 *
 * Passed into the OPTIONS field of the ARGP structure.
 */
static struct argp_option options[] = {
        {"verbose",       'v', 0,        0, "Produce verbose output"},
        {"quiet",         'q', 0,        0, "Do not produce any output at all"},
        {"silent",        's', 0, OPTION_ALIAS},
        {"keep_comments", 'c', 0,        0, "Keep the comments instead of removing them"},
        {"input",         'i', "<file>", 0, "Name of the \"*.c\" input <file>"},
        {"output",        'o', "<file>", 0, "Place output into <file>"},
        {0}
};

/**
 * Parses an ARGP_OPTION.
 *
 * Passed into the PARSER field of the ARGP structure.
 *
 * @param key A key associated with an option.
 * @param arg An argument associated with the key.
 * @param state The current state of argument parsing.
 * @return 0 if successful, error code otherwise.
 */
static error_t parse_opt(int key, char *arg, struct argp_state *state) {
    Arguments *args = state->input;

    switch (key) {
        case 'v':
            args->verbose = 1;
            break;
        case 'q':
        case 's':
            args->quiet = 1;
            break;

        case 'c':
            args->keep_comments = 1;
            break;

        case 'i':
            args->input_file = arg;
            break;
        case 'o':
            args->output_file = arg;
            break;

        case ARGP_KEY_ARG:
            argp_usage(state);
            break;
        case ARGP_KEY_END:
            if (!args->input_file) {
                fprintf(stderr, "No input file specified.\n\n");
                argp_usage(state);
            } else {
                size_t length = strlen(args->input_file);

                if (length <= 2 || args->input_file[length - 2] != '.' || args->input_file[length - 1] != 'c') {
                    fprintf(stderr, "Wrong C input file format (\"*.c\" expected).\n\n");
                    argp_usage(state);
                }

                if (!args->output_file) {
                    args->output_file = malloc((length + 1) * (sizeof *args->output_file));
                    strcpy(args->output_file, args->input_file);
                    args->output_file[length - 1] = 'o';
                }
            }

            if (args->verbose && args->quiet) {
                fprintf(stderr, "Cannot be verbose and quiet at the same time.\n\n");
                argp_usage(state);
            }

            break;

        default:
            return ARGP_ERR_UNKNOWN;
    }

    return 0;
}

/**
 * Verbose version of printf. Writes formatted output to stdout only if ACTIVE argument is true.
 *
 * Adapted from StackOverflow answer:
 * https://stackoverflow.com/a/36096037/7353602
 *
 * @param format A format specifier string.
 * @param ... Additional arguments.
 * @return 0 if suppressed. A total number of characters written if successful. A negative number is returned.
 */
int verbose_printf(const char *restrict format, ...) {
    if (!args->verbose) {
        return 0;
    }

    va_list ap;
    va_start(ap, format);
    int ret = vprintf(format, ap);
    va_end(ap);

    return ret;
}

/**
 * Quiet version of printf. Writes formatted output to stdout only if QUIET argument is false.
 *
 * Adapted from StackOverflow answer:
 * https://stackoverflow.com/a/36096037/7353602
 *
 * @param format A format specifier string.
 * @param ... Additional arguments.
 * @return 0 if suppressed. A total number of characters written if successful. A negative number is returned.
 */
int normal_printf(const char *restrict format, ...) {
    if (args->quiet) {
        return 0;
    }

    va_list ap;
    va_start(ap, format);
    int ret = vprintf(format, ap);
    va_end(ap);

    return ret;
}

/**
 * Parses the option strings into global arguments array ARGS.
 *
 * @param argc Argument count.
 * @param argv Argument vector.
 */
void args_parse(int argc, char **argv) {
    struct argp argp = {options, parse_opt, 0, doc};
    args = calloc(1, sizeof *args);
    argp_parse(&argp, argc, argv, 0, 0, args);
}
