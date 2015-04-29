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

#include <algorithm>
#include <assert.h>

#include "FileWriter.h"

FileWriter::FileWriter(const char *fileToWrite, const char *mode)
        : writeFile(fileToWrite), originalMode(mode), logger("FileWriter") {
    neutral_buffer = NULL;
    file = fopen(fileToWrite, mode);
    isOpen = true;
}

FileWriter::~FileWriter() {
    if (isOpen) {
        fclose(file);
    }
    if (neutral_buffer != NULL)
        delete neutral_buffer;
}


void FileWriter::OpenFile() {
    if (!isOpen) {
        file = fopen(writeFile, "a");
        isOpen = true;
    }
}

void FileWriter::CloseFile() {
    if (isOpen) {
        fclose(file);
    }
}

void FileWriter::lazy_init_neutral_buffer(std::size_t size) {
    if (neutral_buffer != NULL) {
        if (sizeof(neutral_buffer) < size)
            delete neutral_buffer;
        else
            return;
    }
    neutral_buffer = new std::uint8_t[size];
    std::fill_n(neutral_buffer, size, 128);
}

void FileWriter::writeFrame(cv::Mat *Y, cv::Mat *U, cv::Mat *V) {
    assert(Y);

    std::uint8_t *y, *u, *v;
    cv::Mat ycpy;
    Y->copyTo(ycpy);
    y = ycpy.data;

    cv::Mat vcpy;
    cv::Mat ucpy;

    if(!U){
        lazy_init_neutral_buffer(Y->cols*Y->rows*Y->elemSize1());
        u = neutral_buffer;
    }else{
        U->copyTo(ucpy);
        u = ucpy.data;
    }

    if(!V){
        lazy_init_neutral_buffer(Y->cols*Y->rows*Y->elemSize1());
        v = neutral_buffer;
    }else{
        V->copyTo(vcpy);
        v = vcpy.data;
    }

    fwrite(y, 1, Y->cols*Y->rows, file);
    fwrite(u, 1, U->cols*U->rows, file);
    fwrite(v, 1, V->cols*V->rows, file);
}
