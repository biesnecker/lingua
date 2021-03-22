CC=clang
BINARY_NAME=lingua
ERRFLAGS=-Werror -Wall -Wextra -Wstrict-prototypes
CFLAGS=-std=c17 -O2 -march=native $(ERRFLAGS)
LDLIBS=-lcurl -lpthread

OBJS=\
	src/array.o \
	src/buffer.o \
	src/main.o \
	src/ratelimiter.o

HEADERS=\
	src/macros.h \
	src/ratelimiter.h

%.o: %.c $(HEADERS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(BINARY_NAME): $(OBJS)
	$(CC) -o $@ $^ $(CFLAGS) $(LDLIBS)

.PHONY: clean
clean:
	find . -name '*.[oa]' -exec rm -f {} ';'
	rm -f $(BINARY_NAME)