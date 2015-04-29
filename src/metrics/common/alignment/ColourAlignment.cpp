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

#include "ColourAlignment.h"


ColourAlignment::ColourAlignment(unsigned int sequenceLength)
        : logger("ColourAlignment"),
          rawHistY(sequenceLength),
          rawHistU(sequenceLength),
          rawHistV(sequenceLength),
          cumulativeY(256, 1, CV_32FC1, cv::Scalar(0)),
          cumulativeU(256, 1, CV_32FC1, cv::Scalar(0)),
          cumulativeV(256, 1, CV_32FC1, cv::Scalar(0)) {
}

void ColourAlignment::calculateLumaCorrectionCurve(cv::Mat hsIn, cv::Mat hpIn,
        cv::Mat HCsIn, cv::Mat HCpIn, cv::Mat &out_correctionCurveIn) {
    int binS = 0;
    int binP = 128;

    std::vector<float> hs, HCs, hp, HCp;
    std::vector<std::uint8_t> out_correctionCurve;
    out_correctionCurve.resize(256);
    hsIn.copyTo(hs);
    hpIn.copyTo(hp);
    HCsIn.copyTo(HCs);
    HCpIn.copyTo(HCp);

    float fracS = hs[binS];
    float cumFracS = HCs[binS];
    float fracP = hp[binP];
    float cumFracP = HCp[binP];


    float tarS = 0;
    int steepnessS = 0;
    while ((HCs[binS] < cumFracP) && binS < 255) {
        binS++;
    }

    for (binP = 128; binP < 256; binP++) {
        fracP = hp[binP];
        cumFracP = HCp[binP];
        if (binS < 255) {
            if ((fracS < 0.0008) && fracP < 0.0008) {
                binS++;
                fracS = hs[binS];
                cumFracS = HCs[binS];

                tarS = static_cast<float>(binS);
            }
            else {
                steepnessS = 0;
                while (cumFracS < cumFracP && binS < 255 && steepnessS <= 50) {
                    binS++;
                    steepnessS++;
                    fracS = hs[binS];
                    cumFracS = HCs[binS];
                }
                if (cumFracS >= cumFracP) {
                    tarS = (binS - 1) * (HCs[binS] - cumFracP) + binS * ((cumFracP - HCs[binS - 1]) / (HCs[binS] - HCs[binS - 1]));
                    tarS = std::max(tarS, static_cast<float>(binS - 1));
                    tarS = std::min(tarS, static_cast<float>(binS));
                }
                else {
                    tarS = static_cast<float>(binS);
                }
            }

        }
        out_correctionCurve[binP] = static_cast<std::uint8_t>(std::round(tarS));
    }

    binS = 255;
    fracS = hs[binS];
    cumFracS = HCs[binS];

    cumFracP = HCp[128];
    //no =
    while (HCs[binS] >= cumFracP && binS >= 0) {
        binS--;
    }

    for (binP = 127; binP >= 0; binP--) {
        fracP = hp[binP];
        cumFracP = HCp[binP];
        //no =
        if (binS > 0) {
            if (fracS < 0.0008 && fracP < 0.0008) {
                binS--;
                fracS = hs[binS];
                cumFracS = HCs[binS];
                tarS = static_cast<float>(binS);
            }
            else {
                steepnessS = 0;
                while (cumFracS > cumFracP && binS >= 0 && steepnessS <= 50) {
                    binS--;
                    steepnessS++;
                    fracS = hs[binS];
                    cumFracS = HCs[binS];
                }
                if (cumFracS <= cumFracP) {
                    tarS = binS * (HCs[binS + 1] - cumFracP) + (binS + 1) * ((cumFracP - HCs[binS]) / (HCs[binS + 1] - HCs[binS]));
                    tarS = std::max(tarS, static_cast<float>(binS));
                    tarS = std::min(tarS, static_cast<float>(binS + 1));
                }
                else {
                    tarS = static_cast<float>(binS);
                }
            }
        }
        out_correctionCurve[binP] = static_cast<std::uint8_t>(std::round(tarS));
    }

    cv::Mat temp(256, 1, CV_8UC1);
    memcpy(temp.data, out_correctionCurve.data(), 256);
    temp.copyTo(out_correctionCurveIn);
}

void ColourAlignment::analyzeFrame(std::shared_ptr<const Frame> f, int t) {
    rawHistY[t] = cv::Mat(256, 1, CV_32FC1, cv::Scalar(0));
    rawHistU[t] = cv::Mat(256, 1, CV_32FC1, cv::Scalar(0));
    rawHistV[t] = cv::Mat(256, 1, CV_32FC1, cv::Scalar(0));

    static int sizes[] = {256};
    static int channels[] = {0};
    static float range[] = {0, 256};
    static const float *ranges[] = {range};

    cv::calcHist(&f->Y, 1, channels, cv::Mat(), rawHistY[t], 1, sizes, ranges, true, false);
    cv::calcHist(&f->U, 1, channels, cv::Mat(), rawHistU[t], 1, sizes, ranges, true, false);
    cv::calcHist(&f->V, 1, channels, cv::Mat(), rawHistV[t], 1, sizes, ranges, true, false);
}

void ColourAlignment::calculateChromaCorrectionCurve(cv::Mat hsIn, cv::Mat hpIn,
        cv::Mat HCsIn, cv::Mat HCpIn, cv::Mat &out_correctionCurveIn) {
    int binS = 0;
    int binP = 128;

    std::vector<float> hs, HCs, hp, HCp;
    std::vector<std::uint8_t> out_correctionCurve;
    out_correctionCurve.resize(256);
    hsIn.copyTo(hs);
    hpIn.copyTo(hp);
    HCsIn.copyTo(HCs);
    HCpIn.copyTo(HCp);

    double fracS = hs[binS];
    double cumFracS = HCs[binS];
    double fracP = hp[binP];
    double cumFracP = HCp[binP];
    double tarS = 0.0;
    int oldBinS = binS;
    double steps = 0.0;

    while ((HCs[binS] < cumFracP) && (binS < 255)) {
        binS++;
    }
    for (binP = 128; binP < 256; binP++) {
        steps = steps + 0.5;
        fracP = hp[binP];
        cumFracP = HCp[binP];

        if (binS < 255) {
            if ((fracS < 0.0008) && (fracP < 0.0008)) {
                binS++;
                fracS = hs[binS];
                cumFracS = HCs[binS];
                tarS = static_cast<double>(binS);
            }
            else {
                while ((cumFracS < cumFracP) && (binS < 255)) {
                    binS++;
                    fracS = hs[binS];
                    cumFracS = HCs[binS];
                }
                if (cumFracS >= cumFracP) {
                    tarS = (binS - 1) * (HCs[binS] - cumFracP) + (binS) * ((cumFracP - HCs[binS - 1]) / (HCs[binS] - HCs[binS - 1]));
                    tarS = std::max(tarS, static_cast<double>(binS - 1));
                    tarS = std::min(tarS, static_cast<double>(binS));
                }
                else {
                    tarS = static_cast<double>(binS);
                }
            }
        }
        if (steps >= 1) {
            if (((binS - oldBinS) / steps) < 1) {
                binS++;
                tarS++;
            }
            steps = 0;
            oldBinS = binS;
        }
        out_correctionCurve[binP] = static_cast<int>(round(tarS));
    }

    binS = 255;
    fracS = hs[binS];
    cumFracS = HCs[binS];
    oldBinS = binS;

    steps = 0;
    cumFracP = HCp[128];
    while ((HCs[binS] >= cumFracP) && (binS > 0)) {
        binS--;
    }
    for (binP = 127; binP >= 0; binP--) {
        steps = steps + 0.5;
        fracP = hp[binP];
        cumFracP = HCp[binP];
        if (binS > 0) {//paper says >=, seems to crash with binS becoming -1
            if ((fracS < 0.0008) && (fracP < 0.0008)) {
                binS--;
                fracS = hs[binS];
                cumFracS = HCs[binS];
            }
            else {
                while ((cumFracS > cumFracP) && (binS >= 0)) {
                    binS--;
                    fracS = hs[binS];
                    cumFracS = HCs[binS];
                    tarS = static_cast<double>(binS); //not in the paper, but its here in the luma function
                }
                if (cumFracS <= cumFracP) {
                    tarS = (binS) * (HCs[binS + 1] - cumFracP) + (binS + 1) * ((cumFracP - HCs[binS]) / (HCs[binS + 1] - HCs[binS]));
                    tarS = std::max(tarS, static_cast<double>(binS));
                    tarS = std::min(tarS, static_cast<double>(binS + 1));
                }
                else {
                    tarS = static_cast<double>(binS);
                }
            }
        }
        if (steps >= 1) {
            if ((static_cast<double>(oldBinS - binS) / steps) < 1.0) {
                binS++;
                tarS++;
            }
            steps = 0;
            oldBinS = binS;
        }
        out_correctionCurve[binP] = static_cast<int>(round(tarS));
    }
    cv::Mat temp(256, 1, CV_8UC1);
    memcpy(temp.data, out_correctionCurve.data(), 256);
    temp.copyTo(out_correctionCurveIn);
}

void ColourAlignment::applyCorrectionCurve(std::shared_ptr<Frame> f, std::vector<cv::Mat> &curve) {
    cv::Mat *data[] = {&f->Y, &f->U, &f->V};
    for (int c = 0; c < 3; c++) {
        std::uint8_t *corrCurve = curve[c].ptr<std::uint8_t>(0);
        for (int row = 0; row < data[c]->rows; row++) {
            for (uint8_t *srcPx = data[c]->ptr(row); srcPx < data[c]->ptr(row) + data[c]->cols; srcPx++) {
                *srcPx = corrCurve[*srcPx];
            }
        }
    }
}

void ColourAlignment::createCumulative(int width, int height) {
    rawHistY[0].copyTo(histY);
    rawHistU[0].copyTo(histU);
    rawHistV[0].copyTo(histV);
    for (unsigned t = 1; t < rawHistY.size(); t++) {
        histY += rawHistY[t];
        histU += rawHistU[t];
        histV += rawHistV[t];
    }

    double scale = 1.0 / (rawHistY.size() * width * height);

    cv::multiply(histY, cv::Scalar(scale), histY);
    cv::multiply(histU, cv::Scalar(scale), histU);
    cv::multiply(histV, cv::Scalar(scale), histV);

    cumulativeY.at<float>(0, 0) = histY.at<float>(0, 0);
    cumulativeU.at<float>(0, 0) = histU.at<float>(0, 0);
    cumulativeV.at<float>(0, 0) = histV.at<float>(0, 0);
    for (int i = 1; i < histY.rows; i++) {
        cv::add(histY.row(i), cumulativeY.row(i - 1), cumulativeY.row(i));
        cv::add(histU.row(i), cumulativeU.row(i - 1), cumulativeU.row(i));
        cv::add(histV.row(i), cumulativeV.row(i - 1), cumulativeV.row(i));
    }
}

std::vector<cv::Mat> ColourAlignment::createCorrectionCurves(ColourAlignment &srcCA, ColourAlignment &pvsCA) {
    cv::Mat corrCurveY, corrCurveU, corrCurveV;

    calculateLumaCorrectionCurve(srcCA.histY, pvsCA.histY, srcCA.cumulativeY, pvsCA.cumulativeY, corrCurveY);
    calculateChromaCorrectionCurve(srcCA.histU, pvsCA.histU, srcCA.cumulativeU, pvsCA.cumulativeU, corrCurveU);
    calculateChromaCorrectionCurve(srcCA.histV, pvsCA.histV, srcCA.cumulativeV, pvsCA.cumulativeV, corrCurveV);

    std::vector<cv::Mat> correctionCurve = {corrCurveY, corrCurveU, corrCurveV};
    return correctionCurve;
}
