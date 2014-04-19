/*
   World of Gnome is a 2D multiplayer role playing game.
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

#ifndef LOG_H
#define LOG_H

#include <stdio.h>

void init_log(char * log_level);
void log_print(char * file,int line,FILE *stream,int level,char * format, ...);

#define LOGUSER		0
#define LOGDEV		1
#define LOGDEBUG	2

#define wlog(level,str,args...) log_print(__FILE__,__LINE__,stdout,level,str, ## args)
#define werr(level,str,args...) log_print(__FILE__,__LINE__,stderr,level,str, ## args)
#endif
