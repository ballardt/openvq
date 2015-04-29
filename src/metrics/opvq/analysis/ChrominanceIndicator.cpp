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

#include "ChrominanceIndicator.h"

#include <opencv2/opencv.hpp>


Logger ChrominanceIndicator::logger = Logger("ChrominanceIndicator");

ChrominanceIndicator::ChrominanceIndicator(unsigned int sequenceLength, int width, int height) : eCbValues(
        sequenceLength), eCrValues(sequenceLength) {
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

double ChrominanceIndicator::getChrominanceIndicator() {
    double accumSum = 0.0;
    for (unsigned t = 0; t < eCbValues.size(); t++) {
        accumSum += eCbValues[t] + eCrValues[t];
    }
    return 0.5 * accumSum / static_cast<double>(eCbValues.size());
}

void ChrominanceIndicator::analyzeFrame(std::shared_ptr<const Frame> srcCurr, std::shared_ptr<const Frame> pvsCurr,
                                        std::shared_ptr<const Frame> srcEdge, std::shared_ptr<const Frame> pvsEdge,
                                        int t) {
    cv::Mat srcUCorr, srcVCorr, pvsUCorr, pvsVCorr;

    srcCurr->U.convertTo(srcUCorr, CV_64FC1);
    srcCurr->V.convertTo(srcVCorr, CV_64FC1);
    pvsCurr->U.convertTo(pvsUCorr, CV_64FC1);
    pvsCurr->V.convertTo(pvsVCorr, CV_64FC1);

    //MX = sqrt(pow(SaU-128) + pow(SaV-128));
    cv::Mat mxCbSub, mxCrSub, mxMat;
    cv::subtract(srcUCorr, cv::Scalar(128.0), mxCbSub);
    cv::subtract(srcVCorr, cv::Scalar(128.0), mxCrSub);
    cv::pow(mxCbSub, 2, mxCbSub);
    cv::pow(mxCrSub, 2, mxCrSub);
    cv::add(mxCbSub, mxCrSub, mxMat);
    cv::sqrt(mxMat, mxMat);

    //MY = sqrt(pow(PaU-128) + pow(PaV-128));
    cv::Mat myCbSub, myCrSub, myMat;
    cv::subtract(pvsUCorr, cv::Scalar(128.0), myCbSub);
    cv::subtract(pvsVCorr, cv::Scalar(128.0), myCrSub);
    cv::pow(myCbSub, 2, myCbSub);
    cv::pow(myCrSub, 2, myCrSub);
    cv::add(myCbSub, myCrSub, myMat);
    cv::sqrt(myMat, myMat);

    //devCbCr = max(cb, cr)
    cv::Mat devCbCr, ZP8MulDevCbCr;
    cv::max(mxMat, myMat, devCbCr);
    cv::multiply(cv::Scalar(0.8), devCbCr, ZP8MulDevCbCr);

    //eCB = (pvsEdgeU - srcEdgeU) / (srcEdgeU + 40 + (0.8*devCbCr))
    cv::Mat eCb, eCbNumer, eCbDenom;
    cv::subtract(pvsEdge->U, srcEdge->U, eCbNumer);
    cv::add(srcEdge->U, cv::Scalar(40.0), eCbDenom);
    cv::add(eCbDenom, ZP8MulDevCbCr, eCbDenom);
    cv::divide(eCbNumer, eCbDenom, eCb);
    cv::multiply(eCb, cv::Scalar(40.0), eCb);

    //eCR = (pvsEdgeV - srcEdgeV) / (srcEdgeV + 40 + (0.8*devCvCr))
    cv::Mat eCr, eCrNumer, eCrDenom;
    cv::subtract(pvsEdge->V, srcEdge->V, eCrNumer);
    cv::add(srcEdge->V, cv::Scalar(40.0), eCrDenom);
    cv::add(eCrDenom, ZP8MulDevCbCr, eCrDenom);
    cv::divide(eCrNumer, eCrDenom, eCr);
    cv::multiply(eCr, cv::Scalar(40.0), eCr);

    //eCb & eCr clipped [-40, 40]
    cv::Mat eCbClipped, eCrClipped;
    cv::min(eCb, cv::Scalar(40.0), eCbClipped);
    cv::max(eCbClipped, cv::Scalar(-40.0), eCbClipped);

    cv::min(eCr, cv::Scalar(40.0), eCrClipped);
    cv::max(eCrClipped, cv::Scalar(-40.0), eCrClipped);

    //mul with wij
    cv::Mat eCbWijMul, eCrWijMul;
    cv::multiply(cv::abs(eCbClipped), wij, eCbWijMul);
    cv::multiply(cv::abs(eCrClipped), wij, eCrWijMul);

    //div with wijsum
    cv::Mat eCbWijDiv, eCrWijDiv;
    cv::divide(eCbWijMul, cv::Scalar(wijSum), eCbWijDiv);
    cv::divide(eCrWijMul, cv::Scalar(wijSum), eCrWijDiv);

    //sum each matrix
    eCbValues[t] = cv::sum(eCbWijDiv)[0];
    eCrValues[t] = cv::sum(eCrWijDiv)[0];
}
