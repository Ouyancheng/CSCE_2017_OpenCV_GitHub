/*
 * main.cpp
 *
 *  Created on: May 18, 2017
 *      Author: Ouyancheng
 */

#include "PreProcess.hpp"
using namespace cv;
int main(int argc, char *argv[]) {
	Mat srcImg = imread("/Users/Ouyancheng/CPPWORKSPACE/OpenCV/Learning/8.png");
	Mat dstImg;
	if (!srcImg.data) {
		std::fprintf(stderr, "\033[1;31mError\033[0m: Cannot open source image. \n");
		return 0;
	}

	PreProcess pp(srcImg);
	dstImg = pp.run();

	imshow("Source Image", srcImg);
	imshow("Destination Image", dstImg);

	while ((uchar)waitKey(1) != 'q');

	return 0;
}


