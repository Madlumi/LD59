
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

debug-run: 
	gcc -g -O0 -fsanitize=address,undefined -fno-omit-frame-pointer src/main.c `sdl2-config --cflags` `sdl2-config --libs` -lSDL2_image -lm -o out/LD59
	./out/LD59
wasm:
	@mkdir -p build $(WOUT)
	emcc -c src/main.c -o build/$(NAME).o \
		--use-port=sdl2 \
		--use-port=sdl2_image:formats=png
	emcc build/$(NAME).o -o $(WOUT)/$(NAME).html \
		--use-port=sdl2 \
		--use-port=sdl2_image:formats=png \
		--preload-file res \
		-sALLOW_MEMORY_GROWTH=1 \
		-sEXIT_RUNTIME=0
	
runwasm: wasm
	cd $(WOUT) && python3 -m http.server 8000
	
windows:
	#migw or something

clean:
	-rm -rf build/  $(WOUT) $(LOUT)
