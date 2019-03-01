CC = gcc

SDIR = src
ODIR = obj

_DEPS = args.h hashmap.h
_SRCS = main.c args.c hashmap.c

DEPS = $(patsubst %,$(SDIR)/%,$(_DEPS))
OBJS = $(patsubst %,$(ODIR)/%,$(_SRCS:.c=.o))

default: tcpp

test_math: tcpp
	./tcpp -i tests/math_functions.c

test_math_c: tcpp
	./tcpp -i tests/math_functions.c -c

test_string: tcpp
	./tcpp -i tests/string_functions.c

test_string_c: tcpp
	./tcpp -i tests/string_functions.c -c

$(ODIR)/%.o: $(SDIR)/%.c $(DEPS)
	@mkdir -p $(@D)
	$(CC) -c -o $@ $<

tcpp: $(OBJS)
	$(CC) -o $@ $^

clean:
	rm -rf $(ODIR) tcpp
