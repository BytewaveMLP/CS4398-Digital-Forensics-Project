CC = gcc
CFLAGS = -Wall -g -O2 -std=c99

SRCS = superblock.c main.c

OBJS = $(SRCS:.c=.o)

MAIN = superblock

.PHONY: clean

all: $(MAIN)

$(MAIN): $(OBJS)
	$(CC) $(CFLAGS) -o $(MAIN) $(OBJS)

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) *.o $(MAIN)
