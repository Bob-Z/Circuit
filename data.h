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

typedef struct map {
	anim_t * picture;
	/* Physical width e.g. 4 km.
	This is independant of the picture pixel size (pixel may not be square
	You can use any unit, but the chosen unit must be the same for the other elements of the game */
	double w;
	double h;
} map_t;

typedef struct car {
	anim_t * picture;
	/* Physical width e.g. 4 km.
	This is independant of the picture pixel size (pixel may not be square
	You can use any unit, but the chosen unit must be the same for the other elements of the game */
	double w;
	double h;
} car_t;

map_t * data_load_map(SDL_Renderer * render,char * map_name);
car_t * data_load_car(SDL_Renderer * render,char * car_name);

#endif