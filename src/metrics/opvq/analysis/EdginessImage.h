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

#ifndef EdginessImage_h
#define EdginessImage_h

#include <memory>
#include <io/Frame.h>
#include <io/Logger.h>


class EdginessImage
{
public:
    static std::shared_ptr<Frame> createEdginessImage(std::shared_ptr<Frame> inputFrame);

private:
    static Logger logger;

	static void RunFilter(cv::Mat& in, cv::Mat& out, cv::Mat Kh, cv::Mat Kv);
};

#endif //EdginessImage_h
