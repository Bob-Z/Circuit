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
map_t * map;
car_t * car;

#define UNIT_TO_PIX(a) ((a)  / map->w *(double) map->picture->w )
#define PIX_TO_UNIT(a) ((a)  * map->w /(double) map->picture->w )

static void calculate_new_pos(item_t * item, car_t * car)
{
	Uint32 time;
	double t;
	static Uint32 old_time = 0;

	time = SDL_GetTicks();
	t = (double)(time-old_time) / 1000.0;
	car->a += t * car->ts * angle_sign;
	if(accel) {
		car->speed += t * car->accel ;
		if(car->speed > car->max_speed) {
			car->speed = car->max_speed;
		}
	}
	if(decel) {
		car->speed -= t * car->decel ;
		if(car->speed < -car->max_speed) {
			car->speed = -car->max_speed;
		}
	}

printf("speed = %f\n",car->speed);

	car->x += cos(car->a / 180.0 * M_PI) * car->speed * t;
	car->y += sin(car->a / 180.0 * M_PI) * car->speed * t;

	car->futur_x = car->x + cos(car->a / 180.0 * M_PI) * car->speed * FUTUR_TIME;
	car->futur_y = car->y + sin(car->a / 180.0 * M_PI) * car->speed * FUTUR_TIME;

	old_time = time;

	item_set_pos(item,UNIT_TO_PIX(car->x),UNIT_TO_PIX(car->y));
	item_set_angle(item,car->a);
}

static void set_display(car_t * car)
{
	double dx;
	double dy;
	double min_x;
	double max_x;
	double min_y;
	double max_y;
	double zoom;

	min_x = max_x = car->x;
	min_y = max_y = car->y;

	if( car->futur_x < min_x )
		min_x = car->futur_x;
	if( car->futur_x > max_x )
		max_x = car->futur_x;
	if( car->futur_y < min_y )
		min_y = car->futur_y;
	if( car->futur_y > max_y )
		max_y = car->futur_y;

	dx = max_x - min_x;
	dy = max_y - min_y;

#if 0
	sdl_force_virtual_x(UNIT_TO_PIX( min_x + dx/2.0));
	sdl_force_virtual_y(UNIT_TO_PIX( min_y + dy/2.0));
#endif
	sdl_force_virtual_x(UNIT_TO_PIX(car->x));
	sdl_force_virtual_y(UNIT_TO_PIX(car->y));

	max_x = sqrt(dx * dx + dy * dy);
	max_x += sqrt( car->w * car->w + car->h * car->h);

printf("zoom = %f / %f = %f\n",map->w , max_x, map->w / max_x);
	sdl_force_virtual_z(map->w / (max_x * 4.0)  );
}

static void screen_display(sdl_context_t * ctx)
{
        SDL_Event event;

        while( 1 ) {

                while (SDL_PollEvent(&event)) {
                        if(event.type==SDL_KEYDOWN && event.key.keysym.sym==SDLK_ESCAPE) {
                                return;
                        }

                        sdl_screen_manager(ctx,&event);
                        sdl_mouse_manager(ctx,&event,item_list);
                        sdl_keyboard_manager(&event);
                }

		calculate_new_pos(item_list->next,&car[0]);

		set_display(&car[0]);

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
