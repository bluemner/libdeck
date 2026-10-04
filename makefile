OUT:=build
COMPILER:=g++ -std=c++17
C_COMPILER:=gcc -std=gnu17 -O1

DEBUG_FLAGS:=-Wall -DDEBUG -g -fno-omit-frame-pointer -ggdb -fsanitize=address
SOURCE:=source/

SDL_LIB:=`sdl2-config --cflags --libs` -lm -lSDL2_ttf

TEST:=build/test/
DEBUG:=build/debug/
RELEASE:=build/release/

HEADERS_DIR:=include/
INC:=-I $(HEADERS_DIR)

DECK:=$(SOURCE)deck.c
DEMO:=example/demo.c

LIB_NAME:=libdeck.so

COLOR_BLACK	  = \033[30m
COLOR_RED     = \033[31m
COLOR_GREEN   = \033[32m
COLOR_YELLOW  = \033[33m
COLOR_BLUE    = \033[34m
COLOR_MAGENTA = \033[35m
COLOR_CYAN    = \033[36m
COLOR_WHITE   = \033[37m

# --- Terminal Colors ---
COLOR_RESET   = \033[0m
COLOR_HELP    = \033[36m
COLOR_TITLE   = \033[1;35m
COLOR_BUILD   = \033[32m
COLOR_RUN     = \033[33m
COLOR_DIR     = \033[34m


.PHONY: all help dir ttf deck deck-debug deck-debug-run build_c_debug build_test_c_debug build_c_debug_lib build_c_release_lib

all: build_c_debug_lib deck deck-debug

help:
	@echo "$(COLOR_MAGENTA)Available Makefile Commands:$(COLOR_RESET)"
	@echo "  $(COLOR_HELP)make help$(COLOR_RESET)                   - Show this help menu"
	@echo "  $(COLOR_HELP)make all$(COLOR_RESET)                    - Build debug library, release binary, and debug binary"
	@echo "  $(COLOR_HELP)make dir$(COLOR_RESET)                    - Create build, release, debug, and test directories"
	@echo "  $(COLOR_HELP)make tff$(COLOR_RESET)                    - Copy assets from assets/ to build directories"
	@echo "  $(COLOR_HELP)make deck$(COLOR_RESET)                   - Build the release version of the deck binary"
	@echo "  $(COLOR_HELP)make deck-debug$(COLOR_RESET)             - Build the debug version of the deck binary with AddressSanitizer"
	@echo "  $(COLOR_HELP)make deck-debug-run$(COLOR_RESET)         - Build and run the debug version with ASAN suppressions"
	@echo "  $(COLOR_HELP)make build_c_debug_lib$(COLOR_RESET)      - Build the shared library (.so) with debug flags"
	@echo "  $(COLOR_HELP)make build_c_release_lib$(COLOR_RESET)    - Build the shared library (.so) for release"

dir:
	mkdir -p $(DEBUG) $(RELEASE) $(TEST)

ttf: dir
	cp -r assets/* $(DEBUG).
	cp -r assets/* $(RELEASE).
	cp -r assets/* $(TEST).


deck: $(DECK) $(DEMO) | ttf
	$(C_COMPILER) $(INC) $(DEMO) $(DECK) $(SDL_LIB) -o $(RELEASE)deck
	@echo "$(COLOR_GREEN)Build Complete: $(COLOR_CYAN) $(RELEASE)deck $(COLOR_RESET)"

deck-debug: $(DECK) $(DEMO) | ttf
	$(C_COMPILER) $(DEBUG_FLAGS) $(INC) $(DEMO) $(DECK) $(SDL_LIB) -o $(DEBUG)deck
	@echo "$(COLOR_GREEN)Build Complete: $(COLOR_CYAN) $(DEBUG)deck $(COLOR_RESET)"

deck-debug-run: deck-debug
	@echo "$(COLOR_YELLOW)Starting Debug with asan suppressions $(COLOR_RESET)"
	LSAN_OPTIONS=suppressions=./config/asan.supp G_SLICE=always-malloc ./build/debug/deck


build_c_debug_lib: $(DECK) | dir
	$(C_COMPILER) -shared $(DEBUG_FLAGS) $(INC) -fPIC $(BLUEZ_ALL) $(DECK) $(SDL_LIB) -o $(DEBUG)$(LIB_NAME)

build_c_release_lib: $(DECK) | dir
	$(C_COMPILER) -shared $(INC) -fPIC $(BLUEZ_ALL) $(DECK) $(SDL_LIB) -o $(RELEASE)$(LIB_NAME)

