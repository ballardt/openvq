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

#ifndef __OPVQ_H
#define __OPVQ_H

#include <metrics/Algorithm.h>
#include <io/VideoProperties.h>

#include "score/DMOSMapper.h"

class OPVQ : public ParallelFullReferenceAlgorithm {
public:

    struct ResolutionData {
        ResolutionID id;
        int crop;
        MappingCoefficients coeff;
    };

    OPVQ();

    virtual void init(int argc, const char **argv) override;

    int run() override;

private:
    static std::vector<ResolutionData> supportedResolutions;

    bool enableSpatialAlignment;
    bool enableColourCorrection;
    ResolutionData res;
    int croppedWidth;
    int croppedHeight;

    virtual void validateInput(VideoInfo &srcInfo, VideoInfo &pvsInfo) override;
};

#endif //__OPVQ_H