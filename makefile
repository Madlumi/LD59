
NAME ?= LD59
WOUT  ?= LD59
LOUT  ?= out

CFLAGS_NATIVE := `sdl2-config --cflags`
LIBS_NATIVE   := `sdl2-config --libs` -lSDL2_image -lm


linux:
	@mkdir -p $(LOUT)
	gcc src/main.c $(CFLAGS_NATIVE) $(LIBS_NATIVE) -o $(LOUT)/$(NAME)

run: linux
	./$(LOUT)/$(NAME)
	
wasm:
	@#emcc fuckery
	@mkdir -p build $(WOUT)
	emcc -c src/main.c -o build/$(NAME).o -sUSE_SDL=2
	emcc build/$(NAME).o -o $(WOUT)/$(NAME).js -sUSE_SDL=2 -sALLOW_MEMORY_GROWTH=1 -sEXIT_RUNTIME=0
	@# keep the page alongside wasm outputs
	@cp -f $(NAME)_raw.html $(WOUT)/$(NAME)_raw.html 2>/dev/null || true
	
runwasm: wasm
	#add liek a python serve thingie
	
windows:
	#migw or something

clean:
	-rm -rf build/  $(WOUT) $(LOUT)
