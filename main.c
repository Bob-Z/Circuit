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

#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include "sdl.h"
#include "log.h"
#include "data.h"

const char optstring[] = "?i:u:p:l:";
const struct option longopts[] = {
	{ "ip",required_argument,NULL,'i' },
	{ "user",required_argument,NULL,'u' },
	{ "pass",required_argument,NULL,'p' },
	{ "log",required_argument,NULL,'l' },
	{NULL,0,NULL,0}
};

#define NUM_ANIM 2

item_t * item_list = NULL;
context_t context;

char * config_path = NULL;

double angle_sign = 0.0;
Uint32 angle_time = 0;
map_t * map;
car_t * car;

static void screen_display(context_t * ctx)
{
        SDL_Event event;
	Uint32 time;
	double angle;

        while( 1 ) {

                while (SDL_PollEvent(&event)) {
                        if(event.type==SDL_KEYDOWN && event.key.keysym.sym==SDLK_ESCAPE) {
                                return;
                        }

                        sdl_screen_manager(ctx,&event);
                        sdl_mouse_manager(ctx,&event,item_list);
                        sdl_keyboard_manager(&event);
                }

		time = SDL_GetTicks();
		angle = (double)(time-angle_time) * car[0].ts / 1000.0 * angle_sign;
		angle_time = time;
		item_set_angle(item_list->next,item_list->next->angle+=angle);
                SDL_RenderClear(ctx->render);

                sdl_blit_item_list(ctx,item_list);

                sdl_blit_to_screen(ctx);

                sdl_loop_manager();
        }

        return;
}

static void cb_key_left_down(void * arg)
{
	angle_sign = -1.0;
}
static void cb_key_left_up(void * arg)
{
	angle_sign = 0.0;
}
static void cb_key_right_down(void * arg)
{
	angle_sign = 1.0;
}
static void cb_key_right_up(void * arg)
{
	angle_sign = 0.0;
}

/**************************
  main
**************************/

int main (int argc, char **argv)
{
	int opt_ret;
	char * ip = NULL;
	char * user = NULL;
	char * pass = NULL;
	char * log = NULL;

	while((opt_ret = getopt_long(argc, argv, optstring, longopts, NULL))!=-1) {
		switch(opt_ret) {
		case 'i':
			ip = strdup(optarg);;
			break;
		case 'u':
			user = strdup(optarg);;
			break;
		case 'p':
			pass = strdup(optarg);;
			break;
		case 'l':
			log = strdup(optarg);;
			break;
		default:
			printf("HELP:\n\n");
			printf("-i --ip : Set a server IP\n");
			printf("-u --user: Set a user name\n");
			printf("-p --pass: Set a user password\n");
			printf("-l --log: Set log level\n");
			exit(0);
		}
	}

	sdl_init(&context);

	init_log(log);

	// Load graphics
	item_t * item = NULL;
	anim_t * anim[NUM_ANIM];

	map = data_load_map(context.render,NULL);
	anim[0] = map->picture;
	car = data_load_car(context.render,NULL);
	anim[1] = car->picture;

	item_list = item_list_add(NULL);
	item_set_anim(item_list,0,0,anim[0]);
	item = item_list_add(item_list);
	/* Starting line-up coord refer to the center of the vehicle, item coord refer to top/left picture */
	item_set_anim(item,
		map->start_x[0]-(anim[1]->w/2*car->w / map->w),
		map->start_y[0]-(anim[1]->h/2*car->w / map->w),
		anim[1]);
	item_set_zoom_x(item,car->w / map->w);
	item_set_zoom_y(item,car->w / map->w);
	item_set_angle(item, map->start_a[0] - car->a);

	sdl_set_virtual_x(item->rect.x);
	sdl_set_virtual_y(item->rect.y);
	sdl_set_virtual_z(6.0);

	sdl_free_keycb(NULL);
	sdl_add_keycb(SDL_SCANCODE_LEFT,cb_key_left_down,cb_key_left_up);
	sdl_add_keycb(SDL_SCANCODE_RIGHT,cb_key_right_down,cb_key_right_up);
	//Run the main loop
	screen_display(&context);

	return 0;
}
