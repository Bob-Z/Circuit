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

#include "data.h"

map_t * data_load_map(SDL_Renderer * render,char * map_name)
{
	char tmp[1024];
	map_t * map;

	map = malloc(sizeof(map_t));

	strcpy(tmp,getenv("HOME"));
	strcat(tmp,"/.config/circuit/");
	strcat(tmp,"hungaroring_circuit02.jpg");
	map->picture = anim_load(render,tmp);

//	map->w = 4000.0;
//	map->h = 3947.0;
	map->w = 300.0;
	map->h = 300.0;

#define NUM_START 1
	map->num_start = NUM_START;
	map->start_x = malloc(sizeof(int) * NUM_START);
	map->start_y = malloc(sizeof(int) * NUM_START);
	map->start_a = malloc(sizeof(double) * NUM_START);

	map->start_x[0] = 1613;
	map->start_y[0] = 3348;
	map->start_a[0] = 218.0;

	return map;
}

car_t * data_load_car(SDL_Renderer * render,char * car_name)
{
	char tmp[1024];
	car_t * car;

	car = malloc(sizeof(car_t));

	strcpy(tmp,getenv("HOME"));
	strcat(tmp,"/.config/circuit/");
	strcat(tmp,"3541.gif");
	car->picture = anim_load(render,tmp);

	car->w = 4.39;
	car->h = 2.315;
	car->a = 0.0; // Head to the right

	return car;
}
