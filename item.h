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

#ifndef ITEM_H
#define ITEM_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "anim.h"

typedef struct item {
	SDL_Rect rect;
	int x;	// For smooth animation
	int y;	// For smooth animation
	double angle;
	double zoom_x;
	double zoom_y;
	int old_x;	// For smooth animation
	int old_y;	// For smooth animation
	Uint32 timer;	// For smooth animation
	int tile_x;	// Coordinate in tile
	int tile_y;	// Coordinate in tile
	int overlay;
	anim_t * anim;
	int anim_start;
	int anim_end;
	int current_frame;
	int frame_normal; // if -1 it's an animation
	int frame_over;
	int frame_click;
	int clicked;
	void (*click_left)(void * arg); //callback on left click on this item
	void * click_left_arg;
	void (*click_right)(void * arg); //callback on right click on this item
	void * click_right_arg;
	void (*over)(void * arg); //callback on mouse over this item
	void * over_arg;
	const char * string;		// string centered on item
	TTF_Font * font;
	SDL_Texture * str_tex;
	struct item * next;	// next element in a list of item
} item_t;

item_t * item_list_add(item_t * item_list);
void item_list_free(item_t * item_list);
void item_init(item_t * item);
void item_set_frame(item_t * item, int x, int y,anim_t * anim);
void item_set_anim(item_t * item, int x, int y,anim_t * anim);
void item_set_smooth_anim(item_t * item, int x, int y,int old_x, int old_y, Uint32 timer, anim_t * anim);
void item_set_tile(item_t * item, int x, int y);
void item_set_angle(item_t * item, double a);
void item_set_zoom_x(item_t * item, double a);
void item_set_zoom_y(item_t * item, double a);
void item_set_overlay(item_t * item, int overlay);
void item_set_frame_normal(item_t * item, int num_frame);
void item_set_frame_over(item_t * item, int num_frame);
void item_set_frame_click(item_t * item, int num_frame);
void item_set_click_left(item_t * item,void (*click_left)(void * arg),void * click_left_arg);
void item_set_click_right(item_t * item,void (*click_right)(void * arg),void * click_right_arg);
void item_set_over(item_t * item,void (*over)(void * arg),void * over_arg);
void item_set_string(item_t * item,const char * string);
void item_set_geometry(item_t * item,int x, int y, int w, int h);
void item_set_anim_start(item_t * item, int start_frame);
void item_set_anim_end(item_t * item, int end_frame);
void item_set_font(item_t * item, TTF_Font * font);

#endif
