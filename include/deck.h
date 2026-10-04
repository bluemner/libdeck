/**
 * SPDX-License-Identifier: LGPL-2.1-only
 * Copyright (c) 2026 Brandon Bluemner <dev@brandonbluemner.com>
 * 
 * Packages needed to compile
 * 
 *     libsdl2-dev         # core lib
 *     libsdl2-ttf-dev     # TrueType Fonts
 * 
 *   optional:
 *     libsdl2-image-dev  # Image Loading 
 *     libsdl2-mixer-dev  # Audio/Music Mixer 
 *     libsdl2-net-dev    # Networking
 */
#ifndef __LIB_DECK_H__
#define __LIB_DECK_H__

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include <stdbool.h>
#include <stdlib.h>
#include <math.h>

#include <stdio.h>
#include <unistd.h>

#ifndef nullptr
    #define nullptr ((void *)0)
#endif

#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

#ifndef __TYPE_VOID_PTR__
    #define __TYPE_VOID_PTR__
    typedef void *ptr;
#endif


#define SDL_SCANCODE_BLUETOOTH 291
#define SDL_SCANCODE_WIFI 292
#define SDL_SCANCODE_USB 293

#define SDLK_BLUETOOTH SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_BLUETOOTH)
#define SDLK_WIFI SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_WIFI)
#define SDLK_USB SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_USB)


#ifdef DEBUG
#  define  debug_printf printf
#else
#  define debug_printf(fmt, ...) ((void)0)
#endif


typedef void (*event_handler_fn)(SDL_Event *event, ptr input);

typedef struct deck_input
{
    bool ready;
    bool bluetooth;
    bool usb;
    bool wifi;
    bool running;
    bool settings;
    char *dir;
    event_handler_fn events;
    ptr hid;
    ptr config;  

} deck_input_t;

void quit();
void render(deck_input_t *input);


#endif