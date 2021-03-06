/*
   Circuit is a racing game
   Copyright (C) 2013 carabobz@gmail.com

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
*/

#ifndef SDL_H
#define SDL_H

#include <SDL2/SDL.h>
#include "item.h"

#define SDL_OPAQUE 0xff
#define SDL_TRANSPARENT 0x00

#define RMASK 0x00ff0000
#define GMASK 0x0000ff00
#define BMASK 0x000000ff
#define AMASK 0xff000000

#define DEFAULT_SCREEN_W 640
#define DEFAULT_SCREEN_H 480

#define FRAME_DELAY 10

#define VIRTUAL_ANIM_DURATION 200

//#define PAL_TO_RGB(x) x.r<<2,x.g<<2,x.b<<2,SDL_OPAQUE
#define PAL_TO_RGB(x) x.r,x.g,x.b,SDL_OPAQUE

typedef struct sdl_context {
        SDL_Renderer * render;
        SDL_Window * window;
} sdl_context_t;


typedef struct keycb {
        SDL_Scancode code;
        void (*cb)(void*);
        void (*cb_up)(void*);
		void * arg;
        struct keycb * next;
} keycb_t;

void sdl_init(sdl_context_t * context);
void sdl_cleanup(void);
void sdl_set_pixel(SDL_Surface *surface, int x, int y, Uint32 R, Uint32 G, Uint32 B, Uint32 A);
void sdl_mouse_manager(sdl_context_t * ctx,SDL_Event * event, item_t * item_list);
void sdl_screen_manager(sdl_context_t * ctx,SDL_Event * event);
void sdl_loop_manager();
void sdl_blit_tex(sdl_context_t * ctx,SDL_Texture * tex, SDL_Rect * rect,double angle, double zoom_x,double zoom_y, int overlay);
int sdl_blit_anim(sdl_context_t * ctx,anim_t * anim, SDL_Rect * rect, double angle, double zoom_x, double zoom_y, int start, int end,int overlay);
void sdl_print_item(sdl_context_t * ctx,item_t * item);
int sdl_blit_item(sdl_context_t * ctx,item_t * item);
void sdl_blit_item_list(sdl_context_t * ctx,item_t * item_list);
void sdl_keyboard_init(char * string, void (*cb)(void*arg));
char * sdl_keyboard_get_buf();
void sdl_keyboard_manager(SDL_Event * event);
void sdl_blit_to_screen(sdl_context_t * ctx);
void sdl_set_virtual_x(int x);
void sdl_set_virtual_y(int y);
void sdl_set_virtual_z(double z);
void sdl_force_virtual_x(int x);
void sdl_force_virtual_y(int y);
void sdl_force_virtual_z(double z);
keycb_t * sdl_add_keycb(SDL_Scancode code,void (*cb)(void*),void (*cb_up)(void*),void * arg);
void sdl_free_keycb(keycb_t ** key);

#endif
