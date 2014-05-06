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

#ifndef LOG_C
#define LOG_C

#include "log.h"
#include <string.h>
#include <time.h>
#include <stdarg.h>

static int log_level = 0;

char *log_level_string[3] = {"user","dev","debug"};

void log_print(char * file,int line,FILE *stream,int level,char * format, ...)
{
	va_list ap;
	char buf[10000];
	struct timespec now;

	if(level > log_level) {
		return;
	}

	va_start(ap,format);
	vsnprintf(buf,sizeof(buf),format,ap);
	va_end(ap);

	if(log_level == LOGDEBUG) {
		clock_gettime(CLOCK_MONOTONIC, &now);
		fprintf(stream,"%ld.%09ld | %10s:%3d | %s\n",now.tv_sec,now.tv_nsec,file,line,buf);
	} else if(log_level == LOGDEV) {
		clock_gettime(CLOCK_MONOTONIC, &now);
		fprintf(stream,"%ld.%09ld | %s\n",now.tv_sec,now.tv_nsec,buf);
	} else if(log_level == LOGUSER) {
		fprintf(stream,"%s\n",buf);
	}
}

void init_log(char * log)
{
	if(log == NULL) {
		return;
	}

	if(!strcmp(log,"0") || !strcmp(log,"user")) {
		log_level = LOGUSER;
		return;
	}
	if(!strcmp(log,"1") || !strcmp(log,"dev")) {
		log_level = LOGDEV;
		return;
	}
	if(!strcmp(log,"2") || !strcmp(log,"debug")) {
		log_level = LOGDEBUG;
		return;
	}
}
#endif
