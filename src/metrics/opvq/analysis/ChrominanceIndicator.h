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

#ifndef ChrominanceIndicator_h
#define ChrominanceIndicator_h

#include <memory>
#include <io/Logger.h>
#include <io/Frame.h>
#include "EdginessImage.h"


class ChrominanceIndicator {
public:
    ChrominanceIndicator(unsigned int sequenceLength, int width, int height);

    void analyzeFrame(std::shared_ptr<const Frame> srcCurr, std::shared_ptr<const Frame> pvsCurr,
                      std::shared_ptr<const Frame> srcEdge, std::shared_ptr<const Frame> pvsEdge, int t);

    double getChrominanceIndicator();


private:
    static Logger logger;

    cv::Mat wij;
    double wijSum;

    std::vector<double> eCbValues;
    std::vector<double> eCrValues;
};

#endif //ChrominanceIndicator_h
 