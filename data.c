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

option_t *  init_option()
{
	option_t * o = malloc(sizeof(option_t));

	o->zoom = 1;

	return o;
}

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
	map->bounce = 1.0;

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
				map->h = (int) ((double)map->picture->h / (double)map->picture->w * map->w);
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
			if(strncmp(data,"bounce",strlen("bounce")) == 0) {
				map->bounce = atof(data+strlen("bounce")+1);
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

car_t * data_parse_car(SDL_Renderer * render,char * car_name)
{
	int fd;
	char * data;
	int index;
	car_t * car;
	ssize_t ret;

	fd = open(car_name,O_RDONLY);
	if(fd == -1) return NULL;

	car = malloc(sizeof(car_t));
	car->speed = 0.0;
	car->a = 0.0;

	index=0;
	data = malloc(index+1);
	while( (ret=read(fd,data+index,1)) == 1) {
		if(*(data+index) == '\n') {
			*(data+index) = 0;
			if(strncmp(data,"picture",strlen("picture")) == 0) {
				car->picture = anim_load(render,data+strlen("picture")+1);
				if(car->picture == NULL) {
					ret = -1;
					break;
				}
			}
			if(strncmp(data,"width",strlen("width")) == 0) {
				car->w = atof(data+strlen("width")+1);
			}
			if(strncmp(data,"height",strlen("height")) == 0) {
				car->h = atof(data+strlen("height")+1);
			}
			if(strncmp(data,"angle",strlen("angle")) == 0) {
				car->angle = atof(data+strlen("angle")+1);
			}
			if(strncmp(data,"turn_speed",strlen("turn_speed")) == 0) {
				car->ts = atof(data+strlen("turn_speed")+1);
			}
			if(strncmp(data,"accel",strlen("accel")) == 0) {
				car->accel = atof(data+strlen("accel")+1);
			}
			if(strncmp(data,"decel",strlen("decel")) == 0) {
				car->decel = atof(data+strlen("decel")+1);
			}
			if(strncmp(data,"max_speed",strlen("max_speed")) == 0) {
				car->max_speed = atof(data+strlen("max_speed")+1);
			}
			if(strncmp(data,"engine_brake",strlen("engine_brake")) == 0) {
				car->engine_brake = atof(data+strlen("engine_brake")+1);
			}

			car->angle_sign = 0.0;
			car->key_u = 0;
			car->key_d = 0;
			car->key_l = 0;
			car->key_r = 0;
			car->forward = 0;
			car->backward = 0;


			index=-1;
		}
		index++;
		data = realloc(data,index+1);
	}

	free(data);

	if( ret == -1) {
		free(car);
		return NULL;
	}

	return car;
}
car_t * data_load_car(SDL_Renderer * render,char * car_name)
{
	char tmp[1024];
	car_t * car;

	car = data_parse_car(render,car_name);

	if(car) return car;

	strcpy(tmp,getenv("HOME"));
	strcat(tmp,"/.config/circuit/");
	strcat(tmp,car_name);

	car = data_parse_car(render,tmp);

	return car;
}
