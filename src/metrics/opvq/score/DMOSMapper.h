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

#ifndef __DMOSMAPPER_H
#define __DMOSMAPPER_H

#include <vector>
#include <io/Logger.h>

enum {
    LUMA_IND = 0,
    CHROMA_IND,
    INTRO_IND,
    OMIT_IND,
    NUM_IND
};

struct MappingCoefficients {
    double Imin[NUM_IND];
    double Imax[NUM_IND];
    double w[NUM_IND];
    double alpha[NUM_IND];
    double beta[NUM_IND];
    double LinearOffset;
};

class DMOSMapper {
public:
    static MappingCoefficients VGAcoeff;
    static MappingCoefficients CIFcoeff ;
    static MappingCoefficients QCIFcoeff;

    static double calculateAggregateScore(std::vector<double> &indicators, MappingCoefficients coeff);

    static Logger logger;
};

#endif // __DMOSMAPPER_H