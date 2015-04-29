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
#include <sstream>
#include <functional>
#include <mutex>

#ifndef __LOGGER_H
#define __LOGGER_H

enum LogLevel {
    TRACE = 0,
    DEBUG,
    INFO,
    WARN,
    ERROR
};

class Logger;

class LogStream : public std::ostringstream {
    Logger &logger;
    LogLevel logLevel;

public:
    LogStream(Logger &logger, LogLevel logLevel);

    LogStream(const LogStream &ls);

    ~LogStream();
};


class Logger {
    std::string className;
    static std::ostringstream queue;

    static LogLevel threshold;
    static std::mutex outmutex;
    static std::mutex queuemutex;
    static std::mutex progmutex;
    static bool inProgress;

public:
    Logger(std::string className);

    void log(LogLevel logLevel, std::string msg);

    LogStream operator()(LogLevel level);

    static void setThreshold(std::string &level);

    static bool initProgress();
    static bool resetProgress();
    static void logProgress(int progress, int length);

    static void flush();
};

#endif // __LOGGER_H