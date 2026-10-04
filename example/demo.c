/**
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026 Brandon Bluemner <dev@brandonbluemner.com>
 * 
 * sudo apt install libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev
 * gcc -std=gnu17 -O1  -Wall -g -fno-omit-frame-pointer -ggdb -fsanitize=address -I include/ source/deck.c source/deck.c `sdl2-config --cflags --libs` -lm -o build/debug/deck &&./build/debug/deck
 */

#include <libgen.h>
#include "deck.h"



int main(int argc, char *argv[])
{
    char *dir = dirname(argv[0]);
    printf("%s\n", dir);
    deck_input_t input = {
        .ready = false,
        .bluetooth = false,
        .usb = false,
        .wifi = false,
        .running = false,
        .settings = false,
        .config = nullptr,
        .events = nullptr,
        .hid = nullptr,
        .dir = dir
    };
    render(&input);
    return 0;
}