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

#ifndef SSIM_H
#define SSIM_H


#include <io/Frame.h>
#include "metrics/Algorithm.h"

class SSIM : public FullReferenceAlgorithm {
public:
    SSIM();

    virtual void init(int argc, const char **argv) override;

    int run() override;

private:
    bool enableSpatialAlignment;
    double ssimAccum;
    int framesCalculated;

    double ssimWindow(cv::Mat &src, cv::Mat &pvs);

    void addFrame(std::shared_ptr<const Frame> srcFrame, std::shared_ptr<const Frame> pvsFrame);

    double calcSsim();
};

#endif