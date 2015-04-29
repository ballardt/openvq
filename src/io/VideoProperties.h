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

#ifndef __VIDEOPROPERTIES_H
#define __VIDEOPROPERTIES_H

#include <vector>

enum ResolutionID {
    VGA = 0,
    WVGA,
    FHD,
    HD,
    qHD,
    QHD,
    UHD4K,
    UHD5K,
    CIF,
    QCIF,
    RES_UNSUPPORTED
};

struct ResolutionInfo {
    int width;
    int height;
};

class VideoProperties {
public:
    static const ResolutionInfo resolutionInfo[];

    static ResolutionID identifyResolution(int width, int height);
};

#endif // __VIDEOPROPERTIES_H