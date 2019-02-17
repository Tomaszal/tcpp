CC = gcc

SDIR = src
ODIR = obj

_DEPS = args.h
_SRCS = main.c args.c

DEPS = $(patsubst %,$(SDIR)/%,$(_DEPS))
OBJS = $(patsubst %,$(ODIR)/%,$(_SRCS:.c=.o))

all: tcpp

$(ODIR)/%.o: $(SDIR)/%.c $(DEPS)
	@mkdir -p $(@D)
	$(CC) -c -o $@ $<

tcpp: $(OBJS)
	$(CC) -o $@ $^

.PHONY: clean

clean:
	rm -rf $(ODIR) tcpp
