/*
 * PreProcess.cpp
 *
 *  Created on: May 18, 2017
 *      Author: Ouyancheng
 */

#include "PreProcess.hpp"

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

cv::Mat PreProcess::run() {
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

	return dstImg;
}


