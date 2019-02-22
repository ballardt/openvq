/*
 * This file is part of the video quality assessment toolkit OpenVQ
 *
 * Copyright (C) 2015 Henrik Bjørlo, Kristian Skarseth, Carsten Griwodz
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License version 3
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

extern "C" {
#include "libavcodec/version.h"
#if LIBAVCODEC_VERSION_MAJOR < 55
#include "libavcodec/avcodec.h"
#define AVFRAME_ALLOC() avcodec_alloc_frame()
#define AVFRAME_FREE(__fptr) avcodec_free_frame(__fptr)
#else
#include "libavutil/frame.h"
#define AVFRAME_ALLOC() av_frame_alloc()
#define AVFRAME_FREE(__fptr) av_frame_free(__fptr)
#endif
#include "libavformat/version.h"
#include "libavformat/avformat.h"
#if LIBAVFORMAT_VERSION_MAJOR < 55
#define GET_FRAME_RATE(__vstreamptr) (((float) (__vstreamptr)->r_frame_rate.num) / ((float) (__vstreamptr)->r_frame_rate.den))
#else
#define GET_FRAME_RATE(__vstreamptr) (((float) (__vstreamptr)->avg_frame_rate.num) / ((float) (__vstreamptr)->avg_frame_rate.den))
#endif
#include "libswscale/swscale.h"
#include "libavutil/pixfmt.h"
}

#endif // __CONFIG_H
