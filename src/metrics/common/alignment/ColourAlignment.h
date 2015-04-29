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

#ifndef CoarseLuminanceAlignment_h
#define CoarseLuminanceAlignment_h

#include <memory>
#include <vector>
#include <opencv2/opencv.hpp>

#include <io/Frame.h>
#include <io/Logger.h>


class ColourAlignment {
public:
    ColourAlignment(unsigned int sequenceLength);

    void analyzeFrame(std::shared_ptr<const Frame> f, int t);

    void createCumulative(int width, int height);

    static std::vector<cv::Mat> createCorrectionCurves(ColourAlignment &srcCA, ColourAlignment &pvsCA);

    static void calculateLumaCorrectionCurve(cv::Mat hsIn, cv::Mat hpIn, cv::Mat HCsIn, cv::Mat HCpIn, cv::Mat &out_correctionCurveIn);

    static void calculateChromaCorrectionCurve(cv::Mat hsIn, cv::Mat hpIn, cv::Mat HCsIn, cv::Mat HCpIn, cv::Mat &out_correctionCurveIn);

    static void applyCorrectionCurve(std::shared_ptr<Frame> f, std::vector<cv::Mat> &curve);

private:
    Logger logger;

    std::vector<cv::Mat> rawHistY;
    std::vector<cv::Mat> rawHistU;
    std::vector<cv::Mat> rawHistV;
    cv::Mat histY;
    cv::Mat histU;
    cv::Mat histV;
    cv::Mat cumulativeY;
    cv::Mat cumulativeU;
    cv::Mat cumulativeV;
};

#endif //CoarseLuminanceAlignment_h