VPATH = src
PROGRAM = antimatter
CFLAGS = -Werror -Wall -Wpedantic -Wextra -fwrapv -std=c17 $(shell pkg-config --cflags sdl2)
OFLAGS = -O3
LDFLAGS = -lm $(shell pkg-config --libs sdl2)
OBJECTS = main.o gamestate.o scene.o sdl_backend.o sprite.o sound.o midi.o

WCC = zig cc
WPROGRAM = antimatter.wasm
WCFLAGS = -Weverything --target=wasm32-wasi -DWASM_BACKEND -std=c17
WOBJECTS = main.wasm gamestate.wasm scene.wasm sprite.wasm wasm_backend.wasm
WAPROGRAM = antimatter_audio.wasm
WAUDIO = wasm_audio.wasm sound.wasm midi.wasm

HEADERS = antimatter.h backend.h gamestate.h level_data.h \
		  scene.h sprite.h sound.h midi.h texture_data.h midi_data.h

$(PROGRAM) : $(OBJECTS) 
	$(CC) $(CFLAGS) $(OFLAGS) $(OBJECTS) $(LDFLAGS) -o $(PROGRAM)

$(OBJECTS) : %.o: %.c $(HEADERS)
	$(CC) -c $(CFLAGS) $(OFLAGS) $< -o $@

$(WPROGRAM) : $(WOBJECTS) $(WAPROGRAM)
	$(WCC) $(WCFLAGS) $(OFLAGS) $(WOBJECTS) -o $(WPROGRAM)

$(WOBJECTS) : %.wasm: %.c $(HEADERS)
	$(WCC) -c $(WCFLAGS) $(OFLAGS) $< -o $@

$(WAPROGRAM) : $(WAUDIO)
	$(WCC) $(WCFLAGS) $(OFLAGS) $(WAUDIO) -o $(WAPROGRAM) 

$(WAUDIO) : %.wasm: %.c $(HEADERS)
	$(WCC) -c $(WCFLAGS) $(OFLAGS) $< -o $@

.PHONY : clean
clean :
	rm -f $(PROGRAM) *.o *.wasm

