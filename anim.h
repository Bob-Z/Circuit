/*
   Circuit is a racing game
   Copyright (C) 2014 carabobz@gmail.com

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

#ifndef ANIM_H
#define ANIM_H

#include <SDL2/SDL.h>

typedef struct {
	int num_frame;
	SDL_Texture ** tex;
	int current_frame;
	int w; // width
	int h; // height
	Uint32 * delay; //delay between each frame in millisecond
	Uint32 prev_time; //time to show the current_frame
} anim_t;

anim_t * anim_load(SDL_Renderer * render,const char * filename);
void anim_reset_anim(anim_t * anim);
#endif
