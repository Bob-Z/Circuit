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
#include "play.h"
#include "log.h"

//const char optstring[] = "?i:u:p:l:";
const char optstring[] = "m:c:l:";
const struct option longopts[] = {
#if 0
	{ "ip",required_argument,NULL,'i' },
	{ "user",required_argument,NULL,'u' },
	{ "pass",required_argument,NULL,'p' },
#endif
	{ "map",required_argument,NULL,'m' },
	{ "car",required_argument,NULL,'c' },
	{ "log",required_argument,NULL,'l' },
	{NULL,0,NULL,0}
};

/**************************
  main
**************************/

int main (int argc, char **argv)
{
	sdl_context_t sdl_context;
	int opt_ret;
	char * map = NULL;
	int num_car = 0;
	char ** car = NULL;
	char * log = NULL;
#if 0
	char * ip = NULL;
	char * user = NULL;
	char * pass = NULL;
#endif

	while((opt_ret = getopt_long(argc, argv, optstring, longopts, NULL))!=-1) {
		switch(opt_ret) {
#if 0
		case 'i':
			ip = strdup(optarg);;
			break;
		case 'u':
			user = strdup(optarg);;
			break;
		case 'p':
			pass = strdup(optarg);;
			break;
#endif
		case 'm':
			map = strdup(optarg);;
			break;
		case 'c':
			num_car++;
			car = realloc(car,sizeof(char*)*(num_car));
			car[num_car-1] = strdup(optarg);;
			break;
		case 'l':
			log = strdup(optarg);;
			break;
		default:
			printf("HELP:\n\n");
#if 0
			printf("-i --ip : Set a server IP\n");
			printf("-u --user: Set a user name\n");
			printf("-p --pass: Set a user password\n");
#endif
			printf("-m --map: map data file (local or in $HOME/.config/circuit/)\n");
			printf("-c --car: car data file (local or in $HOME/.config/circuit/)\n");
			printf("-l --log: Set log level\n");
			exit(0);
		}
	}

	if( map == NULL || car == NULL) {
		werr(LOGUSER,"You must provide a map data file name and at least a car data file name");
		return -1;
	}

	sdl_init(&sdl_context);

	init_log(log);

	play(&sdl_context,map,car,num_car);

	return 0;
}
