CFLAGS = -O2 -std=c99

SRCS = superblock.c main.c blocks.c
OBJS = $(SRCS:.c=.o)
MAIN = indirect-blocks

.PHONY: clean zip

all: $(MAIN)

debug: CFLAGS = -Wall -Wextra -Wpedantic -g -O0 -std=c99 -DDEBUG
debug: $(MAIN)

zip: screenshots/* debug
	zip -r cs4398-project-part$${PARTNO:?}-group22.zip *.c *.h Makefile README.md screenshots/

$(MAIN): $(OBJS)
	$(CC) $(CFLAGS) -o $(MAIN) $(OBJS)

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) *.o $(MAIN) *.zip
