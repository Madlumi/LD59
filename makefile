NAME ?= LD59
WOUT ?= LD59
LOUT ?= out
REL  ?= rel

SRC        := src/main.c
SHELL_HTML := src/index.html

CFLAGS_NATIVE := $(shell sdl2-config --cflags)
LIBS_NATIVE   := $(shell sdl2-config --libs) -lSDL2_image -lm

EMFLAGS := \
	--use-port=sdl2 \
	--use-port=sdl2_image:formats=png \
	--shell-file $(SHELL_HTML) \
	--preload-file res \
	-sALLOW_MEMORY_GROWTH=1 \
	-sEXIT_RUNTIME=0

linux:
	@mkdir -p $(LOUT)
	gcc $(SRC) $(CFLAGS_NATIVE) $(LIBS_NATIVE) -o $(LOUT)/$(NAME)

run: linux
	./$(LOUT)/$(NAME)

debug-run:
	@mkdir -p $(LOUT)
	gcc -g -O0 -fsanitize=address,undefined -fno-omit-frame-pointer $(SRC) $(CFLAGS_NATIVE) $(LIBS_NATIVE) -o $(LOUT)/$(NAME)
	./$(LOUT)/$(NAME)

wasm:
	@mkdir -p $(WOUT)
	emcc $(SRC) -o $(WOUT)/index.html $(EMFLAGS)

runwasm: wasm
	cd $(WOUT) && python3 -m http.server 8000

ldjam: wasm
	cd $(WOUT) && zip -r ../$(NAME)-ldjam.zip .

linux-release: linux
	@mkdir -p $(REL)/$(NAME)
	cp $(LOUT)/$(NAME) $(REL)/$(NAME)/
	cp -r res $(REL)/$(NAME)/
	cd $(REL) && zip -r ../$(NAME)-linux.zip $(NAME)

windows:
	# mingw or something

clean:
	-rm -rf build $(WOUT) $(LOUT) $(REL) $(NAME)-ldjam.zip $(NAME)-linux.zip
