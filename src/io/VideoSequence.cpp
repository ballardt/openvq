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

#include <io/FileWriter.h>
#include <sstream>
#include "config.h"
#include "VideoSequence.h"

Logger Decoder::logger = Logger("Decoder");

Decoder::Decoder() : formatContext(NULL), codecContext(NULL), codec(NULL) {
}

void Decoder::err(const char *msg, int errNum) {
    char outErr[1024] = {0};
    av_strerror(errNum, outErr, 1024);
    std::stringstream what;
    what << msg << ": " << outErr;
    throw std::runtime_error(what.str().c_str());
}

void Decoder::loadVideo(std::string &url) {
    formatContext = avformat_alloc_context();

    int ret = avformat_open_input(&formatContext, url.c_str(), NULL, NULL);
    if (ret < 0) {
        std::string msg = "Could not open file " + url;
        err(msg.c_str(), ret);
    }

    ret = avformat_find_stream_info(formatContext, NULL);
    if (ret < 0) {
        err("couldn't find stream info for file", ret);
    }

    videoStream = -1;
    for (unsigned int i = 0; i < formatContext->nb_streams; i++) {
        if (formatContext->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoStream = i;
            break;
        }
    }
    if (videoStream == -1) {
        err("Didn't find video stream in file", -1);
    }
    codecContext = formatContext->streams[videoStream]->codec;

    codec = avcodec_find_decoder(codecContext->codec_id);
    if (codec == NULL) {
        err("video stream codec not found for file", -2);
    }

    if (codec->capabilities & CODEC_CAP_TRUNCATED)
        codecContext->flags |= CODEC_FLAG_TRUNCATED;

    ret = avcodec_open2(codecContext, codec, NULL);
    if (ret < 0) {
        err("could not open codec", ret);
    }

    videoInfo.width = codecContext->width;
    videoInfo.height = codecContext->height;
    videoInfo.duration = static_cast<float>(formatContext->duration / AV_TIME_BASE);
    videoInfo.avg_framerate = GET_FRAME_RATE(formatContext->streams[videoStream]);
    videoInfo.filename = formatContext->filename;
    videoInfo.frame_count = countFrames();
}

int Decoder::countFrames() {
    int frameCount = 0;
    bool isLastFrame = false;
    AVPacket packet;
    AVFrame *frame = AVFRAME_ALLOC();

    for (;;) {
        bool noError = getNextFrame(&packet, frame, &isLastFrame);
        if (!noError || isLastFrame) {
            break;
        }
        frameCount++;
    }
    AVFRAME_FREE(&frame);
    rewindAndFlushDecoder();

    return frameCount;
}

bool Decoder::getNextFrame(AVPacket *packet, AVFrame *frame, bool *isLastFrame) {
    if (!formatContext)
        return false;

    av_init_packet(packet);

    int got_frame = 0;
    bool eof = true;
    while (!av_read_frame(formatContext, packet)) {
        if (packet->stream_index == videoStream) {
            int nbytes = avcodec_decode_video2(codecContext, frame, &got_frame, packet);
            if (nbytes < 0) {
                err("Error while decoding frame", nbytes);
            }
            if (got_frame) {
                eof = false;
                break;
            }
        }
    }

    *isLastFrame = eof;
    return true;
}

void Decoder::rewindAndFlushDecoder() {
    av_seek_frame(formatContext, videoStream, 0, AVSEEK_FLAG_BACKWARD);
    avcodec_flush_buffers(codecContext);
}


void Decoder::freeBuffers() {
    avformat_close_input(&formatContext);
}

Logger VideoSequence::logger = Logger("VideoSequence");

VideoSequence::~VideoSequence() {
    decoder.freeBuffers();
}

VideoInfo VideoSequence::init(std::string &url, int maxFrames, AVPixelFormat pixelFormat) {
    this->maxFrames = maxFrames;
    this->pixelFormat = pixelFormat;
    frameCounter = 0;
    decoder.loadVideo(url);
    return decoder.getVideoInfo();
}

std::shared_ptr<Frame> VideoSequence::nextFrame() {
    AVPacket packet;
    AVFrame *frame;
    frame = AVFRAME_ALLOC();

    bool isLastFrame = false;

    bool noError = decoder.getNextFrame(&packet, frame, &isLastFrame);

    if (!noError || isLastFrame) {
        AVFRAME_FREE(&frame);
        if (isLastFrame) {
            logger(DEBUG) << "Reached end of the sequence at frame number: " << frameCounter;
        }
        return NULL;
    }

    SwsContext *ctxt = sws_getContext(frame->width, frame->height, (AVPixelFormat) frame->format,
                                      frame->width, frame->height, pixelFormat, SWS_X, nullptr, nullptr, nullptr);

    if (ctxt == nullptr) {
        throw std::runtime_error("Could not convert input format to desired pixel format");
    }

    // TODO: Derive channel sizes from pixelFormat instead of allocating for 4:4:4
    cv::Mat y(frame->height, frame->width, CV_8UC1);
    cv::Mat u(frame->height, frame->width, CV_8UC1);
    cv::Mat v(frame->height, frame->width, CV_8UC1);

    std::uint8_t *channels[3] = {y.ptr<std::uint8_t>(0), u.ptr<std::uint8_t>(0), v.ptr<std::uint8_t>(0)};
    int lineSizes[3] = {frame->width, frame->width, frame->width};
    sws_scale(ctxt, frame->data, frame->linesize, 0, frame->height, channels, lineSizes);
    sws_freeContext(ctxt);
    AVFRAME_FREE(&frame);
    av_free_packet(&packet);

    if (++frameCounter == maxFrames)
        logger(DEBUG) << "Read max number of frames (" << maxFrames << ")";

    return std::make_shared<Frame>(y, u, v);
}

void VideoSequence::rewind() {
    if (frameCounter > 0) {
        frameCounter = 0;
        decoder.rewindAndFlushDecoder();
    }
}

VideoInfo &Decoder::getVideoInfo() {
    return videoInfo;
}

