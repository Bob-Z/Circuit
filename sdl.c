/*
   Circuit is a racing game
   Copyright (C) 2013 carabobz@gmail.com

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

static int fullscreen = 0;

static char keyboard_buf[2048];
static unsigned int keyboard_index = 0;
static void (*keyboard_cb)(void * arg) = NULL;

static int virtual_x = 0;
static int virtual_y = 0;
static int old_vx = 0;
static int old_vy = 0;
static int current_vx = 0;
static int current_vy = 0;
static Uint32 virtual_tick = 0;

static keycb_t * key_callback = NULL;

//You must SDL_LockSurface(surface); then SDL_UnlockSurface(surface); before calling this function
void sdl_set_pixel(SDL_Surface *surface, int x, int y, Uint32 R, Uint32 G, Uint32 B, Uint32 A)
{
	Uint32 color = (A << 24) + (R << 16) + (G << 8) + (B);
	Uint8 *target_pixel = (Uint8 *)surface->pixels + y * surface->pitch + x * sizeof(color);
	*(Uint32 *)target_pixel = color;
}

void sdl_cleanup()
{
	SDL_Quit();
}

void sdl_init(context_t * context)
{
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
		werr(LOGUSER,"SDL init failed: %s.\n",SDL_GetError());
		exit(EXIT_FAILURE);
	}

	if (TTF_Init() == -1){
		werr(LOGUSER,"TTF init failed: %s.\n",SDL_GetError());
		exit(EXIT_FAILURE);
	}

	atexit(sdl_cleanup);

	context->window = SDL_CreateWindow("Circuit",
								 SDL_WINDOWPOS_UNDEFINED,
								 SDL_WINDOWPOS_UNDEFINED,
								 DEFAULT_SCREEN_W, DEFAULT_SCREEN_H,
								 SDL_WINDOW_RESIZABLE);
	if( context->window == NULL) {
		werr(LOGUSER,"SDL window init failed: %s.\n",SDL_GetError());
		exit(EXIT_FAILURE);
	}

	context->render = SDL_CreateRenderer(context->window, -1, SDL_RENDERER_PRESENTVSYNC);
	if( context->render == NULL) {
		werr(LOGUSER,"SDL renderer init failed: %s\n",SDL_GetError());
		exit(EXIT_FAILURE);
	}

	SDL_RenderSetLogicalSize(context->render,DEFAULT_SCREEN_W,DEFAULT_SCREEN_H);
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
}

static void get_virtual(context_t * ctx,int * vx, int * vy)
{
	int sx;
	int sy;

	SDL_GetRendererOutputSize(ctx->render,&sx,&sy);
	*vx = (sx/2)-current_vx;
	*vy = (sy/2)-current_vy;
}

void sdl_mouse_manager(context_t * ctx, SDL_Event * event, item_t * item_list)
{
	SDL_Rect rect;
	int vx;
	int vy;
	item_t * I;
	int overlay_first = 1;
	int skip_non_overlay = 0;

	if(item_list == NULL) {
		return;
	}

#if 0
	printf("Mouse moved by %d,%d to (%d,%d)\n",
			event->motion.xrel, event->motion.yrel,
			event->motion.x, event->motion.y);
	printf("orig coord = %d,%d \n",rect.x,rect.y);
#endif

	/* First test overlay (UI) before background */
	while(overlay_first!=-1) {
		I = item_list;
		while(I) {
			if(I->overlay) {
				if(!overlay_first) {
					I = I->next;
					continue;
				}
				rect.x = event->motion.x;
				rect.y = event->motion.y;
			}
			else {
				if(overlay_first) {
					I = I->next;
					continue;
				}
				get_virtual(ctx,&vx,&vy);
				rect.x = event->motion.x - vx;
				rect.y = event->motion.y - vy;
			}

			I->current_frame = I->frame_normal;
			if( (I->rect.x < rect.x) &&
					((I->rect.x+I->rect.w) > rect.x) &&
					(I->rect.y < rect.y) &&
					((I->rect.y+I->rect.h) > rect.y) ) {
				/* We are on overlay item: skip, non-overlay item */
				if(overlay_first) {
					skip_non_overlay=1;
				}
				switch (event->type) {
					case SDL_MOUSEMOTION:
						I->current_frame = I->frame_over;
						if( I->over ) {
							I->over(I->over_arg);
						}
						break;
					case SDL_MOUSEBUTTONDOWN:
						I->current_frame = I->frame_click;
						if( I->click_left && event->button.button == SDL_BUTTON_LEFT) {
							I->current_frame=I->frame_click;
							I->clicked=1;
						}
						if( I->click_right && event->button.button == SDL_BUTTON_RIGHT) {
							I->current_frame=I->frame_click;
							I->clicked=1;
						}
						break;
					case SDL_MOUSEBUTTONUP:
						I->clicked=0;
						I->current_frame = I->frame_normal;
						if( I->click_left && event->button.button == SDL_BUTTON_LEFT) {
							I->click_left(I->click_left_arg);
						}
						if( I->click_right && event->button.button == SDL_BUTTON_RIGHT) {
							I->click_right(I->click_right_arg);
						}
						break;
				}
			}
			if(I->clicked) {
				I->current_frame = I->frame_click;
			}
			I = I->next;
		}
		overlay_first--;
		if(skip_non_overlay) break;
	}
}

/* Take care of system's windowing event */
void sdl_screen_manager(context_t * ctx,SDL_Event * event)
{
	const Uint8 *keystate;

	switch (event->type) {
        case SDL_WINDOWEVENT:
		switch(event->window.event) {
		case SDL_WINDOWEVENT_RESIZED:
			SDL_RenderSetLogicalSize(ctx->render,event->window.data1,event->window.data2);
			break;
		}
	break;
	case SDL_KEYDOWN:
		switch (event->key.keysym.sym) {
		case SDLK_RETURN:
			keystate = SDL_GetKeyboardState(NULL);

			if( keystate[SDL_SCANCODE_RALT] || keystate[SDL_SCANCODE_LALT] ) {
				if(!fullscreen) {
					fullscreen = SDL_WINDOW_FULLSCREEN_DESKTOP;
				} else {
					fullscreen = 0;
				}
				SDL_SetWindowFullscreen(ctx->window,fullscreen);
				break;
			}
			break;
		default:
			break;
		}
		break;
	case SDL_QUIT:
		exit(EXIT_SUCCESS);
		break;
	default:
		break;
	}
}

void sdl_loop_manager()
{
	static Uint32 old_timer = 0;
	Uint32 timer;

	if( old_timer == 0 ) {
		old_timer = SDL_GetTicks();
	}

	timer = SDL_GetTicks();

	if( timer < old_timer + FRAME_DELAY ) {
		SDL_Delay(old_timer + FRAME_DELAY - timer);
	}

	timer = SDL_GetTicks();
	if( virtual_tick + VIRTUAL_ANIM_DURATION > timer ) {
		current_vx = (int)((float)old_vx + (float)( virtual_x - old_vx ) * (float)(timer - virtual_tick) / (float)VIRTUAL_ANIM_DURATION);
		current_vy = (int)((float)old_vy + (float)( virtual_y - old_vy ) * (float)(timer - virtual_tick) / (float)VIRTUAL_ANIM_DURATION);
	}
	else {
		old_vx = virtual_x;
		current_vx = virtual_x;

		old_vy = virtual_y;
		current_vy = virtual_y;
	}
}

void sdl_blit_tex(context_t * ctx,SDL_Texture * tex, SDL_Rect * rect,double angle, int overlay)
{
	SDL_Rect r;
        int vx;
        int vy;
	static double zoom = 1.0;

	SDL_Rect DestR;

	zoom-=0.0001;

	if(overlay) {
		r.x = rect->x;
		r.y = rect->y;
	}
	else {
		get_virtual(ctx,&vx,&vy);

		r.x = rect->x + vx;
		r.y = rect->y + vy;
	}

	r.w = rect->w;
	r.h = rect->h;

	r.x *= zoom;
	r.y *= zoom;
	r.w *= zoom;
	r.h *= zoom;

	if( tex ) {
//		if( SDL_RenderCopy(ctx->render,tex,NULL,&r) < 0) {
		if( SDL_RenderCopyEx(ctx->render,tex,NULL,&r,angle,NULL,SDL_FLIP_NONE) < 0) {
			werr(LOGDEV,"SDL_RenderCopy error\n");
		}
	}
}

int sdl_blit_anim(context_t * ctx,anim_t * anim, SDL_Rect * rect, double angle, int start, int end,int overlay)
{
	Uint32 time = SDL_GetTicks();

	sdl_blit_tex(ctx,anim->tex[anim->current_frame],rect,angle,overlay);

	if( anim->prev_time == 0 ) {
		anim->prev_time = time;
	}
	if( time >= anim->prev_time + anim->delay[anim->current_frame]) {
		(anim->current_frame)++;
		anim->prev_time = time;
		if( end != -1 ) {
			if(anim->current_frame >= end) {
				anim->current_frame = start;
				return 1;
			}
		} else {
			if(anim->current_frame >= anim->num_frame) {
				anim->current_frame = 0;
				return 1;
			}
		}
	}

	return 0;
}

void sdl_print_item(context_t * ctx,item_t * item)
{
	SDL_Surface * surf;
//	SDL_Color bg={0,0,0};
	SDL_Color fg={0xff,0xff,0xff};
	SDL_Rect r;

	/* Get center of item */
        r.x = item->rect.x + (item->rect.w/2);
        r.y = item->rect.y + (item->rect.h/2);

	/* Get top/left of text */
	TTF_SizeText(item->font, item->string, &r.w, &r.h);
	r.x = r.x-(r.w/2);
	r.y = r.y-(r.h/2);

	if(item->str_tex == NULL ) {
		surf = TTF_RenderText_Blended(item->font, item->string, fg);
		item->str_tex=SDL_CreateTextureFromSurface(ctx->render,surf);
		SDL_FreeSurface(surf);
	}

	sdl_blit_tex(ctx,item->str_tex,&r,item->angle,item->overlay);
}

int sdl_blit_item(context_t * ctx,item_t * item)
{
	Uint32 timer = SDL_GetTicks();

	if(item->anim) {
		if( item->timer ) {
			if( item->timer + VIRTUAL_ANIM_DURATION > timer) {
				item->rect.x = (int)((float)item->old_x + (float)(item->x - item->old_x) * (float)(timer - item->timer) / (float)VIRTUAL_ANIM_DURATION);
				item->rect.y = (int)((float)item->old_y + (float)(item->y - item->old_y) * (float)(timer - item->timer) / (float)VIRTUAL_ANIM_DURATION);
			}
			else {
				item->rect.x =item->x;
				item->rect.y =item->y;
//printf("+++ anim reset : %d x %d\n", item->rect.x,item->rect.y);
			}
		}

		if( item->frame_normal == -1 ) {
			sdl_blit_anim(ctx,item->anim,&item->rect,item->angle,item->anim_start,item->anim_end,item->overlay);
		} else {
			sdl_blit_tex(ctx,item->anim->tex[item->frame_normal],&item->rect,item->angle,item->overlay);
		}
	}

	if( item->font != NULL && item->string != NULL ) {
                sdl_print_item(ctx,item);
        }

	return 0;
}

void sdl_blit_item_list(context_t * ctx,item_t * list)
{
	item_t * item;

	item = list;
	while(item)  {
		sdl_blit_item(ctx,item);
		item = item->next;
	}
}

void sdl_keyboard_init(char * string, void (*cb)(void*arg))
{
	keyboard_index=0;
	if( string ) {
		strcpy(keyboard_buf,string);
		keyboard_index=strlen(keyboard_buf);
	}
	keyboard_buf[keyboard_index]=0;
	keyboard_cb=cb;
}

char * sdl_keyboard_get_buf()
{
	if(keyboard_cb) {
		return keyboard_buf;
	} else {
		return NULL;
	}
}

void sdl_keyboard_manager(SDL_Event * event)
{
	const Uint8 *keystate;
	keycb_t * key;

	switch (event->type) {
	case SDL_KEYDOWN:
		key = key_callback;
		if(key) {
			do {
				if( event->key.keysym.scancode == key->code) {
					key->cb(NULL);
				}
				key=key->next;
			} while(key);
		}
		if( event->key.keysym.sym == SDLK_RETURN ) {
			if( keyboard_cb ) {
				keyboard_cb(NULL);
				keyboard_cb=NULL;
			}
		}

		if( event->key.keysym.sym == SDLK_DELETE ||
				event->key.keysym.sym == SDLK_BACKSPACE) {
			if(keyboard_index > 0 ) {
				keyboard_index--;
			}
			keyboard_buf[keyboard_index]=0;
		}

		if( event->key.keysym.sym >= SDLK_SPACE &&
				event->key.keysym.sym < SDLK_DELETE ) {

			/* Uppercase */
			keystate = SDL_GetKeyboardState(NULL);
			if( (keystate[SDL_SCANCODE_RSHIFT] ||
					keystate[SDL_SCANCODE_LSHIFT] ) &&
					(event->key.keysym.sym >=SDL_SCANCODE_A &&
					 event->key.keysym.sym <=SDL_SCANCODE_Z) ) {
				event->key.keysym.sym = (SDL_Scancode)(event->key.keysym.sym-32);
			}
			keyboard_buf[keyboard_index]=event->key.keysym.sym;
			if( keyboard_index < sizeof(keyboard_buf)) {
				keyboard_index++;
			}
			keyboard_buf[keyboard_index]=0;
		}
		break;
	default:
		break;
	}
}

void sdl_blit_to_screen(context_t * ctx)
{
	SDL_RenderPresent(ctx->render);
}

void sdl_set_virtual_x(int x)
{
	if( x != virtual_x ) {
		old_vx = current_vx;
		virtual_x = x;
		virtual_tick = SDL_GetTicks();
	}
}
void sdl_set_virtual_y(int y)
{
	if( y != virtual_y ) {
		old_vy = current_vy;
		virtual_y = y;
		virtual_tick = SDL_GetTicks();
	}
}

void sdl_force_virtual_x(int x)
{
	virtual_x = x;
	current_vx = x;
	old_vx =x;
}
void sdl_force_virtual_y(int y)
{
	virtual_y = y;
	current_vy = y;
	old_vx =y;
}

keycb_t * sdl_add_keycb(SDL_Scancode code,void (*cb)(void*))
{
	keycb_t * key;

	if(key_callback==NULL) {
		key = malloc(sizeof(keycb_t));
		key_callback = key;
		key->code = code;
		key->cb = cb;
		key->next = NULL;
		return key;
	}
	else {
		key = key_callback;
		while(key->next != NULL) {
			key = key->next;
		}
		key->next = malloc(sizeof(keycb_t));
		key = key->next;
		key->code = code;
		key->cb = cb;
		key->next = NULL;
		return key;
	}
}

static void rec_free_keycb(keycb_t * key) {
	if(key == NULL) {
                return;
        }

	if (key->next) {
		rec_free_keycb(key->next);
	}

	free(key);
}
void sdl_free_keycb(keycb_t ** key)
{
	rec_free_keycb(key_callback);

	key_callback = NULL;
}
