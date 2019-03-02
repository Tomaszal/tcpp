# Tomaszal's C preprocessor (TCPP)

A C preprocessor implements the macro language used to transform C programs before they are compiled.

This project is an attempt to recreate the essential parts of GCC's built in C preprocessor (CPP). It started out as a simple coursework for the [SET07109](http://www.modules.napier.ac.uk/Module.aspx?ID=set07109) module, but quickly morphed into this 
complex and challenging idea.

[GCC CPP documentation](https://gcc.gnu.org/onlinedocs/cpp/)

## Usage

```
Usage: tcpp [OPTION...]

  -c, --keep_comments        Keep the comments instead of removing them
  -i, --input=<file>         Name of the "*.c" input <file>
  -o, --output=<file>        Place output into <file>
  -q, -s, --quiet, --silent  Do not produce any output at all
  -v, --verbose              Produce verbose output
  -?, --help                 Give this help list
      --usage                Give a short usage message
  -V, --version              Print program version
```

## Building

This project provides several Makefile configurations. It is meant to be compiled and used on a Linux stack (GCC, GNU Make).

```
Usage: make [OPTION]

  make                Compiles the program into a 'tcpp' executable
  make clean          Removes binaries (executables) and object files

  make test_math      Test case 'math_functions.c'
  make test_string    Test case 'string_functions.c'
  make test_both      Both test cases

  make test_math_c    Test case 'math_functions.c' with 'keep_comments'
  make test_string_c  Test case 'string_functions.c' with 'keep_comments'
  make test_both_c    Test both cases with 'keep_comments'
```
