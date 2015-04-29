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

#include <iomanip>
#include "Metrics.h"
#include "opvq/OPVQ.h"
#include "psnr/PSNR.h"
#include "ssim/SSIM.h"

#define NEW_INSTANCE(__alg__) []() -> std::unique_ptr<__alg__> { return std::unique_ptr<__alg__>(new __alg__()); }

std::vector<Metric> Metrics::metrics = {
        {"opvq", "Open Perceptual Video Quality metric", NEW_INSTANCE(OPVQ)},
        {"psnr", "Peak Signal-to-Noise Ratio", NEW_INSTANCE(PSNR)},
        {"ssim", "Structural Similarity Index", NEW_INSTANCE(SSIM)}
};

std::unique_ptr<Algorithm> Metrics::getAlgorithm(const std::string &command) {
    for (auto metric : metrics) {
        if (command == metric.command) {
            return metric.factory();
        }
    }

    throw std::runtime_error("Unknown command: " + command);
}

std::string Metrics::availableCommands() {
    std::ostringstream s;
    s << "Available commands: " << std::endl;
    for (auto metric : metrics) {
        s << std::setw(2) << "";
        s << std::setw(24) << std::left << metric.command;
        s << std::setw(60) << std::left << metric.description;
        s << std::endl;
    }

    return s.str();
}
