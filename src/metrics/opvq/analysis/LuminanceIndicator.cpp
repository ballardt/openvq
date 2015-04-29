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

#include "LuminanceIndicator.h"
#include <numeric>

Logger LuminanceIndicator::logger = Logger("LuminanceIndicator");

LuminanceIndicator::LuminanceIndicator(unsigned int sequenceLength, int width, int height)
        : weightedL5NormValues(sequenceLength) {
    wij = cv::Mat(height, width, CV_64FC1);
    wijSum = 0;
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            double sinw = std::sin(M_PI * ((double) x / (double) width));
            double sinh = std::sin(M_PI * ((double) y / (double) height));
            wij.at<double>(y, x) = std::abs(sinw * sinh);
            wijSum += wij.at<double>(y, x);
        }
    }
}

void LuminanceIndicator::analyzeFrame(std::shared_ptr<const Frame> srcCurr, std::shared_ptr<const Frame> pvsCurr,
                                      std::shared_ptr<const Frame> srcEdge, std::shared_ptr<const Frame> pvsEdge,
                                      int t) {
    cv::Mat corrSA, corrPA;
    srcCurr->Y.convertTo(corrSA, CV_64FC1);
    pvsCurr->Y.convertTo(corrPA, CV_64FC1);

    //dev = max( |SaY[i,j,t]-100|, |PaY[i,j,t]-100)
    cv::Mat dev, SaYSub100, PaYSub100;
    cv::subtract(corrSA, cv::Scalar(100.0), SaYSub100);
    cv::subtract(corrPA, cv::Scalar(100.0), PaYSub100);
    SaYSub100 = cv::abs(SaYSub100);
    PaYSub100 = cv::abs(PaYSub100);
    cv::max(SaYSub100, PaYSub100, dev);

    //eYNumer: PedgeY - SedgeY
    cv::Mat eYNumer;
    cv::subtract(pvsEdge->Y, srcEdge->Y, eYNumer);

    //eYDenom: SedgeY + 80 + dev
    cv::Mat eYDenom;
    cv::add(srcEdge->Y, cv::Scalar(80.0), eYDenom);
    cv::add(eYDenom, dev, eYDenom);

    //eY: eYNumer/eYDenom
    cv::Mat eY;
    cv::divide(eYNumer, eYDenom, eY);

    //Not described in paper.
    cv::multiply(eY, cv::Scalar(80.0), eY);

    //eYClipped: -40 <= eY <= 40
    cv::min(eY, cv::Scalar(40.0), eY);
    cv::max(eY, cv::Scalar(-40.0), eY);

    //Final Matrices numer: |eY|^5 * wij
    eY = cv::abs(eY);
    cv::pow(eY, 5.0, eY);
    cv::multiply(eY, wij, eY);
    double finalSum = cv::sum(eY)[0];
    weightedL5NormValues[t] = cv::pow(finalSum / wijSum, 0.2);
}

double LuminanceIndicator::getLuminanceIndicator() {
    double accumSum = std::accumulate(weightedL5NormValues.begin(), weightedL5NormValues.end(), 0.0);
    return (1.0 / static_cast<double>(weightedL5NormValues.size())) * accumSum;
}
