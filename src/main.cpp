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

#include <string>
#include <iostream>
#include <cstring>

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>

extern "C" {
#include <libavformat/avformat.h>
};

#include "io/Logger.h"
#include "metrics/Metrics.h"

Logger mainLogger("OpenVQ");

namespace opts = boost::program_options;


int main(int argc, const char *argv[]) {
    opts::options_description globalOptions("Global options");
    globalOptions.add_options()
            ("help,h", "Print help message")
            ("log-level", opts::value<std::string>(), "Set log level threshold {trace,debug,info,warn,error}");

    opts::options_description fullDesc("All options");
    fullDesc.add(globalOptions);
    fullDesc.add_options()
            ("command", opts::value<std::string>(), "Algorithm to run")
            ("subargs", opts::value<std::vector<std::string> >(), "Algorithm specific arguments");

    opts::positional_options_description positional;
    positional.add("command", 1).add("subargs", -1);

    opts::variables_map vars;
    opts::parsed_options parsed = opts::command_line_parser(argc, argv).
            options(fullDesc).
            positional(positional).
            allow_unregistered().
            run();
    opts::store(parsed, vars);

    /* Set log level */
    std::string logLevel = "info";
    if (vars.count("log-level")) {
        logLevel = vars["log-level"].as<std::string>();
    }
    Logger::setThreshold(logLevel);

    /* Parse main command */
    std::unique_ptr<Algorithm> algorithm;
    try {
        algorithm = Metrics::getAlgorithm(vars["command"].as<std::string>());
    } catch(std::exception e) {
        LogLevel severity;
        int retval;
        if (vars.count("help") || argc == 1) {
            severity = INFO;
            retval = 0;
        } else {
            severity = ERROR;
            retval = 1;
            mainLogger(severity) << e.what();
        }

        mainLogger(severity) << std::endl
                << "Usage: " << std::endl
                << "  openvq [global options] command [command specific options]" << std::endl
                << std::endl
                << Metrics::availableCommands()
                << std::endl
                << globalOptions
                << std::endl
                << "To get additional help for the individual commands, run: " << std::endl
                << "  openvq <command> --help";
        exit(retval);
    }

    /* Initialize Libav */
    av_register_all();

    /* Run selected algorithm */
    try {
        algorithm->init(argc, argv);
    } catch (std::runtime_error e) {
        mainLogger(ERROR) << e.what();
        exit(1);
    }
    int ret = algorithm->run();

    /* Cleanup */
    exit(ret);
}
