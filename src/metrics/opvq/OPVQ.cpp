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

#include <metrics/common/alignment/SpatialAlignment.h>
#include <metrics/common/alignment/ColourAlignment.h>
#include <io/VideoProperties.h>

#include "analysis/LuminanceIndicator.h"
#include "analysis/ChrominanceIndicator.h"
#include "analysis/TemporalVariabilityIndicators.h"
#include "score/DMOSMapper.h"
#include "OPVQ.h"


OPVQ::OPVQ()
        : ParallelFullReferenceAlgorithm("OPVQ") {
    options.add_options()
            ("disable-spatial-alignment", "Disable spatial alignment")
            ("disable-colour-correction", "Disable colour correction");
    res.id = RES_UNSUPPORTED;
}

void OPVQ::init(int argc, const char **argv) {
    ParallelFullReferenceAlgorithm::init(argc, argv);

    opts::variables_map vm;
    opts::parsed_options parsed = opts::command_line_parser(argc, argv).
            options(options).
            allow_unregistered().
            run();
    opts::store(parsed, vm);

    enableSpatialAlignment = !static_cast<bool>(vm.count("disable-spatial-alignment"));
    enableColourCorrection = !static_cast<bool>(vm.count("disable-colour-correction"));
}

void OPVQ::validateInput(VideoInfo &srcInfo, VideoInfo &pvsInfo) {
    FullReferenceAlgorithm::validateInput(srcInfo, pvsInfo);

    ResolutionID resolutionID = VideoProperties::identifyResolution(srcInfo.width, srcInfo.height);
    for (auto resolutionData : supportedResolutions) {
        if (resolutionData.id == resolutionID) {
            res = resolutionData;
        }
    }
    if (res.id == RES_UNSUPPORTED) {
        logger(WARN) << "Resolution is not supported, mapping to DMOS will not be accurate.";
        res = supportedResolutions.front();
    }
    croppedWidth = srcInfo.width - (2 * res.crop);
    croppedHeight = srcInfo.height - (2 * res.crop);
}

std::vector<OPVQ::ResolutionData> OPVQ::supportedResolutions = {
        {VGA,  12, DMOSMapper::VGAcoeff},
        {CIF,  6,  DMOSMapper::CIFcoeff},
        {QCIF, 3,  DMOSMapper::QCIFcoeff}
};

int OPVQ::run() {
    SpatialAlignment spatialAlignment;
    std::vector<cv::Point2i> spatialOffset(sequenceLength, cv::Point2i(0, 0));

    ColourAlignment srcColour(sequenceLength), pvsColour(sequenceLength);
    std::vector<cv::Mat> correctionCurves;

    LuminanceIndicator luminanceIndicator(sequenceLength, croppedWidth, croppedHeight);
    ChrominanceIndicator chrominanceIndicator(sequenceLength, croppedWidth, croppedHeight);
    TemporalVariabilityIndicators temporalVariabilityIndicators(sequenceLength);

    /* Alignment and correction */
    int passCount = 0;
    if (enableSpatialAlignment || enableColourCorrection) {
        logger(INFO) << "Pass " << ++passCount;
        makePass([&](std::shared_ptr<Frame> srcCurr, std::shared_ptr<Frame> pvsCurr, int tCurr) {
            if (enableSpatialAlignment) {
                spatialOffset[tCurr] = spatialAlignment.spatialOffsetDetermination(srcCurr, pvsCurr, res.crop);
            }
            spatialAlignment.cropAndAlign(srcCurr, pvsCurr, res.crop, spatialOffset[tCurr]);

            if (enableColourCorrection) {
                srcColour.analyzeFrame(srcCurr, tCurr);
                pvsColour.analyzeFrame(pvsCurr, tCurr);
            }
        });
    }

    /* If enabled, create colour correction curve from histograms */
    if (enableColourCorrection) {
        srcColour.createCumulative(croppedWidth, croppedHeight);
        pvsColour.createCumulative(croppedWidth, croppedHeight);
        correctionCurves = ColourAlignment::createCorrectionCurves(srcColour, pvsColour);
    }

    /* Main analysis */
    logger(INFO) << "Pass " << ++passCount;
    makePassWithPrev([&](std::shared_ptr<Frame> srcCurr, std::shared_ptr<Frame> pvsCurr, std::shared_ptr<Frame> srcPrev,
                         std::shared_ptr<Frame> pvsPrev, int tCurr) {
        spatialAlignment.cropAndAlign(srcCurr, pvsCurr, res.crop, spatialOffset[tCurr]);
        if (enableColourCorrection) {
            //applying correction curve to pvsraded signal.
            ColourAlignment::applyCorrectionCurve(pvsCurr, correctionCurves);
        }

        std::shared_ptr<Frame> srcEdge = EdginessImage::createEdginessImage(srcCurr);
        std::shared_ptr<Frame> pvsEdge = EdginessImage::createEdginessImage(pvsCurr);

        luminanceIndicator.analyzeFrame(srcCurr, pvsCurr, srcEdge, pvsEdge, tCurr);
        chrominanceIndicator.analyzeFrame(srcCurr, pvsCurr, srcEdge, pvsEdge, tCurr);

        if (tCurr > 0) {
            assert(srcPrev);
            assert(pvsPrev);
            temporalVariabilityIndicators.analyzeFrame(srcCurr, pvsCurr, srcPrev, pvsPrev, tCurr);
        }
    });

    /* Retrieve indicator values and map to score */
    std::vector<double> indicators(NUM_IND);
    indicators[LUMA_IND] = luminanceIndicator.getLuminanceIndicator();
    indicators[CHROMA_IND] = chrominanceIndicator.getChrominanceIndicator();
    indicators[INTRO_IND] = temporalVariabilityIndicators.getIntroducedComponentIndicator();
    indicators[OMIT_IND] = temporalVariabilityIndicators.getOmittedComponentIndicator();
    writeCSV(pvsURL, indicators);

    double aggregateScore = DMOSMapper::calculateAggregateScore(indicators, res.coeff);
    logger(INFO) << "Aggregated final score: " << aggregateScore;

    return 0;
}