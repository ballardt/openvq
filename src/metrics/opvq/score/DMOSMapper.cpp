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

#include "DMOSMapper.h"

#include <cmath>
#include <algorithm>
#include <iomanip>
#include <fstream>


MappingCoefficients DMOSMapper::VGAcoeff = {
        {0.0000000, 0.0888870, 0.0000000, 0.0000000},
        {26.3458920, 11.9341383, 1603.3526610, 44.0389137},
        {5.5178358, -61.9967023, -12.8507869, -0.2219432},
        {0.1982675, 0.8956342, 0.0026048, 0.7256163},
        {-1.9184154, -14.5877780, 2.3705606, -15.7681800},
        63.1413711
};
MappingCoefficients DMOSMapper::CIFcoeff = {
        {0.00000000, 0.0888870, 0.00000000, 0.00000000},
        {26.34589190, 11.9341383, 1603.35266000, 44.03891370},
        {5.31724460, 492.6884156, -26163.42844000, -1.28731130},
        {0.18924704, 0.5660372, 0.00234274, 0.04960221},
        {-1.83546050, 7.2721399, 10.06642100, -0.51780410},
        1.6945190
};
MappingCoefficients DMOSMapper::QCIFcoeff = {
        {0.0000000, 0.0888870, 0.0000000, 0.0000000},
        {26.3458920, 11.9341383, 1603.3526600, 44.0389137},
        {6.1842156, 2.5008131, -5699.5850910, -0.8661656},
        {0.1683161, 1.3506481, 0.0036791, 0.3119277},
        {-1.4493381, -17.7480150, 8.7293193, -6.7674783},
        -0.9269844
};

Logger DMOSMapper::logger = Logger("DMOSMapper");

double DMOSMapper::calculateAggregateScore(std::vector<double> &indicators, MappingCoefficients coeff) {
    const char *shortnames[] = {"Luma", "Chroma", "Omitted", "Introduced"};

    double score = coeff.LinearOffset;
    std::cout << "LINEAR OFFSET: " << coeff.LinearOffset << std::endl;
    std::cout << std::left << std::fixed << std::setw(15) << "INDICATOR";
    std::cout << std::fixed << std::setw(15) << "VALUE";
    std::cout << std::fixed << std::setw(12) << "MIN";
    std::cout << std::fixed << std::setw(12) << "MAX";
    std::cout << std::fixed << std::setw(12) << "CONTRIBUTION" << std::endl;

    for (unsigned i = 0; i < NUM_IND; i++) {
        double Ilim = std::max(std::min(indicators[i], coeff.Imax[i]), coeff.Imin[i]);
        double contrib = coeff.w[i] / (1 + std::exp(coeff.alpha[i] * Ilim + coeff.beta[i]));
        score += contrib;
        std::cout << std::left << std::fixed << std::setw(15) << shortnames[i];
        std::cout << std::fixed << std::setw(15) << Ilim;
        std::cout << std::fixed << std::setw(12) << coeff.Imin[i];
        std::cout << std::fixed << std::setw(12) << coeff.Imax[i];
        std::cout << std::fixed << std::setw(12) << contrib << std::endl;
    }

    std::cout << "Final score: " << score << " (will be clipped to [1, 5])" << std::endl;

    return std::max(std::min(score, 5.0), 1.0);
}
