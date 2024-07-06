CC = gcc
CFLAGS = -g -Wall
SRCMODULES = read_commands.c
OBJMODULES = $(SRCMODULES:.c=.o)

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $< -o $@

shell: main.c $(OBJMODULES)
	$(CC) $(CFLAGS) $^ -o $@
