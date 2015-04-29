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

#include "Logger.h"
#include <cmath>


LogStream::LogStream(Logger &logger, LogLevel logLevel) : logger(logger), logLevel(logLevel) {
}

LogStream::LogStream(const LogStream &ls) : logger(ls.logger), logLevel(ls.logLevel) {
}

LogStream::~LogStream() {
    logger.log(logLevel, str());
}


Logger::Logger(std::string className) : className(className) {
}

LogStream Logger::operator()(LogLevel level) {
    return LogStream(*this, level);
}

LogLevel Logger::threshold = TRACE;
std::mutex Logger::outmutex;
std::mutex Logger::queuemutex;
std::mutex Logger::progmutex;
std::ostringstream Logger::queue;
bool Logger::inProgress = false;

void Logger::setThreshold(std::string &level) {
    if (level == "trace")
        Logger::threshold = TRACE;
    else if (level == "debug")
        Logger::threshold = DEBUG;
    else if (level == "info")
        Logger::threshold = INFO;
    else if (level == "warn")
        Logger::threshold = WARN;
    else if (level == "error")
        Logger::threshold = ERROR;
    else
        throw std::runtime_error("Invalid log level \"" + level + "\"");
}

void Logger::log(LogLevel logLevel, std::string msg) {
    if (logLevel < threshold)
        return;

    const static char *levelToString[] = {"TRACE", "DEBUG", "INFO", "WARN", "ERROR"};
    time_t now = time(NULL);
    char timestamp[20];
    strftime(timestamp, sizeof(timestamp), "%X", localtime(&now));

    {
        std::lock_guard<std::mutex> g(queuemutex);
        queue << timestamp << " ";
        queue << "[" << levelToString[logLevel] << "] ";
#ifdef OPENVQ_DEBUG
        queue << className << ": ";
#endif
        queue << msg << std::endl;
    }
    flush();
}

bool Logger::initProgress() {
    std::lock_guard<std::mutex> q(progmutex);
    if (inProgress) {
        return false;
    }
    return (inProgress = true);
}

bool Logger::resetProgress() {
    {
        std::lock_guard<std::mutex> g(progmutex);
        if (!inProgress) {
            return false;
        }
        inProgress = false;
    }
    {
        std::lock_guard<std::mutex> g(outmutex);
        std::cout << std::endl;
    }
    flush();
    return true;
}

/*progress is the current progress while length is the total final length*/
void Logger::logProgress(int progress, int length)
{
    std::lock_guard<std::mutex> pg(progmutex);
    if (!inProgress) {
        return;
    }
    std::lock_guard<std::mutex> og(outmutex);
    int percent = static_cast<int>(std::round((double)progress * 100.0/ (double)length));
    std::cout << "\r";

    static const int totalBarLength = 72;
    int barLength = (progress * totalBarLength) / length;

    std::cout << "[";
    for (int i = 0; i < barLength; i++)
        std::cout << "=";
    std::cout << ">";
    for (int i = barLength; i < totalBarLength-1; i++){
        std::cout << " ";
    }
    std::cout << "] " << percent << "%";
}

void Logger::flush() {
    {
        std::lock_guard<std::mutex> g(progmutex);
        if (inProgress) {
            return;
        }
    }
    std::string q;
    {
        std::lock_guard<std::mutex> g(queuemutex);
        q = queue.str();
        queue.clear();
        queue.str("");
    }
    {
        std::lock_guard<std::mutex> g(outmutex);
        std::cout << q;
    }
}
