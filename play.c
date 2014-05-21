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
#define MAX_JOY 64

item_t * item_list = NULL;
SDL_Joystick* joystick[MAX_JOY];
int num_joy = 0;

double angle_sign = 0.0;
int forward = 0;
int backward = 0;
map_t * map;
car_t * car;
option_t * option;

int key_u = 0;
int key_d = 0;
int key_l = 0;
int key_r = 0;
static void cb_key_left_down(void * arg);
static void cb_key_right_down(void * arg);
static void cb_key_up_down(void * arg);
static void cb_key_down_down(void * arg);

static void calculate_new_pos(item_t * item, car_t * car, map_t * map)
{
	Uint32 time;
	double t;
	static Uint32 old_time = 0;
	int joy_forward = 0;
	int joy_backward = 0;
	int joy_x = 0;

	time = SDL_GetTicks();
	t = (double)(time-old_time) / 1000.0;

	if(joystick[0]) {
		joy_forward |= SDL_JoystickGetButton(joystick[0],0);
		joy_backward |= SDL_JoystickGetButton(joystick[0],1);
		joy_x = SDL_JoystickGetAxis(joystick[0], 0);
	}

	if( angle_sign) {
		car->a += t * car->ts * angle_sign;
	}
	else {
		// Limit jitter
		if(joy_x > 1000 || joy_x < -1000) {
			car->a += t * car->ts * (double)joy_x / 32767.0;
		}
	}

	if(forward || joy_forward) {
		/* Acceleration forward */
		if(car->speed > 0.0) {
			car->speed += t * car->accel ;
			if(car->speed > car->max_speed) {
				car->speed = car->max_speed;
			}
		}
		/* Brake when going backward */
		else {
			car->speed += t * car->decel ;
		}
	}
	else if(backward || joy_backward) {
		/* Acceleration backward */
		if(car->speed < 0.0) {
			car->speed -= t * car->accel ;
			if(car->speed < -car->max_speed) {
				car->speed = -car->max_speed;
			}
		}
		/* Brake when going forward */
		else {
			car->speed -= t * car->decel ;
		}
	}
	else {
		if(car->speed > 0) {
			car->speed -= t * car->engine_brake;
			if(car->speed < 0) car->speed=0;
		}
		else {
			car->speed += t * car->engine_brake;
			if(car->speed > 0) car->speed=0;
		}
	}

	//wlog(LOGDEBUG,"speed = %f",car->speed);

	car->x += cos((car->a + car->angle) / 180.0 * M_PI) * car->speed * t;
	car->y += sin((car->a + car->angle) / 180.0 * M_PI) * car->speed * t;

	int bounce = 0;
	if(car->x < 0.0){
		 car->x = 0.0;
		bounce = 1;
	}
	if(car->y < 0.0) {
		car->y = 0.0;
		bounce = 1;
	}
	if(car->x > map->w ) {
		car->x = map->w;
		bounce = 1;
	}
	if(car->y > map->h) {
		car->y = map->h;
		bounce = 1;
	}

	if(bounce) {
		car->speed *= map->bounce;
	}

	car->futur_x = car->x + cos((car->a + car->angle)  / 180.0 * M_PI) * car->speed * FUTUR_TIME;
	car->futur_y = car->y + sin((car->a  + car->angle) / 180.0 * M_PI) * car->speed * FUTUR_TIME;

	old_time = time;

	item_set_pos(item,UNIT_TO_PIX(car->x - car->w/2.0),UNIT_TO_PIX(car->y - car->h/2.0));
	item_set_angle(item,car->a);
}

static void set_display(sdl_context_t * ctx, car_t * car)
{
	double dx;
	double dy;
	double min_x;
	double max_x;
	double min_y;
	double max_y;
	double zoom;
	int sx;
	int sy;

	SDL_GetRendererOutputSize(ctx->render,&sx,&sy);

	if(option->zoom) {
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

		sdl_set_virtual_x(UNIT_TO_PIX( min_x + dx/2.0));
		sdl_set_virtual_y(UNIT_TO_PIX( min_y + dy/2.0));

		// Zoom calculation
		// sx/2 = Constant size in pixel to display on the screen: the distance on the screen between the car and it's futur postion
		// have to be constant.
		double distance = sqrt(dx*dx+dy*dy);
		if( distance < 2.0 * car->w ) {
			distance = 2.0 * car->w;
		}
		zoom = (double)(sx/2) / (double)( UNIT_TO_PIX(distance)) ;
		//wlog(LOGDEBUG,"zoom = %f",zoom);
		sdl_set_virtual_z(zoom );
	}
	else {
		sdl_force_virtual_x(UNIT_TO_PIX( car->x));
		sdl_force_virtual_y(UNIT_TO_PIX( car->y));

		zoom = (double)(sx/2) / (double)( UNIT_TO_PIX(7.0*car->w)) ;
		sdl_force_virtual_z(zoom );
	}
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

		calculate_new_pos(item_list->next,&car[0],map);

		set_display(ctx,&car[0]);

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
	key_l = 1;
}
static void cb_key_left_up(void * arg)
{
	angle_sign = 0.0;
	key_l = 0;
	if(key_r) {
		cb_key_right_down(arg);
	}
}
static void cb_key_right_down(void * arg)
{
	angle_sign = 1.0;
	key_r = 1;
}
static void cb_key_right_up(void * arg)
{
	angle_sign = 0.0;
	key_r = 0;
	if(key_l) {
		cb_key_left_down(arg);
	}
}
static void cb_key_up_down(void * arg)
{
	forward = 1;
	backward = 0;
	key_u = 1;
}
static void cb_key_up_up(void * arg)
{
	forward = 0;
	key_u = 0;
	if(key_d) {
		cb_key_down_down(arg);
	}
}
static void cb_key_down_down(void * arg)
{
	backward = 1;
	forward = 0;
	key_d = 1;
}
static void cb_key_down_up(void * arg)
{
	backward = 0;
	key_d = 0;
	if(key_u) {
		cb_key_up_down(arg);
	}

}

void play(sdl_context_t * context, char * map_name, char ** car_name, int car_num,option_t * o)
{
	SDL_Joystick* joy;

	option = o;

	// Init Joysticks
	while( (joy=SDL_JoystickOpen(num_joy)) != NULL ) {
		joystick[num_joy] = joy;
		wlog(LOGDEBUG,"Opened Joystick %d",num_joy);
		wlog(LOGDEBUG,"Name: %s", SDL_JoystickNameForIndex(num_joy));
		wlog(LOGDEBUG,"Number of Axes: %d", SDL_JoystickNumAxes(joystick[num_joy]));
		wlog(LOGDEBUG,"Number of Buttons: %d", SDL_JoystickNumButtons(joystick[num_joy]));
		wlog(LOGDEBUG,"Number of Balls: %d", SDL_JoystickNumBalls(joystick[num_joy]));
		num_joy++;
	}

	// Init items
	item_t * item = NULL;
	anim_t * anim[NUM_ANIM];

	map = data_load_map(context->render,map_name);
	if(map == NULL) {
		werr(LOGUSER,"Cannot read map %s",map_name);
		return;
	}
	anim[0] = map->picture;
	car = data_load_car(context->render,car_name[0]);
	if(car == NULL) {
		werr(LOGUSER,"Cannot read car %s",car_name[0]);
		return;
	}
	anim[1] = car->picture;

	item_list = item_list_add(NULL);
	item_set_anim(item_list,0,0,anim[0]);
	item = item_list_add(item_list);
	/* Starting line-up coord refer to the center of the vehicle, item coord refer to top/left picture */
	item_set_anim(item,
			map->start_x[0]-(anim[1]->w/2*car->w / map->w),
			map->start_y[0]-(anim[1]->h/2*car->w / map->w),
			anim[1]);
	item_set_zoom_x(item,car->w / map->w * (double)map->picture->w / (double)car->picture->w);
	item_set_zoom_y(item,car->w / map->w * (double)map->picture->w / (double)car->picture->w);
	car->x = PIX_TO_UNIT(map->start_x[0]);
	car->y = PIX_TO_UNIT(map->start_y[0]);
	car->a = map->start_a[0] - car->angle;
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
