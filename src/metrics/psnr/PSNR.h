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

#ifndef PSNR_H
#define PSNR_H

#include "io/Frame.h"
#include "metrics/Algorithm.h"

class PSNR : public FullReferenceAlgorithm {
public:
    PSNR();

    virtual void init(int argc, const char **argv) override;

    int run() override;

private:
    bool enableSpatialAlignment;
    double psnrAccum;
    int framesCalculated;

    //todo: equal src and pvs will return 0. Handle in the calling function to avoid division by zero.
    //todo: alternatively return <double>::max value as per definition the result is "infinitely high".
    double calcMSEFrame(std::shared_ptr<const Frame> srcFrame, std::shared_ptr<const Frame> pvsFrame);

    void addFrame(std::shared_ptr<const Frame> srcFrame, std::shared_ptr<const Frame> pvsFrame);

    double calcPsnr();
};

#endif