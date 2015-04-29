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

#include "EdginessImage.h"

#include <io/FileWriter.h>

Logger EdginessImage::logger = Logger("EdginessImage");


std::shared_ptr<Frame> EdginessImage::createEdginessImage(std::shared_ptr<Frame> inputFrame) {
    double filter1D[5] = { 0.5, 0.5, 0, -0.5, -0.5 };
	double filter1DFlipped[5] = { -0.5, -0.5, 0, 0.5, 0.5 };

	cv::Mat Kv = cv::Mat(5, 1, CV_64FC1, &filter1D[0]);
	cv::Mat Kh = cv::Mat(1, 5, CV_64FC1, &filter1DFlipped[0]);

	std::shared_ptr<Frame> retFrame = std::make_shared<Frame>(inputFrame->Y.rows, inputFrame->Y.cols, CV_64FC1);

	RunFilter(inputFrame->Y, retFrame->Y, Kh, Kv);
	RunFilter(inputFrame->U, retFrame->U, Kh, Kv);
	RunFilter(inputFrame->V, retFrame->V, Kh, Kv);

	return retFrame;
}

void EdginessImage::RunFilter(cv::Mat& in, cv::Mat& out, cv::Mat Kh, cv::Mat Kv)
{
	cv::Mat temp_y_h(in.rows, in.cols, in.type());
	cv::Mat temp_y_v(in.rows, in.cols, in.type());
	cv::Mat sqrtRes(in.rows, in.cols, in.type());

	cv::filter2D(in, temp_y_h, CV_64F, Kh);
	cv::filter2D(in, temp_y_v, CV_64F, Kv);

	cv::pow(temp_y_h, 2.0, temp_y_h);
	cv::pow(temp_y_v, 2.0, temp_y_v);
	
	cv::Mat res(temp_y_h.rows, temp_y_h.cols, temp_y_h.type());
	cv::add(temp_y_h, temp_y_v, res);

	cv::sqrt(res, sqrtRes);
	cv::dilate(sqrtRes, out, cv::Mat());
}
