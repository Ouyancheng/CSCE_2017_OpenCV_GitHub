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
	Mat srcImg2 = imread("frame1.png");
	Mat dstImg;
	std::vector<cv::Mat> ROI;
	if (!srcImg.data) {
		std::fprintf(stderr, "\033[1;31mError\033[0m: Cannot open source image. \n");
		return 0;
	}

//	PreProcess pp(srcImg);
	std::shared_ptr<PreProcess> ppptr = PreProcess::getInstance(srcImg);
	ROI = ppptr->run();

	imshow("Source Image", srcImg);
	//imshow("Destination Image", dstImg);
	cv::waitKey(0);
	cv::destroyAllWindows();

	std::shared_ptr<PreProcess> aptr(ppptr);
	aptr->loadImage(srcImg2);
	aptr->run();

	for (auto &i : ROI) {
		if (i.data) imshow("ROI", i);
		else std::fprintf(stderr, "ROI empty!\n");
		cv::waitKey(0);
	}

	while ((uchar)waitKey(1) != 'q');

	return 0;
}


