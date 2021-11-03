PROGRAM = antimatter

VPATH = src

CFLAGS = -Werror -Wall -Wpedantic -Wextra -fanalyzer -std=c17 $(shell pkg-config --cflags sdl2)

OFLAGS = -O0

LDFLAGS = -lm $(shell pkg-config --libs sdl2)

OBJECTS = main.o gamestate.o sprite.o sdl_backend.o

HEADERS = antimatter.h backend.h gamestate.h level_data.h sprite.h

$(PROGRAM) : $(OBJECTS)
	$(CC) $(CFLAGS) $(OFLAGS) $(OBJECTS) $(LDFLAGS) -o $(PROGRAM)

$(OBJECTS) : %.o: %.c $(HEADERS)
	$(CC) -c $(CFLAGS) $(OFLAGS) $< -o $@

.PHONY : clean
clean :
	rm -f $(PROGRAM) $(OBJECTS)