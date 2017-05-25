/*
 * PreProcess.hpp
 *
 *  Created on: May 18, 2017
 *      Author: Ouyancheng
 */

#ifndef PREPROCESS_HPP_
#define PREPROCESS_HPP_
#include "includes.hpp"
class PreProcess {
public:
	enum SPILTTED_CHANNELS {
		BLUE = 0,
		GREEN = 1,
		RED = 2
	};

	enum BLUR_TYPE {
		MEAN_BLUR = 0,
		GAUSSIAN_BLUR = 1,
		MEDIAN_BLUR = 2,
		BILATERAL_BLUR = 3
	};

public:
	PreProcess();
	virtual ~PreProcess();

	//Opens an image.
	PreProcess(const cv::Mat &_srcImg);

	//Threshold or use adaptive threshold.
	void threshold(cv::Mat &_srcImg, cv::Mat &_dstImg, bool adaptive = false);

	//Blur the image. Choose blurType from one of the BLUR_TYPE.
	void blur(cv::Mat &_srcImg, cv::Mat &_dstImg, int blurType = BLUR_TYPE::MEDIAN_BLUR);

	//Returns the preprocessed images.
	std::vector<cv::Mat> run();

public:
	int meanBlurKernelSize;
	int GaussianBlurKernelSize;
	int medianBlurKernelSize;
	int bilateralBlurValue;
	int adaptiveThresholdBlockSize;

	cv::Mat srcImg;

};

#endif /* PREPROCESS_HPP_ */
