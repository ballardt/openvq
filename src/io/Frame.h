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

#ifndef __FRAME_H
#define __FRAME_H

#include <opencv2/opencv.hpp>

struct Frame {
    Frame(cv::Mat y, cv::Mat u, cv::Mat v);
    Frame(int rows, int cols, int type);

    cv::Mat Y, U, V;

    void adjustROI(int dtop, int dbottom, int dleft, int dright);
};

#endif