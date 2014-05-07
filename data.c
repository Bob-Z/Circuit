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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "data.h"

map_t * data_parse_map(SDL_Renderer * render,char * map_name)
{
	int fd;
	char * data;
	int index;
	map_t * map;
	ssize_t ret;

	fd = open(map_name,O_RDONLY);
	if(fd == -1) return NULL;

	map = malloc(sizeof(map_t));
	map->num_start = 0;
	map->start_x = NULL;
	map->start_y = NULL;
	map->start_a = NULL;

	index=0;
	data = malloc(index+1);
	while( (ret=read(fd,data+index,1)) == 1) {
		if(*(data+index) == '\n') {
			*(data+index) = 0;
			if(strncmp(data,"picture",strlen("picture")) == 0) {
				map->picture = anim_load(render,data+strlen("picture")+1);
				if(map->picture == NULL) {
					ret = -1;
					break;
				}
			}
			if(strncmp(data,"width",strlen("width")) == 0) {
				map->w = atof(data+strlen("width")+1);
			}
			if(strncmp(data,"height",strlen("height")) == 0) {
				map->h = atof(data+strlen("height")+1);
			}
			if(strncmp(data,"start_x",strlen("start_x")) == 0) {
				map->num_start ++;
				map->start_x = realloc(map->start_x, sizeof(int) * map->num_start);
				map->start_y = realloc(map->start_y, sizeof(int) * map->num_start);
				map->start_a = realloc(map->start_a, sizeof(double) * map->num_start);

				map->start_x[map->num_start-1] = atoi(data+strlen("start_x")+1);
			}
			if(strncmp(data,"start_y",strlen("start_y")) == 0) {
				map->start_y[map->num_start-1] = atoi(data+strlen("start_y")+1);
			}
			if(strncmp(data,"start_a",strlen("start_a")) == 0) {
				map->start_a[map->num_start-1] = atoi(data+strlen("start_a")+1);
			}
			index=-1;
		}
		index++;
		data = realloc(data,index+1);
	}

	free(data);

	if( ret == -1) {
		free(map);
		return NULL;
	}

	return map;
}
map_t * data_load_map(SDL_Renderer * render,char * map_name)
{
	char tmp[1024];
	map_t * map;

	map = data_parse_map(render,map_name);

	if(map) return map;

	strcpy(tmp,getenv("HOME"));
	strcat(tmp,"/.config/circuit/");
	strcat(tmp,map_name);

	map = data_parse_map(render,tmp);

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
	car->ts= 90.0;
	car->accel = 10.0;
	car->decel = 30.0;
	car->max_speed = 84.7; //~305 km/h
	car->speed = 0.0;
	car->engine_brake = 5.0;

	return car;
}
