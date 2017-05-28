/*
 * PreProcess.cpp
 *
 *  Created on: May 18, 2017
 *      Author: Ouyancheng
 */

#include "PreProcess.hpp"
using namespace cv;
PreProcess::PreProcess() :
	srcImg() {
	// TODO Auto-generated constructor stub
	std::printf("Instance Constructed!~ \n");
}

PreProcess::~PreProcess() {
	// TODO Auto-generated destructor stub
	std::printf("Instance Destructed!~ \n");
}

PreProcess::PreProcess(const cv::Mat &_srcImg) {
	if (!_srcImg.data) {
		std::fprintf(stderr, "\033[1;31mError\033[0m: Source image is empty. \n");
		srcImg = cv::Mat();
	} else {
		srcImg = _srcImg;
	}
	std::printf("Instance Constructed!~ \n");
}
std::shared_ptr<PreProcess> PreProcess::instance = nullptr;
std::shared_ptr<PreProcess> PreProcess::getInstance(const cv::Mat &_srcImg) {
	if (instance != nullptr) {
		instance->loadImage(_srcImg);
		return instance;
	}
	else return std::unique_ptr<PreProcess>(new PreProcess(_srcImg));
}

bool PreProcess::loadImage(const cv::Mat &_srcImg) {
	if (_srcImg.data) {
		srcImg = _srcImg;
		return true;
	} else {
		return false;
	}
}

void PreProcess::threshold(cv::Mat &_srcImg, cv::Mat &_dstImg, bool adaptive) {
	(adaptive == true) ?
			(cv::adaptiveThreshold(_srcImg, _dstImg, 255,
					CV_ADAPTIVE_THRESH_MEAN_C,
					CV_THRESH_BINARY,
					6 * 2 + 3, 3)) :
			((void)cv::threshold(_srcImg, _dstImg, 150, 255, CV_THRESH_BINARY));
	return;
}

void PreProcess::blur(cv::Mat &_srcImg, cv::Mat &_dstImg, int blurType, int value) {
	switch (blurType) {
	case BLUR_TYPE::MEAN_BLUR:
		cv::blur(_srcImg, _dstImg, cv::Size(value, value));
		break;
	case BLUR_TYPE::GAUSSIAN_BLUR:
		cv::GaussianBlur(_srcImg, _dstImg,
				cv::Size(value, value),
				0, 0);
		break;
	case BLUR_TYPE::MEDIAN_BLUR:
		cv::medianBlur(_srcImg, _dstImg, value);
		break;
	case BLUR_TYPE::BILATERAL_BLUR:
		cv::bilateralFilter(_srcImg, _dstImg, value, value * 2, value / 2);
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

//	std::vector<cv::Mat> channels;
//	cv::split(srcImg, channels);
//	srcImg = channels[2];

	cv::cvtColor(srcImg, srcImg, ::CV_BGR2GRAY);

	this->threshold(srcImg, srcImg, true);
	this->blur(srcImg, srcImg, BLUR_TYPE::MEDIAN_BLUR);

	cv::imshow("srcImg2", srcImg);

	std::vector<std::vector<cv::Point>> contours;
	std::vector<cv::Vec4i> hierarchy;

	cv::findContours(srcImg, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_NONE);
	for (int i = 0; i < contours.size(); i++) {
		cv::drawContours(dstImg, contours, i, CV_RGB(0, 0, 255), 3, CV_FILLED, hierarchy, INT_MAX);
	}

	const float ratio = 28.0 / 16.0;
	const int sudokuWidth = 127;
	const int sudokuHeight = 71;
	const int angleTolerance = 6;
	const float ratioToleranceRate = 0.2;
	const float dimensionsToleranceRate = 0.4;

	std::vector<cv::Mat> ROI;
	std::vector<cv::RotatedRect> rects;

	for (auto &i : contours) {
		cv::RotatedRect tempRect = cv::minAreaRect(i);
		tempRect = (tempRect.size.width > tempRect.size.height) ?
				(tempRect) :
				(cv::RotatedRect(tempRect.center, cv::Size2f(tempRect.size.height, tempRect.size.width), tempRect.angle + 90.0));
		const cv::Size2f &s= tempRect.size;
		float ratio_cur = s.width / s.height;

		if (ratio_cur > (1.0-ratioToleranceRate) * ratio && ratio_cur < (1.0+ratioToleranceRate) * ratio &&
			s.width > (1.0-dimensionsToleranceRate) * sudokuWidth && s.width < (1.0+dimensionsToleranceRate) * sudokuWidth &&
			s.height > (1.0-dimensionsToleranceRate) * sudokuHeight && s.height < (1.0+dimensionsToleranceRate) * sudokuHeight &&
			((tempRect.angle > -angleTolerance && tempRect.angle < angleTolerance) || tempRect.angle < (-180+angleTolerance) || tempRect.angle > (180-angleTolerance))) {
			rects.push_back(tempRect);
			cv::line(dstImg, tempRect.center, tempRect.center, Scalar(0, 0, 255), 6);
//			printf("(%f, %f)\n", tempRect.center.x, tempRect.center.y);
			cv::Point2f vertices[4];
			tempRect.points(vertices);
			for (int i=0; i<4; i++) {
				cv::line(dstImg, vertices[i], vertices[(i+1) % 4], cv::Scalar(0, 255, 255), 2);
			}
//			ROI.push_back(srcImg(cv::Rect(vertices[1].x, vertices[1].y, abs(vertices[2].x - vertices[0].x), abs(vertices[2].y - vertices[0].y))));
		}
	}

	std::printf("Size of rects: %lu\n", rects.size());

	if (rects.size() < 9) {
		std::fprintf(stderr, "\033[1;32mWarning\033[0m: Size of rects vector less than 9. \n");
	} else if (rects.size() > 9) {
		std::fprintf(stderr, "\033[1;32mWarning\033[0m: Size of rects vector greater than 9. \n");

		double minDist = DBL_MAX;
		int centerIndex;
		std::vector<DistanceStruct> distanceVector(rects.size(), DistanceStruct(rects.size()));

		//Calculate the distances and choose a rectangle with smallest distances sum as the center.
		for (int i=0; i<rects.size(); i++) {
			for (int j=0; j<rects.size(); j++) {
				distanceVector[i].distances.at(j).dist = std::sqrt((rects[i].center.x-rects[j].center.x)*(rects[i].center.x-rects[j].center.x) + (rects[i].center.y-rects[j].center.y)*(rects[i].center.y-rects[j].center.y));
				distanceVector[i].distances.at(j).index = j;
				distanceVector[i].totalDist += distanceVector[i].distances.at(j).dist;
			}
			if (distanceVector[i].totalDist < minDist) {
				minDist = distanceVector[i].totalDist;
				centerIndex = i;
			}
		}

		//Search 9 nearest rectangles.
		std::vector<DistanceWithIndex> &selectedDistances = distanceVector.at(centerIndex).distances;
		for (int i=0; i<selectedDistances.size(); i++) {
			selectedDistances[i].dist += 0.1 * abs(rects[selectedDistances[i].index].boundingRect().height * rects[selectedDistances[i].index].boundingRect().width - sudokuWidth * sudokuHeight);
		}
		std::sort(selectedDistances.begin(), selectedDistances.end(), [](DistanceWithIndex &a, DistanceWithIndex &b){return (a.dist < b.dist);});

		/*
		 *	1 2 3
		 *	4 5 6
		 *	7 8 9
		 *
		 * First to select 2,4,5,6,8 because they have the nearest distances theoretically... But the picture should be upright...
		 *
		 * TODO We can solve the rotated picture using the rotated angle of the center rectangle!~
		 */
//		for (int i=0; i<5; i++) {
//			cv::Point2f vertices[4];
//			rects[selectedDistances.at(i).index].points(vertices);
//			for (int i=0; i<4; i++) {
//				cv::line(dstImg, vertices[i], vertices[(i+1) % 4], cv::Scalar(255, 0, 255), 2);
//			}
//			ROI.push_back(srcImg(cv::Rect(vertices[1].x, vertices[1].y, abs(vertices[2].x - vertices[0].x), abs(vertices[2].y - vertices[0].y))));
//		}
		const float distEPS = 10;
		bool upleft = false, upright = false, downleft = false, downright = false, mid = false;
		//int x_axis = 0, y_axis = 0;
		bool up = false, down = false, left = false, right = false;
		float &centerX = rects[centerIndex].center.x;
		float &centerY = rects[centerIndex].center.y;
		for (int i=0; i<rects.size(); i++) {
			float &targetX = rects[selectedDistances.at(i).index].center.x;
			float &targetY = rects[selectedDistances.at(i).index].center.y;
			//Edges
			if (targetX - centerX < distEPS && targetX - centerX > -distEPS && targetY - centerY < distEPS && targetY - centerY > -distEPS) {
				if (mid) continue; else mid = true;
			} else if (targetX - centerX < distEPS && targetX - centerX > -distEPS && targetY > centerY) {
				if (down) continue; else down = true;
			} else if (targetX - centerX < distEPS && targetX - centerX > -distEPS && targetY < centerY) {
				if (up) continue; else up = true;
			} else if (targetY - centerY < distEPS && targetY - centerY > -distEPS && targetX > centerX) {
				if (right) continue; else right = true;
			} else if (targetY - centerY < distEPS && targetY - centerY > -distEPS && targetX < centerX) {
				if (left) continue; else left = true;
			}
			//Corners
			else if (targetX < centerX && targetY < centerY) {
				if (upleft) continue; else upleft = true;
			} else if (targetX < centerX && targetY > centerY) {
				if (downleft) continue; else downleft = true;
			} else if (targetX > centerX && targetY < centerY) {
				if (upright) continue; else upright = true;
			} else if (targetX > centerX && targetY > centerY) {
				if (downright) continue; else downright = true;
			}
			cv::Point2f vertices[4];
			rects[selectedDistances.at(i).index].points(vertices);
			for (int i=0; i<4; i++) {
				cv::line(dstImg, vertices[i], vertices[(i+1) % 4], cv::Scalar(255, 0, 255), 2);
			}
			ROI.push_back(srcImg(cv::Rect(vertices[1].x, vertices[1].y, abs(vertices[2].x - vertices[0].x), abs(vertices[2].y - vertices[0].y))));
		}

		std::printf("downLeft:%d downright:%d downRight:%d downleft:%d up:%d down:%d left:%d right:%d\n", upright, downright, upleft, downleft, up, down, left, right);
		//TODO This is not a good algorithm for searching because if you set angleTolerance to 8, it will choose the number (3) twice and will not choose the number (9).
		//DONE...
	} else {

		for (int i=0; i<rects.size(); i++) {
			cv::Point2f vertices[4];
			rects[i].points(vertices);
			for (int i=0; i<4; i++) {
				cv::line(dstImg, vertices[i], vertices[(i+1) % 4], cv::Scalar(255, 0, 255), 2);
			}
			ROI.push_back(srcImg(cv::Rect(vertices[1].x, vertices[1].y, abs(vertices[2].x - vertices[0].x), abs(vertices[2].y - vertices[0].y))));
		}

	}

	std::printf("Size of ROI: %lu\n", ROI.size());


	//ROI.push_back(dstImg);
	cv::imshow("Destination Image", dstImg);

	return ROI;
}


