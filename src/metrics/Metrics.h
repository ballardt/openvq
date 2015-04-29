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

#ifndef __METRICS_H
#define __METRICS_H

#include <sstream>
#include "Algorithm.h"

struct Metric {
    std::string command;
    std::string description;
    std::function<std::unique_ptr<Algorithm>(void)> factory;
};

class Metrics {
public:
    static std::vector<Metric> metrics;

    static std::unique_ptr<Algorithm> getAlgorithm(const std::string &command);

    static std::string availableCommands();
};

#endif //__METRICS_H
