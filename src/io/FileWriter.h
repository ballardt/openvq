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

#ifndef FileWriter_h__
#define FileWriter_h__

#include <fstream>
#include <iostream>
#include <sstream>
#include <cstdint>
#include <cstddef>

#include <io/Frame.h>
#include <io/VideoProperties.h>
#include "Logger.h"

struct Frame;
class FileWriter {
public:
    FileWriter(const char *fileToWrite, const char *mode = "wb");

    ~FileWriter();

    void writeFrame(cv::Mat* Y, cv::Mat* U, cv::Mat* V);

    void OpenFile();

    void CloseFile();
    
protected:
    FILE *file;
    const char *writeFile;
    const char *originalMode;
    bool isOpen;
    std::uint8_t * neutral_buffer;
    Logger logger;
    void lazy_init_neutral_buffer(std::size_t size);
};

#endif // FileWriter_h__
