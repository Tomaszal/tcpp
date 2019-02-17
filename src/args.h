#ifndef TCPP_ARGS_H
#define TCPP_ARGS_H

typedef struct Arguments {
    int verbose;
    int quiet;

    int keep_comments;

    char *input_file;
    char *output_file;
} Arguments;

Arguments *args;

void args_parse(int argc, char **argv);

int verbose_printf(const char *restrict format, ...);
int normal_printf(const char *restrict format, ...);

#endif //TCPP_ARGS_H
