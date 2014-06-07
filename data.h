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

#ifndef DATA_H
#define DATA_H

#include "anim.h"
#include "item.h"

#define UNIT_TO_PIX(a) ((a)  / map->w *(double) map->picture->w )
#define PIX_TO_UNIT(a) ((a)  * map->w /(double) map->picture->w )

// Number of seconds to forsee the position
#define FUTUR_TIME (1.5)

typedef struct player_key {
	SDL_Scancode up;
	SDL_Scancode down;
	SDL_Scancode left;
	SDL_Scancode right;
} player_key_t;


typedef struct map {
	anim_t * picture;
	/* Physical width e.g. 4 km.
	This is independant of the picture pixel size
	You can use any unit, but the chosen unit must be the same for the other elements of the game */
	double w; // Used to compute sprite zoom
	double h; // unused
	int num_start; // Number of starting plot
	int * start_x; // Start coordinate in pixel
	int * start_y;
	double * start_a;  // Car angle on start-up line
	double bounce; //Car speed multiplier in case of collision
} map_t;

typedef struct car {
	item_t * item;
	anim_t * picture;
	/* Physical width e.g. 4 km.
	This is independant of the picture pixel size (pixel may not be square
	You can use any unit, but the chosen unit must be the same for the other elements of the game */
	double w; // Used to compute sprite zoom
	double h; //unused
	double x;
	double y;
	double futur_x; // Coordinate if nothing change in FUTUR_TIME second
	double futur_y; // Coordinate if nothing change in FUTUR_TIME second
	double angle; // orientation of the car in the picture
	double ts; // turning speed in degrees/second
	double accel; // in map_unit / second^2
	double decel; // in map_unit / second^2
	double max_speed; // in map_unit / second
	double speed;
	double a; // orientation of the car during the game
	double engine_brake; // in map_unit / second^2
	SDL_Joystick* joy;
	player_key_t * player_key;
	//Variable used for dynamic behaviour
	double angle_sign;
	int key_u;
	int key_d;
	int key_l;
	int key_r;
	int forward;
	int backward;
	Uint32 old_anim_time;
} car_t;

typedef struct game_option {
	int zoom;
} option_t;

option_t * init_option();
map_t * data_load_map(SDL_Renderer * render,char * map_name);
car_t * data_load_car(SDL_Renderer * render,char * car_name);

#endif
