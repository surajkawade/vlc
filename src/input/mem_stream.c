/*****************************************************************************
 * mem_stream.c
 *****************************************************************************
 * Copyright (C) 1999-2004 VideoLAN
 * $Id: stream.c 9390 2004-11-22 09:56:48Z fenrir $
 *
 * Authors: Sigmund Augdal <sigmunau@idi.ntnu.no>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111, USA.
 *****************************************************************************/

#include <stdlib.h>
#include <vlc/vlc.h>
#include <vlc/input.h>

#include "input_internal.h"

struct stream_sys_t
{
    int64_t     i_pos;      /* Current reading offset */
    int64_t     i_size;
    uint8_t    *p_buffer;

};

static int  AStreamReadMem( stream_t *, void *p_read, int i_read );
static int  AStreamPeekMem( stream_t *, uint8_t **pp_peek, int i_read );
static int  AStreamControl( stream_t *, int i_query, va_list );

/****************************************************************************
 * stream_MemoryNew: create a stream from a buffer
 ****************************************************************************/
stream_t *__stream_MemoryNew( vlc_object_t *p_this, uint8_t *p_buffer,
                              int64_t i_size )
{
    stream_t *s = vlc_object_create( p_this, VLC_OBJECT_STREAM );
    stream_sys_t *p_sys;

    if( !s )
        return NULL;

    s->p_sys = p_sys = malloc( sizeof( stream_sys_t ) );
    p_sys->i_pos = 0;
    p_sys->i_size = i_size;
    p_sys->p_buffer = p_buffer;

    s->pf_block  = NULL;
    s->pf_read   = AStreamReadMem;    /* Set up later */
    s->pf_peek   = AStreamPeekMem;
    s->pf_control= AStreamControl;
    vlc_object_attach( s, p_this );
    
    return s;
}

void stream_MemoryDelete( stream_t *s, vlc_bool_t b_free_buffer )
{
    if( b_free_buffer )
    {
        free( s->p_sys->p_buffer );
    }
    free( s->p_sys );
    vlc_object_destroy( s );
}
/****************************************************************************
 * AStreamControl:
 ****************************************************************************/
static int AStreamControl( stream_t *s, int i_query, va_list args )
{
    stream_sys_t *p_sys = s->p_sys;

    vlc_bool_t *p_bool;
    int64_t    *pi_64, i_64;
    int        i_int;

    switch( i_query )
    {
        case STREAM_GET_SIZE:
            pi_64 = (int64_t*)va_arg( args, int64_t * );
            *pi_64 = p_sys->i_size;
            break;

        case STREAM_CAN_SEEK:
            p_bool = (vlc_bool_t*)va_arg( args, vlc_bool_t * );
            *p_bool = VLC_TRUE;
            break;

        case STREAM_CAN_FASTSEEK:
            p_bool = (vlc_bool_t*)va_arg( args, vlc_bool_t * );
            *p_bool = VLC_TRUE;
            break;

        case STREAM_GET_POSITION:
            pi_64 = (int64_t*)va_arg( args, int64_t * );
            *pi_64 = p_sys->i_pos;
            break;

        case STREAM_SET_POSITION:
            i_64 = (int64_t)va_arg( args, int64_t );
            i_64 = __MAX( i_64, 0 );
            i_64 = __MIN( i_64, s->p_sys->i_size ); 
            p_sys->i_pos = i_64;

        case STREAM_GET_MTU:
            return VLC_EGENERIC;

        case STREAM_CONTROL_ACCESS:
            i_int = (int) va_arg( args, int );
            msg_Err( s, "Hey, what are you thinking ?"
                     "DON'T USE STREAM_CONTROL_ACCESS !!!" );
            return VLC_EGENERIC;

        default:
            msg_Err( s, "invalid stream_vaControl query=0x%x", i_query );
            return VLC_EGENERIC;
    }
    return VLC_SUCCESS;
}

static int  AStreamReadMem( stream_t *s, void *p_read, int i_read )
{
    stream_sys_t *p_sys = s->p_sys;
    int i_res = __MIN( i_read, p_sys->i_size - p_sys->i_pos );
    memcpy( p_read, p_sys->p_buffer + p_sys->i_pos, i_res );
    p_sys->i_pos += i_res;
    return i_res;
}

static int  AStreamPeekMem( stream_t *s, uint8_t **pp_peek, int i_read )
{
    stream_sys_t *p_sys = s->p_sys;
    int i_res = __MIN( i_read, p_sys->i_size - p_sys->i_pos );
    *pp_peek = p_sys->p_buffer + p_sys->i_pos;
    return i_res;
}
