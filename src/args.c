#include <argp.h>
#include <error.h>
#include <string.h>
#include <stdlib.h>
#include "args.h"

/* Program information. */
const char *argp_program_version =
        "tcpp 0.1";
const char *argp_program_bug_address =
        "<mrtomaszal@gmail.com>";

/* Program documentation.
 * Passed in the DOC field of the ARGP structure. */
static char doc[] =
        "Tomaszal's C PreProcessor (tcpp) -- a program for preprocessing C computer programming language.";

/* An array of accepted ARGP_OPTION's.
 * Passed in the OPTIONS field of the ARGP structure. */
static struct argp_option options[] = {
        {"verbose",       'v', 0,        0, "Produce verbose output"},
        {"quiet",         'q', 0,        0, "Do not produce any output at all"},
        {"silent",        's', 0, OPTION_ALIAS},
        {"keep_comments", 'c', 0,        0, "Keep the comments instead of removing them"},
        {"input",         'i', "<file>", 0, "Name of the \"*.c\" input <file>"},
        {"output",        'o', "<file>", 0, "Place output into <file>"},
        {0}
};

/* A function that deals with parsing a ARGP_OPTION.
 * Passed in the PARSER field of the ARGP structure. */
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

            break;

        default:
            return ARGP_ERR_UNKNOWN;
    }

    return 0;
}

/* Parse the options strings in ARGC & ARGV into ARGS structure using ARGP. */
void args_parse(int argc, char **argv) {
    struct argp argp = {options, parse_opt, 0, doc};
    args = calloc(1, sizeof *args);
    argp_parse(&argp, argc, argv, 0, 0, args);
}
