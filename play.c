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

#define GLOBAL_KEY_NUM 2
//player_key_t global_key[GLOBAL_KEY_NUM] = {{SDL_SCANCODE_UP,SDL_SCANCODE_DOWN,SDL_SCANCODE_LEFT,SDL_SCANCODE_RIGHT},{SDL_SCANCODE_W,SDL_SCANCODE_S,SDL_SCANCODE_Q,SDL_SCANCODE_D}};
player_key_t global_key[GLOBAL_KEY_NUM] = {{SDL_SCANCODE_W,SDL_SCANCODE_S,SDL_SCANCODE_A,SDL_SCANCODE_D},{SDL_SCANCODE_UP,SDL_SCANCODE_DOWN,SDL_SCANCODE_LEFT,SDL_SCANCODE_RIGHT}};

item_t * item_list = NULL;
SDL_Joystick* joystick[MAX_JOY];
int num_joy = 0;

map_t * map;
car_t ** all_car;
int car_num;
option_t * option;

static void cb_key_left_down(void * arg);
static void cb_key_right_down(void * arg);
static void cb_key_up_down(void * arg);
static void cb_key_down_down(void * arg);

/* Source: http://devmag.org.za/2009/04/13/basic-collision-detection-in-2d-part-1/ */
static int collision(double A1x, double A1y, double A2x, double A2y, double B1x, double B1y, double B2x, double B2y)
{
	//wlog(LOGDEBUG," test A1(%f,%f) A2(%f,%f), B1(%f,%f) B2(%f,%f)",A1x,A1y,A2x,A2y,B1x,B1y,B2x,B2y);
	double denom = ((B2y - B1y) * (A2x - A1x)) -
		((B2x - B1x) * (A2y - A1y));

	/* Lines are parallel */
	if ( denom == 0.0 ) {
		return 0;
	}

	double uA =  (((B2x - B1x)*(A1y - B1y))-((B2y - B1y)*(A1x - B1x)))/denom;
	double uB =  (((A2x - A1x)*(A1y - B1y))-((A2y - A1y)*(A1x - B1x)))/denom;
	wlog(LOGDEBUG,"uA = %f",uA);
	wlog(LOGDEBUG,"uB = %f",uB);
	if( uA < 1.0 && uA > 0.0 && uB < 1.0 && uB > 0.0 ) {
		wlog(LOGDEBUG," test A1(%f,%f) A2(%f,%f), B1(%f,%f) B2(%f,%f)",A1x,A1y,A2x,A2y,B1x,B1y,B2x,B2y);
		return 1;
	}
	return 0;
}

static int box_collision(double *Ax,double *Ay, double *Bx, double *By)
{
	int i;
	int j;
	int c;

	for(i=0;i<4;i++) {
		for(j=0;j<4;j++) {
			c = collision(Ax[i],Ay[i],Ax[(i+1)%4],Ay[(i+1)%4],Bx[j],By[j],Bx[(j+1)%4],By[(j+1)%4]);
			if(c) {
				wlog(LOGDEBUG,"Collision");
				return 1;
			}
		}
	}

	return 0;
}

static int wall_collision(car_t * car)
{
	/* Map points*/
	double Ax[4];
	double Ay[4];

	/* Car points*/
	double Bx[4];
	double By[4];

	Ax[0] = 0.0;
	Ay[0] = 0.0;
	Ax[1] = map->w;
	Ay[1] = 0.0;
	Ax[2] = map->w;
	Ay[2] = map->h;
	Ax[3] = 0.0;
	Ay[3] = map->h;

	Bx[0] = (car->x - car->w / 2.0) * cos(car->a + car->angle);
	By[0] = (car->y - car->h / 2.0) * sin(car->a + car->angle);
	Bx[1] = (car->x + car->w / 2.0) * cos(car->a + car->angle);
	By[1] = (car->y - car->h / 2.0) * sin(car->a + car->angle);
	Bx[2] = (car->x + car->w / 2.0) * cos(car->a + car->angle);
	By[2] = (car->y + car->h / 2.0) * sin(car->a + car->angle);
	Bx[3] = (car->x - car->w / 2.0) * cos(car->a + car->angle);
	By[3] = (car->y + car->h / 2.0) * sin(car->a + car->angle);

	return box_collision(Ax,Ay,Bx,By);
}

static int car_collision(car_t * car1, car_t * car2)
{
	double angle;
	double cos_angle;
	double sin_angle;

	/* Car1 points*/
	double Ax[4];
	double Ay[4];

	/* Car2 points*/
	double Bx[4];
	double By[4];
	car_t * car;

	car=car1;
	angle = (car->a + car->angle)/180.0*M_PI;
	cos_angle = cos(angle);
	sin_angle = sin(angle);
	Ax[0] = (car->x) - car->w / 2.0 * cos_angle;
	Ay[0] = (car->y) - car->h / 2.0 * sin_angle;
	Ax[1] = (car->x) + car->w / 2.0 * cos_angle;
	Ay[1] = (car->y) - car->h / 2.0 * sin_angle;
	Ax[2] = (car->x) + car->w / 2.0 * cos_angle;
	Ay[2] = (car->y) + car->h / 2.0 * sin_angle;
	Ax[3] = (car->x) - car->w / 2.0 * cos_angle;
	Ay[3] = (car->y) + car->h / 2.0 * sin_angle;

	car=car2;
	angle = (car->a + car->angle)/180.0*M_PI;
	cos_angle = cos(angle);
	sin_angle = sin(angle);
	Bx[0] = (car->x) - car->w / 2.0 * cos_angle;
	By[0] = (car->y) - car->h / 2.0 * sin_angle;
	Bx[1] = (car->x) + car->w / 2.0 * cos_angle;
	By[1] = (car->y) - car->h / 2.0 * sin_angle;
	Bx[2] = (car->x) + car->w / 2.0 * cos_angle;
	By[2] = (car->y) + car->h / 2.0 * sin_angle;
	Bx[3] = (car->x) - car->w / 2.0 * cos_angle;
	By[3] = (car->y) + car->h / 2.0 * sin_angle;

	return box_collision(Ax,Ay,Bx,By);
}

static void calculate_new_pos(car_t * car, map_t * map)
{
	Uint32 time;
	double t;
	int joy_forward = 0;
	int joy_backward = 0;
	int joy_x = 0;

	time = SDL_GetTicks();
	t = (double)(time-car->old_anim_time) / 1000.0;

	if(car->joy) {
		joy_forward |= SDL_JoystickGetButton(car->joy,0);
		joy_backward |= SDL_JoystickGetButton(car->joy,1);
		joy_x = SDL_JoystickGetAxis(car->joy, 0);
	}

	if( car->angle_sign) {
		car->a += t * car->ts * car->angle_sign;
	}
	else {
		// Limit jitter
		if(joy_x > 1000 || joy_x < -1000) {
			car->a += t * car->ts * (double)joy_x / 32767.0;
		}
	}

	if(car->forward || joy_forward) {
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
	else if(car->backward || joy_backward) {
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

//	wall_collision(car);

	int i;
	for(i=0;i<car_num;i++) {
		if( car != all_car[i] ) {
			car_collision(car,all_car[i]);
		}
	}

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

	car->old_anim_time = time;

	item_set_pos(car->item,UNIT_TO_PIX(car->x - car->w/2.0),UNIT_TO_PIX(car->y - car->h/2.0));
	item_set_angle(car->item,car->a);
}

static void set_display(sdl_context_t * ctx)
{
	double dx;
	double dy;
	double min_x;
	double max_x;
	double min_y;
	double max_y;
	double zoom_x;
	double zoom_y;
	double zoom;
	int sx;
	int sy;
	int i;
	double max_dimension = 0.0;

	SDL_GetRendererOutputSize(ctx->render,&sx,&sy);


	min_x = max_x = all_car[0]->x;
	min_y = max_y = all_car[0]->y;

	if(option->zoom) {
		for(i=0;i<car_num;i++) {
			max_dimension = all_car[i]->w;
			if( max_dimension < all_car[i]->h ) {
				max_dimension = all_car[i]->h;
			}
			if( all_car[i]->x - max_dimension < min_x )
				min_x = all_car[i]->x - max_dimension;
			if( all_car[i]->x + max_dimension > max_x )
				max_x = all_car[i]->x + max_dimension;
			if( all_car[i]->y - max_dimension < min_y )
				min_y = all_car[i]->y - max_dimension;
			if( all_car[i]->y + max_dimension > max_y )
				max_y = all_car[i]->y + max_dimension;

			if( all_car[i]->futur_x < min_x )
				min_x = all_car[i]->futur_x ;
			if( all_car[i]->futur_x > max_x )
				max_x = all_car[i]->futur_x ;
			if( all_car[i]->futur_y < min_y )
				min_y = all_car[i]->futur_y ;
			if( all_car[i]->futur_y > max_y )
				max_y = all_car[i]->futur_y ;
		}

		dx = max_x - min_x;
		dy = max_y - min_y;

		sdl_set_virtual_x(UNIT_TO_PIX( min_x + dx/2.0));
		sdl_set_virtual_y(UNIT_TO_PIX( min_y + dy/2.0));
//		sdl_force_virtual_x(UNIT_TO_PIX( min_x + dx/2.0));
//		sdl_force_virtual_y(UNIT_TO_PIX( min_y + dy/2.0));

		// Zoom calculation
		// sx/2 = Constant size in pixel to display on the screen: the distance on the screen between the car and it's futur postion
		// have to be constant.
		//double distance = sqrt(dx*dx+dy*dy);
		//if( distance < 2.0 * car->w ) {
		//	distance = 2.0 * car->w;
		//}
		//zoom = (double)(sx) / (double)( UNIT_TO_PIX(distance)) ;
		zoom_x = 0.0;
		zoom_y = 0.0;
		if(dx) {
			zoom_x = (double)(sx) / (double)( UNIT_TO_PIX(dx)) ;
		}
		if(dy) {
			zoom_y = (double)(sy) / (double)( UNIT_TO_PIX(dy)) ;
		}

		zoom = zoom_x;
		if(zoom_x > zoom_y ) {
			zoom = zoom_y;
		}
		//wlog(LOGDEBUG,"zoom_x = %f, zoom_y = %f, zoom = %f",zoom_x,zoom_y,zoom);
		sdl_set_virtual_z(zoom );
		//sdl_force_virtual_z(zoom );
	}
	else {
		sdl_force_virtual_x(UNIT_TO_PIX( all_car[0]->x));
		sdl_force_virtual_y(UNIT_TO_PIX( all_car[0]->y));

		zoom = (double)(sx/2) / (double)( UNIT_TO_PIX(7.0*all_car[0]->w)) ;
		sdl_force_virtual_z(zoom );
	}
}

static void screen_display(sdl_context_t * ctx)
{
        SDL_Event event;
	int i;

        while( 1 ) {

                while (SDL_PollEvent(&event)) {
                        if(event.type==SDL_KEYDOWN && event.key.keysym.sym==SDLK_ESCAPE) {
                                return;
                        }

                        sdl_screen_manager(ctx,&event);
                        sdl_mouse_manager(ctx,&event,item_list);
                        sdl_keyboard_manager(&event);
                }

		for(i=0;i<car_num;i++) {
			calculate_new_pos(all_car[i],map);
		}

		set_display(ctx);

                SDL_RenderClear(ctx->render);

                sdl_blit_item_list(ctx,item_list);

                sdl_blit_to_screen(ctx);

                sdl_loop_manager();
        }

        return;
}

static void cb_key_left_down(void * arg)
{
	car_t * car = (car_t*)arg;

	car->angle_sign = -1.0;
	car->key_l = 1;
}
static void cb_key_left_up(void * arg)
{
	car_t * car = (car_t*)arg;

	car->angle_sign = 0.0;
	car->key_l = 0;
	if(car->key_r) {
		cb_key_right_down(arg);
	}
}
static void cb_key_right_down(void * arg)
{
	car_t * car = (car_t*)arg;

	car->angle_sign = 1.0;
	car->key_r = 1;
}
static void cb_key_right_up(void * arg)
{
	car_t * car = (car_t*)arg;

	car->angle_sign = 0.0;
	car->key_r = 0;
	if(car->key_l) {
		cb_key_left_down(arg);
	}
}
static void cb_key_up_down(void * arg)
{
	car_t * car = (car_t*)arg;

	car->forward = 1;
	car->backward = 0;
	car->key_u = 1;
}
static void cb_key_up_up(void * arg)
{
	car_t * car = (car_t*)arg;

	car->forward = 0;
	car->key_u = 0;
	if(car->key_d) {
		cb_key_down_down(arg);
	}
}
static void cb_key_down_down(void * arg)
{
	car_t * car = (car_t*)arg;

	car->backward = 1;
	car->forward = 0;
	car->key_d = 1;
}
static void cb_key_down_up(void * arg)
{
	car_t * car = (car_t*)arg;

	car->backward = 0;
	car->key_d = 0;
	if(car->key_u) {
		cb_key_up_down(arg);
	}

}

void play(sdl_context_t * context, char * map_name, char ** car_name, int num,option_t * o)
{
	SDL_Joystick* joy;
	int i;

	option = o;
	car_num = num;

	if(car_num > 1) {
		option->zoom = 1;
	}

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

	item_list = item_list_add(NULL);
	item_set_anim(item_list,0,0,anim[0]);

	all_car = malloc(car_num*sizeof(car_t*));

	for(i=0;i<car_num;i++) {
		all_car[i] = data_load_car(context->render,car_name[i]);
		if(all_car[i] == NULL) {
			werr(LOGUSER,"Cannot read car %s",car_name[i]);
			return;
		}

		anim[i+1] = all_car[i]->picture;

		item = item_list_add(item_list);
		/* Starting line-up coord refer to the center of the vehicle, item coord refer to top/left picture */
		item_set_anim(item,
				map->start_x[i]-(anim[i+1]->w/2*all_car[i]->w / map->w),
				map->start_y[i]-(anim[i+1]->h/2*all_car[i]->w / map->w),
				anim[i+1]);
		item_set_zoom_x(item,all_car[i]->w / map->w * (double)map->picture->w / (double)all_car[i]->picture->w);
		item_set_zoom_y(item,all_car[i]->w / map->w * (double)map->picture->w / (double)all_car[i]->picture->w);
		all_car[i]->x = PIX_TO_UNIT(map->start_x[i]);
		all_car[i]->y = PIX_TO_UNIT(map->start_y[i]);
		all_car[i]->a = map->start_a[i] - all_car[i]->angle;
		item_set_angle(item, all_car[i]->a);
		all_car[i]->item = item;

		sdl_set_virtual_x(item->rect.x);
		sdl_set_virtual_y(item->rect.y);
		sdl_set_virtual_z(6.0);

		//Input
		all_car[i]->joy = joystick[i];
		//sdl_free_keycb(NULL);
		sdl_add_keycb(global_key[i].up,cb_key_up_down,cb_key_up_up,all_car[i]);
		sdl_add_keycb(global_key[i].down,cb_key_down_down,cb_key_down_up,all_car[i]);
		sdl_add_keycb(global_key[i].left,cb_key_left_down,cb_key_left_up,all_car[i]);
		sdl_add_keycb(global_key[i].right,cb_key_right_down,cb_key_right_up,all_car[i]);

		all_car[i]->old_anim_time=0;
	}
	
	//Run the main loop
	screen_display(context);
}
