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
#include <fstream>
#include <iomanip>

#include <opencv2/core/core.hpp>
#include <metrics/common/alignment/SpatialAlignment.h>
#include "PSNR.h"


PSNR::PSNR()
        : FullReferenceAlgorithm("PSNR"),
          psnrAccum(0), framesCalculated(0) {
    options.add_options()("disable-spatial-alignment", "Disable spatial alignment");
}

void PSNR::init(int argc, const char **argv) {
    FullReferenceAlgorithm::init(argc, argv);

    opts::variables_map vm;
    opts::parsed_options parsed = opts::command_line_parser(argc, argv).
            options(options).
            allow_unregistered().
            run();
    opts::store(parsed, vm);

    enableSpatialAlignment = !static_cast<bool>(vm.count("disable-spatial-alignment"));
}

int PSNR::run() {
    SpatialAlignment spatialAlignment;
    makePass([&](std::shared_ptr<Frame> srcCurr, std::shared_ptr<Frame> pvsCurr, int tCurr) {
        if (enableSpatialAlignment) {
            int crop = 1;
            cv::Point2i sptialOffset = spatialAlignment.spatialOffsetDetermination(srcCurr, pvsCurr, crop);
            spatialAlignment.cropAndAlign(srcCurr, pvsCurr, crop, sptialOffset);
        }

        addFrame(srcCurr, pvsCurr);
    });

    double psnr = calcPsnr();
    logger(INFO) << "PSNR: " << psnr;
    std::vector<double> values = {psnr};
    writeCSV(pvsURL, values);
    return 0;
}


double PSNR::calcPsnr() {
    return psnrAccum / framesCalculated;
}


void PSNR::addFrame(std::shared_ptr<const Frame> srcFrame, std::shared_ptr<const Frame> pvsFrame) {
    psnrAccum += calcMSEFrame(srcFrame, pvsFrame);
}


double PSNR::calcMSEFrame(std::shared_ptr<const Frame> srcFrame, std::shared_ptr<const Frame> pvsFrame) {
    cv::Mat res;
    cv::subtract(srcFrame->Y, pvsFrame->Y, res);
    cv::pow(res, 2, res);
    double sumRes = static_cast<double>(cv::sum(res)[0]);
    sumRes /= srcFrame->Y.cols * srcFrame->Y.rows;

    if (sumRes == 0.0) {
        sumRes = 1e-10;
    }

    framesCalculated++;
    //todo either 255^2(65025) or 235^2 (55225), last aparently suggested by VQEG
    return 10 * cv::log(65025 / sumRes) / cv::log(10);
}
