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

#include "SpatialAlignment.h"


Logger SpatialAlignment::logger = Logger("SpatialAlignment");

cv::Point2i SpatialAlignment::spatialOffsetDetermination(std::shared_ptr<Frame> srcFrame, std::shared_ptr<Frame> pvsFrame, int crop) {
    double minimizedError = std::numeric_limits<double>::max();

    cv::Mat src(srcFrame->Y);
    src.adjustROI(-crop, -crop, -crop, -crop);

    cv::Point2i offset;
    for (int dx = -1; dx <= 1; dx++) {
        for (int dy = -1; dy <= 1; dy++) {
            cv::Mat candidate, diffSquared;
            candidate = cv::Mat(pvsFrame->Y);
            candidate.adjustROI(-(crop + dy), -(crop - dy), -(crop + dx), -(crop - dx));
            cv::pow(src - candidate, 2, diffSquared);
            double error = cv::sum(diffSquared)[0];

            if (error < minimizedError) {
                minimizedError = error;
                offset.x = dx;
                offset.y = dy;
            }
        }
    }
    return offset;
}

void SpatialAlignment::cropAndAlign(std::shared_ptr<Frame> srcFrame, std::shared_ptr<Frame> pvsFrame, int crop, cv::Point2i offset) {
    srcFrame->adjustROI(-crop, -crop, -crop, -crop);
    pvsFrame->adjustROI(-(crop + offset.y), -(crop - offset.y), -(crop + offset.x), -(crop - offset.x));
}
