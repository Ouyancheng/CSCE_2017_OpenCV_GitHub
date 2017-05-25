/*
 * PreProcess.cpp
 *
 *  Created on: May 18, 2017
 *      Author: Ouyancheng
 */

#include "PreProcess.hpp"
using namespace cv;
PreProcess::PreProcess() :
	meanBlurKernelSize(3),
	GaussianBlurKernelSize(3),
	medianBlurKernelSize(3),
	bilateralBlurValue(3),
	adaptiveThresholdBlockSize(3),
	srcImg() {
	// TODO Auto-generated constructor stub
}

PreProcess::~PreProcess() {
	// TODO Auto-generated destructor stub
}

PreProcess::PreProcess(const cv::Mat &_srcImg) :
	meanBlurKernelSize(3),
	GaussianBlurKernelSize(3),
	medianBlurKernelSize(3),
	bilateralBlurValue(3),
	adaptiveThresholdBlockSize(3) {

	if (!_srcImg.data) {
		std::fprintf(stderr, "\033[1;31mError\033[0m: Source image is empty. \n");
		srcImg = cv::Mat();
	} else {
		srcImg = _srcImg;
	}

}

void PreProcess::threshold(cv::Mat &_srcImg, cv::Mat &_dstImg, bool adaptive) {
	(adaptive == true) ?
			(cv::adaptiveThreshold(_srcImg, _dstImg, 255,
					CV_ADAPTIVE_THRESH_MEAN_C,
					CV_THRESH_BINARY,
					adaptiveThresholdBlockSize * 2 + 3, 3)) :
			((void)cv::threshold(_srcImg, _dstImg, 150, 255, CV_THRESH_BINARY));
	return;
}

void PreProcess::blur(cv::Mat &_srcImg, cv::Mat &_dstImg, int blurType) {
	switch (blurType) {
	case BLUR_TYPE::MEAN_BLUR:
		cv::blur(_srcImg, _dstImg, cv::Size(meanBlurKernelSize, meanBlurKernelSize));
		break;
	case BLUR_TYPE::GAUSSIAN_BLUR:
		cv::GaussianBlur(_srcImg, _dstImg,
				cv::Size(GaussianBlurKernelSize, GaussianBlurKernelSize),
				0, 0);
		break;
	case BLUR_TYPE::MEDIAN_BLUR:
		cv::medianBlur(_srcImg, _dstImg, medianBlurKernelSize);
		break;
	case BLUR_TYPE::BILATERAL_BLUR:
		cv::bilateralFilter(_srcImg, _dstImg, bilateralBlurValue, bilateralBlurValue * 2, bilateralBlurValue / 2);
		break;
	default:
		std::fprintf(stderr, "\033[1;31mError\033[0m: Unknown blur type. \n");
		break;
	}

	return;
}

std::vector<cv::Mat> PreProcess::run() {
	if (!srcImg.data) {
		std::fprintf(stderr, "\033[1;31mError\033[0m: Source image is empty. \n");
		return cv::Mat();
	}

	cv::Mat dstImg = srcImg.clone();

	std::vector<cv::Mat> channels;
	cv::split(srcImg, channels);
	srcImg = channels[2];

	this->threshold(srcImg, srcImg);
	this->blur(srcImg, srcImg, BLUR_TYPE::MEDIAN_BLUR);

	std::vector<std::vector<cv::Point>> contours;
	std::vector<cv::Vec4i> hierarchy;

	cv::findContours(srcImg, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_NONE);
	for (int i = 0; i < contours.size(); i++) {
		cv::drawContours(dstImg, contours, i, CV_RGB(0, 0, 255), 3, CV_FILLED, hierarchy, INT_MAX);
	}

	const float ratio = 28.0 / 16.0;
	std::vector<cv::RotatedRect> sudoku;
	const int sudokuWidth = 127;
	const int sudokuHeight = 71;

	std::vector<cv::Mat> ROI;

	for (auto &i : contours) {
		cv::RotatedRect tempRect = cv::minAreaRect(i);
		tempRect = (tempRect.size.width > tempRect.size.height) ?
				(tempRect) :
				(cv::RotatedRect(tempRect.center, cv::Size2f(tempRect.size.height, tempRect.size.width), tempRect.angle + 90.0));
		const cv::Size2f &s= tempRect.size;
		float ratio_cur = s.width / s.height;

		if (ratio_cur > 0.8 * ratio && ratio_cur < 1.2 * ratio &&
			s.width > 0.6 * sudokuWidth && s.width < 1.4 * sudokuWidth &&
			s.height > 0.6 * sudokuHeight && s.height < 1.4 * sudokuHeight &&
			((tempRect.angle > -10 && tempRect.angle < 10) || tempRect.angle < -170 || tempRect.angle > 170)){

			sudoku.push_back(tempRect);

			cv::Point2f vertices[4];
			tempRect.points(vertices);
			for (int i=0; i<4; i++) {
				cv::line(dstImg, vertices[i], vertices[(i+1) % 4], cv::Scalar(255, 0, 255), 2);
			}
			ROI.push_back(srcImg(cv::Rect(vertices[1].x, vertices[1].y, abs(vertices[2].x - vertices[0].x), abs(vertices[2].y - vertices[0].y))));
		}
	}

	if (ROI.size() < 9) {
		std::fprintf(stderr, "\033[1;32mWarning\033[0m: Size of ROI vector less than 9. \n");
	} else if (ROI.size() > 9) {
		std::fprintf(stderr, "\033[1;32mWarning\033[0m: Size of ROI vector greater than 9. \n");
	}

	//ROI.push_back(dstImg);
	cv::imshow("Destination Image", dstImg);

	return ROI;
}


