/*
 * main.cpp
 *
 *  Created on: May 18, 2017
 *      Author: Ouyancheng
 */

#include "PreProcess.hpp"
using namespace cv;
int main(int argc, char *argv[]) {
	Mat srcImg = imread("8.png");
	Mat dstImg;
	std::vector<cv::Mat> ROI;
	if (!srcImg.data) {
		std::fprintf(stderr, "\033[1;31mError\033[0m: Cannot open source image. \n");
		return 0;
	}

	PreProcess pp(srcImg);
	ROI = pp.run();

	imshow("Source Image", srcImg);
	//imshow("Destination Image", dstImg);
	cv::waitKey(0);
	cv::destroyAllWindows();

	for (auto &i : ROI) {
		if (i.data) imshow("ROI", i);
		else std::fprintf(stderr, "ROI empty!\n");
		cv::waitKey(0);
	}

	while ((uchar)waitKey(1) != 'q');

	return 0;
}


