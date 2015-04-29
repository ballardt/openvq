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

#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/parsers.hpp>
#include <metrics/common/alignment/SpatialAlignment.h>

#include "SSIM.h"


SSIM::SSIM()
        : FullReferenceAlgorithm("SSIM"),
          ssimAccum(0), framesCalculated(0) {
    options.add_options()("disable-spatial-alignment", "Disable spatial alignment");
}

void SSIM::init(int argc, const char **argv) {
    FullReferenceAlgorithm::init(argc, argv);

    opts::variables_map vm;
    opts::parsed_options parsed = opts::command_line_parser(argc, argv).
            options(options).
            allow_unregistered().
            run();
    opts::store(parsed, vm);

    enableSpatialAlignment = !static_cast<bool>(vm.count("disable-spatial-alignment"));
}

int SSIM::run() {

    SpatialAlignment spatialAlignment;

    makePass([&](std::shared_ptr<Frame> srcCurr, std::shared_ptr<Frame> pvsCurr, int tCurr) {
        if (enableSpatialAlignment) {
            int crop = 4;
            cv::Point2i sptialOffset = spatialAlignment.spatialOffsetDetermination(srcCurr, pvsCurr, crop);
            spatialAlignment.cropAndAlign(srcCurr, pvsCurr, crop, sptialOffset);
        }

        addFrame(srcCurr, pvsCurr);
    });

    double ssim = calcSsim();
    logger(INFO) << "SSIM: " << ssim;
    std::vector<double> values = {ssim};
    writeCSV(pvsURL, values);
    return 0;
}

double SSIM::calcSsim() {
    return ssimAccum / framesCalculated;
}


void SSIM::addFrame(std::shared_ptr<const Frame> srcFrame, std::shared_ptr<const Frame> pvsFrame) {
    double frameSsim = 0;
    int windowsCalculated = 0;
    cv::Mat srcY, pvsY;
    srcFrame->Y.copyTo(srcY);
    pvsFrame->Y.copyTo(pvsY);
    int width = srcY.cols;
    int height = srcY.rows;
    int displacement = 8;
    int windowWidth = 8;
    int windowHeight = 8;


    //todo: handle cases where width/height are not dividable by displacement value
    for (int j = 0; j < height; j += displacement) {
        cv::Mat srcYline = srcY.rowRange(j, j + windowHeight);
        cv::Mat pvsYline = pvsY.rowRange(j, j + windowHeight);
        for (int i = 0; i < width; i += displacement) {
            cv::Mat srcWindow = srcYline.colRange(i, i + windowWidth);
            cv::Mat pvsWindow = pvsYline.colRange(i, i + windowWidth);
            frameSsim += ssimWindow(srcWindow, pvsWindow);
            windowsCalculated++;
        }
    }
    ssimAccum += frameSsim / windowsCalculated;
    framesCalculated++;
}

double SSIM::ssimWindow(cv::Mat &srcWindow, cv::Mat &pvsWindow) {
    cv::Mat src, pvs;
    srcWindow.convertTo(src, CV_64FC1);
    pvsWindow.convertTo(pvs, CV_64FC1);
    cv::Scalar srcMean, pvsMean, srcStdDev, pvsStdDev;
    cv::meanStdDev(src, srcMean, srcStdDev);
    cv::meanStdDev(pvs, pvsMean, pvsStdDev);

    cv::Scalar srcVariance, pvsVariance;
    cv::pow(srcStdDev, 2.0, srcVariance);
    cv::pow(pvsStdDev, 2.0, pvsVariance);

    double windowSize = src.cols * src.rows;
    cv::Scalar covariance;
    cv::Mat srcDiff, pvsDiff, covarianceUnsummed;
    cv::subtract(src, srcMean, srcDiff);
    cv::subtract(pvs, pvsMean, pvsDiff);
    cv::multiply(srcDiff, pvsDiff, covarianceUnsummed);
    cv::divide(cv::sum(covarianceUnsummed), cv::Scalar(windowSize), covariance);

    cv::Scalar srcMeanSquared, pvsMeanSquared;
    cv::pow(srcMean, 2.0, srcMeanSquared);
    cv::pow(pvsMean, 2.0, pvsMeanSquared);

    //todo: (0.01*dynamic range)^2
    double c1 = 6.5025;

    //todo: (0.03*dynamic range)^2
    double c2 = 58.5225;

    double numerator = (2 * srcMean[0] * pvsMean[0] + c1) * (2 * covariance[0] + c2);
    double denominator = (srcMeanSquared[0] + pvsMeanSquared[0] + c1) * (srcVariance[0] + pvsVariance[0] + c2);
    return numerator / denominator;
}
