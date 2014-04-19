/*
   Circuit is a racing game.
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
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>

#include <gif_lib.h>

#define GIF_GCE 0xf9

#define DISPOSE_DO_NOT 1 /* Draw on top of previous image */
#define DISPOSE_BACKGROUND 2 /* Clean with the background color */
#define DISPOSE_PREVIOUS 3 /* Restore to previous content */

#define DEFAULT_DELAY 40

static anim_t * giflib_load(SDL_Renderer * render,const char * filename)
{
	GifFileType * gif;
	int i;
	int j;
	int transparent;
	unsigned char transparent_color;
	int disposal;
	int delay;
	ColorMapObject * global_pal;
	ColorMapObject * pal;
	SDL_Surface* surf;
	int x;
	int y;
	int col;
	int pix_index;
	anim_t * anim;
	unsigned char bg_color;
	int left;
	int top;
	int width;
	int height;

	gif = DGifOpenFileName(filename);
	if(gif == NULL) {
		return NULL;
	}
	wlog(LOGDEBUG,"Using giflib to decode %s",filename);
	DGifSlurp(gif);

	bg_color = gif->SBackGroundColor;

	wlog(LOGDEBUG,"%d frames %d x %d bg_color=%d",gif->ImageCount, gif->SWidth, gif->SHeight,bg_color);

	anim = malloc(sizeof(anim_t));
	memset(anim,0,sizeof(anim_t));

	anim->num_frame = gif->ImageCount;
	anim->tex = malloc(sizeof(SDL_Texture *) * anim->num_frame);
	anim->w = gif->SWidth;
	anim->h = gif->SHeight;
	anim->delay = malloc(sizeof(Uint32) * anim->num_frame);

	surf = SDL_CreateRGBSurface(0,gif->SWidth,gif->SHeight,32,0xff000000,0x00ff0000,0x0000ff00,0x000000ff);
	memset(surf->pixels,0,gif->SWidth*gif->SHeight*sizeof(Uint32));

	global_pal = gif->SColorMap;
	for(i=0;i<gif->ImageCount;i++) {
		left = gif->SavedImages[i].ImageDesc.Left;
		top = gif->SavedImages[i].ImageDesc.Top;
		width = gif->SavedImages[i].ImageDesc.Width;
		height = gif->SavedImages[i].ImageDesc.Height;

		wlog(LOGDEBUG,"Image %d : %d x %d; %d x %d",i,left,top,width,height);
		/* select palette */
		pal = global_pal;
		if( gif->SavedImages[i].ImageDesc.ColorMap ) {
			pal = gif->SavedImages[i].ImageDesc.ColorMap;
		}
		/* GCE */
		for(j=0;j<gif->SavedImages[i].ExtensionBlockCount;j++) {
			if(gif->SavedImages[i].ExtensionBlocks[j].Function == GIF_GCE) {
				transparent = gif->SavedImages[i].ExtensionBlocks[j].Bytes[0] & 0x01;
				wlog(LOGDEBUG,"transparent : %d",transparent);
				disposal = (gif->SavedImages[i].ExtensionBlocks[j].Bytes[0] & 28)>>2;
				wlog(LOGDEBUG,"disposal : %d",disposal);
				delay = (gif->SavedImages[i].ExtensionBlocks[j].Bytes[1] + gif->SavedImages[i].ExtensionBlocks[j].Bytes[2] * 256)*10;
				if(delay==0) {
					delay = DEFAULT_DELAY;
				}
				wlog(LOGDEBUG,"delay : %d ms",delay);
				if(transparent) {
					transparent_color = gif->SavedImages[i].ExtensionBlocks[j].Bytes[3];
					wlog(LOGDEBUG,"transparent color : %d",transparent_color);
				}
			}
		}

		/* Prepare surface depending of disposal */
		if(disposal==DISPOSE_BACKGROUND) {
			memset(surf->pixels,0,gif->SWidth*gif->SHeight*sizeof(Uint32));
		}
		if(disposal==DISPOSE_PREVIOUS) { /* This is probably wrong */
			memset(surf->pixels,0,gif->SWidth*gif->SHeight*sizeof(Uint32));
		}

		/* Fill surface buffer with raster bytes */
		for(y=0;y<height;y++) {
			for(x=0;x<width;x++) {
				pix_index = ((x+left)+(y+top)*gif->SWidth)*4;
				col = gif->SavedImages[i].RasterBits[(x)+(y)*gif->SavedImages[i].ImageDesc.Width];
				if(col == transparent_color && transparent) {
				}
				else {
					((char*)surf->pixels)[pix_index+3] = pal->Colors[col].Red;
					((char*)surf->pixels)[pix_index+2] = pal->Colors[col].Green;
					((char*)surf->pixels)[pix_index+1] = pal->Colors[col].Blue;
					((char*)surf->pixels)[pix_index+0] = 0xFF;
				}
			}
		}

		anim->delay[i] = delay;
		anim->tex[i] = SDL_CreateTextureFromSurface(render,surf);
	}
	SDL_FreeSurface(surf);

	DGifCloseFile(gif);

	return anim;
}

static anim_t * libav_load(SDL_Renderer * render,const char * filename)
{
	anim_t * anim = NULL;
	anim_t * ret = NULL;
	AVFormatContext *pFormatCtx = NULL;
	int i;
	int videoStream;
	AVCodecContext *pCodecCtx = NULL;
	AVCodec *pCodec = NULL;
	AVFrame *pFrame = NULL;
	AVFrame *pFrameRGB = NULL;
	struct SwsContext * pSwsCtx = NULL;
	AVPacket packet;
	int frameFinished;
	int numBytes;
	uint8_t *buffer = NULL;
	int delay;

	anim = malloc(sizeof(anim_t));
	memset(anim,0,sizeof(anim_t));

        // Register all formats and codecs
        av_register_all();

        // Open video file
        if (avformat_open_input(&pFormatCtx, filename, NULL, NULL) != 0) {
		werr(LOGDEV,"Cannot open file %s",filename);
                goto error;
	}

        // Retrieve stream information
        if (avformat_find_stream_info(pFormatCtx, NULL) < 0) {
		werr(LOGDEV,"Cannot find stream information for file %s",filename);
                goto error;
	}

	wlog(LOGDEBUG,"Loading %d streams in %s",pFormatCtx->nb_streams,filename);
        // Find the first video stream
        videoStream = -1;
        for (i = 0; i < pFormatCtx->nb_streams; i++)
                if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
                        videoStream = i;
                        /* total stream duration (in nanosecond) / number of image in the stream / 1000 (to get milliseconds */
                        delay = pFormatCtx->duration / pFormatCtx->streams[i]->duration / 1000;
                        /* If the above doesn't work try with frame_rate : 
                        pFormatCtx->streams[i]->r_frame_rate
                        */
			anim->num_frame = pFormatCtx->streams[i]->duration;
			anim->tex = (SDL_Texture**)malloc(anim->num_frame * sizeof(SDL_Texture*));
			anim->delay = (Uint32*)malloc(anim->num_frame * sizeof(Uint32));
                        break;
                }

        if (videoStream == -1) {
		werr(LOGDEV,"Didn't find a video stream in %s",filename);
                goto error;
	}

        // Get a pointer to the codec context for the video stream
        pCodecCtx = pFormatCtx->streams[videoStream]->codec;

        // Find the decoder for the video stream
        pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
        if (pCodec == NULL) {
		werr(LOGDEV,"Unsupported codec for %s",filename);
                goto error;
	}

        // Open codec
        if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
		werr(LOGDEV,"Could not open codec for %s",filename);
                goto error;
	}

        // Allocate video frame
        pFrame = avcodec_alloc_frame();
        if (pFrame == NULL) {
		werr(LOGDEV,"Could not allocate video frame for %s",filename);
                goto error;
	}
        // Allocate an AVFrame structure
        pFrameRGB = avcodec_alloc_frame();
        if (pFrameRGB == NULL) {
		werr(LOGDEV,"Could not allocate AVFrame structure for %s",filename);
                goto error;
	}

        // Determine required buffer size and allocate buffer
        numBytes = avpicture_get_size(PIX_FMT_RGBA, pCodecCtx->width,
                        pCodecCtx->height);
        buffer = (uint8_t *) av_malloc(numBytes * sizeof(uint8_t));

        // Assign appropriate parts of buffer to image planes in pFrameRGB
        // Note that pFrameRGB is an AVFrame, but AVFrame is a superset
        // of AVPicture
        avpicture_fill((AVPicture *) pFrameRGB, buffer, PIX_FMT_RGBA,
                        pCodecCtx->width, pCodecCtx->height);

        pSwsCtx = sws_getContext(pCodecCtx->width,
                        pCodecCtx->height, pCodecCtx->pix_fmt,
                        pCodecCtx->width, pCodecCtx->height,
                        PIX_FMT_RGBA, SWS_BILINEAR, NULL, NULL, NULL);

	anim->w = pCodecCtx->width;
	anim->h = pCodecCtx->height;

        if (pSwsCtx == NULL) {
		werr(LOGDEV,"Cannot initialize sws context for %s",filename);
                goto error;
	}

        // Read frames
        i = 0;
        while (av_read_frame(pFormatCtx, &packet) >= 0) {
                // Is this a packet from the video stream?
                if (packet.stream_index == videoStream) {
                        // Decode video frame
                        avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished, &packet);
                        // Did we get a video frame?
                        if (frameFinished) {
                                // Convert the image from its native format to ABGR
                                sws_scale(pSwsCtx,
                                        (const uint8_t * const *) pFrame->data,
                                        pFrame->linesize, 0, pCodecCtx->height,
                                        pFrameRGB->data,
                                        pFrameRGB->linesize);

				anim->delay[i] = delay;
				anim->tex[i] = SDL_CreateTexture(render, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STATIC, pCodecCtx->width,pCodecCtx->height);
				if( anim->tex[i] == NULL ) {
					werr(LOGDEV,"SDL_CreateTexture error: %s",SDL_GetError());
				}
                                /* Copy decoded bits to render texture */
                                if (SDL_UpdateTexture(anim->tex[i],NULL,pFrameRGB->data[0],pFrameRGB->linesize[0]) < 0) {
					werr(LOGDEV,"SDL_UpdateTexture error: %s",SDL_GetError());
				}
				i++;
                        }
                }

                // Free the packet that was allocated by av_read_frame
                av_free_packet(&packet);
        }

	ret = anim;

error:
	if(ret == NULL) {
		if(anim) {
			if(anim->tex) {
				free(anim->tex);
			}
			free(anim);
		}
	}

        // Free the RGB image
	if(buffer) {
		av_free(buffer);
	}

	if(pFrameRGB) {
		av_free(pFrameRGB);
	}

        // Free the YUV frame
	if(pFrame) {
		av_free(pFrame);
	}

        // Close the codec
	if(pCodecCtx) {
		avcodec_close(pCodecCtx);
	}

        // Close the video file
	if(pFormatCtx) {
		avformat_close_input(&pFormatCtx);
	}

	return ret;
}

void anim_reset_anim(anim_t * anim)
{
	anim->prev_time=0;
	anim->current_frame=0;
}

anim_t * anim_load(SDL_Renderer * render,const char * filename)
{
	anim_t * ret;

	ret = giflib_load(render,filename);

	if(ret == NULL) {
		ret = libav_load(render,filename);
	}

	return ret;
}
