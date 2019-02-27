# Tomaszal's C preprocessor (TCPP)

A C preprocessor implements the macro language used to transform C programs before they are compiled.

This project is an attempt to recreate the essential parts of GCC's built in C preprocessor (CPP). It started out as a simple coursework for the [SET07109](http://www.modules.napier.ac.uk/Module.aspx?ID=set07109) module, but quickly morphed into this 
complex and challenging idea.

[GCC CPP documentation](https://gcc.gnu.org/onlinedocs/cpp/)

```
Usage: tcpp [OPTION...]
Tomaszal's C preprocessor (TCPP) -- a program for preprocessing C computer
programming language.

  -c, --keep_comments        Keep the comments instead of removing them
  -i, --input=<file>         Name of the "*.c" input <file>
  -o, --output=<file>        Place output into <file>
  -q, -s, --quiet, --silent  Do not produce any output at all
  -v, --verbose              Produce verbose output
  -?, --help                 Give this help list
      --usage                Give a short usage message
  -V, --version              Print program version

Mandatory or optional arguments to long options are also mandatory or optional
for any corresponding short options.

Report bugs to <mrtomaszal@gmail.com>.
```
