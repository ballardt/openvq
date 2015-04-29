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

#include <opencv2/opencv.hpp>

#include "TemporalVariabilityIndicators.h"


Logger TemporalVariabilityIndicators::logger = Logger("TemporalVariabilityIndicators");

TemporalVariabilityIndicators::TemporalVariabilityIndicators(unsigned int sequenceLength)
        : d_omitted(1, sequenceLength - 1, CV_64FC1), d_introduced(1, sequenceLength - 1, CV_64FC1) {
}

void TemporalVariabilityIndicators::analyzeFrame(std::shared_ptr<const Frame> srcCurr,
                                                 std::shared_ptr<const Frame> pvsCurr,
                                                 std::shared_ptr<const Frame> srcPrev,
                                                 std::shared_ptr<const Frame> pvsPrev, int t) {
    cv::Mat saDiff, paDiff;
    cv::Mat(cv::abs(srcCurr->Y - srcPrev->Y)).convertTo(saDiff, CV_64F);
    cv::Mat(cv::abs(pvsCurr->Y - pvsPrev->Y)).convertTo(paDiff, CV_64F);
    cv::Mat d = saDiff - paDiff;

    /* L norm over space (== mean) */
    cv::Mat do_orig(cv::max(d, 0));
    double do_t = cv::mean(do_orig)[0];

    /* L5 norm over space */
    cv::Mat di_orig(cv::abs(cv::min(d, 0))), di_t;
    cv::pow(di_orig, 5.0, di_orig);
    cv::pow(cv::sum(di_orig / (di_orig.cols * di_orig.rows)), 1.0 / 5.0, di_t);

    d_omitted.at<double>(t - 1) = do_t;
    d_introduced.at<double>(t - 1) = di_t.at<double>(0);
}

double TemporalVariabilityIndicators::getOmittedComponentIndicator() {
    return cv::mean(d_omitted)[0];
}

double TemporalVariabilityIndicators::getIntroducedComponentIndicator() {
    return cv::norm(d_introduced, cv::NORM_L2) / cv::sqrt(d_introduced.cols);
}
