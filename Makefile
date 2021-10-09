CC = gcc
CFLAGS = -Wall -g -O0 -std=c99

SRCS = superblock.c main.c blocks.c

OBJS = $(SRCS:.c=.o)

MAIN = indirect-blocks

.PHONY: clean

all: $(MAIN)

$(MAIN): $(OBJS)
	$(CC) $(CFLAGS) -o $(MAIN) $(OBJS)

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) *.o $(MAIN)
