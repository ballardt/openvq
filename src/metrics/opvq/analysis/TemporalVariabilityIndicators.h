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

#ifndef TEMPORALVARIABILITYINDICATORS_H
#define TEMPORALVARIABILITYINDICATORS_H

#include <memory>
#include <io/Frame.h>
#include <io/Logger.h>


class TemporalVariabilityIndicators {
public:
    TemporalVariabilityIndicators(unsigned int sequenceLength);

    void analyzeFrame(std::shared_ptr<const Frame> srcCurr, std::shared_ptr<const Frame> pvsCurr,
                      std::shared_ptr<const Frame> srcPrev, std::shared_ptr<const Frame> pvsPrev, int t);

    double getOmittedComponentIndicator();

    double getIntroducedComponentIndicator();

private:
    cv::Mat d_omitted;
    cv::Mat d_introduced;

    static Logger logger;
};

#endif // TEMPORALVARIABILITYINDICATORS_H