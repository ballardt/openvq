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

#ifndef VideoSequence_h__
#define VideoSequence_h__

#include <vector>
#include <string>
#include <memory>
#include <opencv2/opencv.hpp>

#include <io/Logger.h>
#include "config.h"
#include "Frame.h"


struct VideoInfo {
    int width;
    int height;
    float duration;
    int frame_count;
    std::string filename;
    float avg_framerate;
};

class Decoder {
public:
    Decoder();

    void loadVideo(std::string &url);

    bool getNextFrame(AVPacket *packet, AVFrame *frame, bool *isLastFrame);

    VideoInfo &getVideoInfo();

    void freeBuffers();

    void rewindAndFlushDecoder();

private:
    AVFormatContext *formatContext;
    AVCodecContext *codecContext;
    AVCodec *codec;
    int videoStream;
    VideoInfo videoInfo;

    void err(const char *msg, int errNum);

    static Logger logger;
};


class VideoSequence {
public:
    VideoInfo init(std::string &url, int maxFrames, AVPixelFormat pixelFormat);

    ~VideoSequence();

    std::shared_ptr<Frame> nextFrame();

    void rewind();

private:
    Decoder decoder;
    int maxFrames;
    int frameCounter;
    AVPixelFormat pixelFormat;
    static Logger logger;
};

#endif //VideoSequence_h__
