/*
 * This file is part of MPlayer.
 *
 * MPlayer is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * MPlayer is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with MPlayer; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "config.h"
#ifdef __MINIOS__
#include <mplaylib.h>
#else
#include <stdlib.h>
#endif
#include <string.h>

#include "stream.h"

static int open_s(stream_t *stream,int mode, void* opts, int* file_format) {
  stream->type = STREAMTYPE_DUMMY;

  return 1;
}


const stream_info_t stream_info_null = {
  "Null stream",
  "null",
  "Albeu",
  "",
  open_s,
  { "null", NULL },
  NULL,
  0 // Urls are an option string
};
