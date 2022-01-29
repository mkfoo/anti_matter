PROGRAM = bundler

CFLAGS = -Werror -Wall -Wpedantic -Wextra -fwrapv -std=c17

OFLAGS = -O0

LDFLAGS = -lm

OBJECTS = main.o

$(PROGRAM) : $(OBJECTS)
	$(CC) $(CFLAGS) $(OFLAGS) $(OBJECTS) $(LDFLAGS) -o $(PROGRAM)

$(OBJECTS) : %.o: %.c
	$(CC) -c $(CFLAGS) $(OFLAGS) $< -o $@

.PHONY : clean
clean :
	rm -f $(PROGRAM) $(OBJECTS)
