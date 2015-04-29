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

#include "VideoProperties.h"

const ResolutionInfo VideoProperties::resolutionInfo[] = {
        {640, 480}, // VGA
        {768, 480}, // WVGA
        {1920, 1080}, // FHD
        {1280, 720}, //HD
        {960, 540}, // qHD
        {2560, 1440}, // QHD,
        {3840, 2160}, //UHD4K
        {5120, 2160}, //UHD5K
        {352, 288},  // CIF
        {176, 144}   // QCIF
};

ResolutionID VideoProperties::identifyResolution(int width, int height) {
    for (int i = 0; i != RES_UNSUPPORTED; i++) {
        if (width == VideoProperties::resolutionInfo[i].width && height == VideoProperties::resolutionInfo[i].height) {
            return static_cast<ResolutionID>(i);
        }
    }
    return RES_UNSUPPORTED;
}
