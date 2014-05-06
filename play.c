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

#include "sdl.h"
#include "log.h"
#include "data.h"

#define NUM_ANIM 2

item_t * item_list = NULL;

double angle_sign = 0.0;
int accel = 0;
int decel = 0;
Uint32 angle_time = 0;
map_t * map;
car_t * car;

#define UNIT_TO_PIX(a) (a  / map->w *(double) map->picture->w )
#define PIX_TO_UNIT(a) (a  * map->w /(double) map->picture->w )

static void calculate_new_pos(item_t * item, car_t * car)
{
	car->x += cos(car->a / 180.0 * M_PI) * car->speed;
	car->y += sin(car->a / 180.0 * M_PI) * car->speed;
printf("car->x = %f\n",car->x);
printf("car->y = %f\n",car->y);
printf("speed = %f\n",car->speed);
printf("angle = %f\n",car->a);
printf("cos(car->a) = %f\n",cos(car->a / 180.0 * M_PI));
printf("sin(car->a) = %f\n",sin(car->a / 180.0 * M_PI));

	item_set_pos(item,UNIT_TO_PIX(car->x),UNIT_TO_PIX(car->y));

	item_set_angle(item,car->a);

	sdl_force_virtual_x(UNIT_TO_PIX(car->x));
	sdl_force_virtual_y(UNIT_TO_PIX(car->y));
}
static void screen_display(sdl_context_t * ctx)
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
		car[0].a += (double)(time-angle_time) * car[0].ts / 1000.0 * angle_sign;
		if(accel) {
			car[0].speed += (double)(time-angle_time) * car[0].accel / 1000.0;
			if(car[0].speed > car[0].max_speed) {
				car[0].speed = car[0].max_speed;
			}
		}
		if(decel) {
			car[0].speed -= (double)(time-angle_time) * car[0].accel / 1000.0;
			if(car[0].speed < -car[0].max_speed) {
				car[0].speed = -car[0].max_speed;
			}
		}
		angle_time = time;

		calculate_new_pos(item_list->next,&car[0]);

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
static void cb_key_up_down(void * arg)
{
	accel = 1;
}
static void cb_key_up_up(void * arg)
{
	accel = 0;
}
static void cb_key_down_down(void * arg)
{
	decel = 1;
}
static void cb_key_down_up(void * arg)
{
	decel = 0;
}

void play(sdl_context_t * context)
{
	// Load graphics
	item_t * item = NULL;
	anim_t * anim[NUM_ANIM];

	map = data_load_map(context->render,NULL);
	anim[0] = map->picture;
	car = data_load_car(context->render,NULL);
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
	car->x = PIX_TO_UNIT(map->start_x[0]);
	car->y = PIX_TO_UNIT(map->start_y[0]);
	car->a = map->start_a[0] - car->a;
	item_set_angle(item, car->a);

	sdl_set_virtual_x(item->rect.x);
	sdl_set_virtual_y(item->rect.y);
	sdl_set_virtual_z(6.0);

	sdl_free_keycb(NULL);
	sdl_add_keycb(SDL_SCANCODE_LEFT,cb_key_left_down,cb_key_left_up);
	sdl_add_keycb(SDL_SCANCODE_RIGHT,cb_key_right_down,cb_key_right_up);
	sdl_add_keycb(SDL_SCANCODE_UP,cb_key_up_down,cb_key_up_up);
	sdl_add_keycb(SDL_SCANCODE_DOWN,cb_key_down_down,cb_key_down_up);
	//Run the main loop
	screen_display(context);

}