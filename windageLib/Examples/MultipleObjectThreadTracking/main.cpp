/* ========================================================================
 * PROJECT: windage Library
 * ========================================================================
 * This work is based on the original windage Library developed by
 *   Woonhyuk Baek (wbaek@gist.ac.kr / windage@live.com)
 *   Woontack Woo (wwoo@gist.ac.kr)
 *   U-VR Lab, GIST of Gwangju in Korea.
 *   http://windage.googlecode.com/
 *   http://uvr.gist.ac.kr/
 *
 * Copyright of the derived and new portions of this work
 *     (C) 2009 GIST U-VR Lab.
 *
 * This framework is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This framework is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this framework; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * For further information please contact 
 *   Woonhyuk Baek
 *   <windage@live.com>
 *   GIST U-VR Lab.
 *   Department of Information and Communication
 *   Gwangju Institute of Science and Technology
 *   1, Oryong-dong, Buk-gu, Gwangju
 *   South Korea
 * ========================================================================
 ** @author   Woonhyuk Baek
 * ======================================================================== */

#include <iostream>

#include <cv.h>
#include <highgui.h>

#include <windage.h>
#include "../Common/FleaCamera.h"

const int WIDTH = 640;
const int HEIGHT = (WIDTH * 3) / 4;
const double INTRINSIC[] = {1033.93, 1033.84, 319.044, 228.858,-0.206477, 0.306424, 0.000728208, 0.0011338};
const double REPROJECTION_ERROR = 2.0;

const int OBJECT_COUNT = 6;
const char* FEATURE_NAME = {"data/PlanarImage/descriptor%03d.txt"};

void main()
{
	windage::Logger logger(&std::cout);

	IplImage* inputImage;
	IplImage* resizeImage = cvCreateImage(cvSize(WIDTH, HEIGHT), IPL_DEPTH_8U, 3);
	IplImage* grayImage = cvCreateImage(cvSize(WIDTH, HEIGHT), IPL_DEPTH_8U, 1);
	IplImage* resultImage = cvCreateImage(cvSize(WIDTH, HEIGHT), IPL_DEPTH_8U, 3);

	FleaCamera* capture = new FleaCamera();
	capture->open();
	capture->start();
	//CvCapture* capture = cvCaptureFromCAM(CV_CAP_ANY);
	cvNamedWindow("result");

	// create and initialize tracker
	double threshold = 50.0;

	windage::Frameworks::MultiplePlanarObjectThreadTracking tracking;
	windage::Calibration* calibration;
	windage::Algorithms::FeatureDetector* detector;
	windage::Algorithms::OpticalFlow* opticalflow;
	windage::Algorithms::HomographyEstimator* estimator;
	windage::Algorithms::OutlierChecker* checker;
	windage::Algorithms::HomographyRefiner* refiner;

	calibration = new windage::Calibration();
//	detector = new windage::Algorithms::SIFTGPUdetector();
	opticalflow = new windage::Algorithms::OpticalFlow();
	estimator = new windage::Algorithms::RANSACestimator();
	checker = new windage::Algorithms::OutlierChecker();
	refiner = new windage::Algorithms::LMmethod();

	calibration->Initialize(INTRINSIC[0], INTRINSIC[1], INTRINSIC[2], INTRINSIC[3], INTRINSIC[4], INTRINSIC[5], INTRINSIC[6], INTRINSIC[7]);
	opticalflow->Initialize(WIDTH, HEIGHT, cvSize(15, 15), 3);
	estimator->SetReprojectionError(REPROJECTION_ERROR);
	checker->SetReprojectionError(REPROJECTION_ERROR * 2);
	refiner->SetMaxIteration(10);

	tracking.AttatchCalibration(calibration);
//	tracking.AttatchDetetor(detector);
	tracking.AttatchTracker(opticalflow);
	tracking.AttatchEstimator(estimator);
	tracking.AttatchChecker(checker);
	tracking.AttatchRefiner(refiner);
	
	tracking.Initialize(WIDTH, HEIGHT, (double)WIDTH, (double)HEIGHT);
	tracking.SetFilter(false);
	tracking.SetDitectionRatio(5);

	bool trained = false;
	std::vector<windage::FeaturePoint> baseFeaturePoints;
	
	windage::FeatureLoader* baseLoader = new windage::FeatureLoader();
	baseLoader->AttatchFeaturePoints(&baseFeaturePoints);

	for(int i=0; i<OBJECT_COUNT; i++)
	{
		char filename[100];
		sprintf(filename, FEATURE_NAME, i);

		baseFeaturePoints.clear();
		baseLoader->DoLoad(filename);
		tracking.TrainingReference(&baseFeaturePoints);
	}

	trained = true;
	
	int keypointCount = 0;
	int matchingCount = 0;
	double processingTime = 0.0;

	char message[100];
	bool fliping = true;
	bool processing = true;
	while(processing)
	{
		// capture image
		capture->update();
		inputImage = capture->GetIPLImage();
		cvCvtColor(inputImage, resizeImage, CV_BGRA2BGR);
/*
		inputImage = cvRetrieveFrame(capture);
		cvResize(inputImage, resizeImage);
		if(fliping)
			cvFlip(resizeImage, resizeImage);
*/

		cvCvtColor(resizeImage, grayImage, CV_BGR2GRAY);
		cvCopyImage(resizeImage, resultImage);

		logger.updateTickCount();

		// track object
		if(trained)
		{
			tracking.UpdateCamerapose(grayImage);
			matchingCount = 0;

			// draw result
			for(int i=0; i<tracking.GetObjectCount(); i++)
			{
				matchingCount += tracking.GetMatchingCount(i);
				int matchedCount = tracking.GetMatchingCount(i);
				if(matchedCount > 9)
				{
					tracking.DrawDebugInfo(resultImage, i);
					tracking.DrawOutLine(resultImage, i, true);

					windage::Calibration* calibrationTemp = tracking.GetCameraParameter(i);
					calibrationTemp->DrawInfomation(resultImage, 100);
					CvPoint centerPoint = calibrationTemp->ConvertWorld2Image(0.0, 0.0, 0.0);
					centerPoint.x += 5;
					centerPoint.y += 10;
					sprintf_s(message, "object #%d (%03d)", i+1, matchedCount);

					windage::Utils::DrawTextToImage(resultImage, centerPoint, 0.6, message);
				}
			}
		}

		processingTime = logger.calculateProcessTime();
		logger.log("processingTime", processingTime);
		logger.logNewLine();

		sprintf_s(message, "Processing Time : %.2lf ms", processingTime);
		windage::Utils::DrawTextToImage(resultImage, cvPoint(10, 20), 0.6, message);
		sprintf_s(message, "Feature Count : %d, Threshold : %.0lf", keypointCount, threshold);
		windage::Utils::DrawTextToImage(resultImage, cvPoint(10, 40), 0.6, message);
		sprintf_s(message, "Matching Count : %d", matchingCount);
		windage::Utils::DrawTextToImage(resultImage, cvPoint(10, 60), 0.6, message);

		sprintf_s(message, "Press 'Space' to add tracking the current image", keypointCount, threshold);
		windage::Utils::DrawTextToImage(resultImage, cvPoint(WIDTH-315, HEIGHT-10), 0.5, message);
		cvShowImage("result", resultImage);

		char ch = cvWaitKey(1);
		switch(ch)
		{
		case 'q':
		case 'Q':
			processing = false;
			break;
		case 'f':
		case 'F':
			fliping = !fliping;
			break;
		case ' ':
		case 's':
		case 'S':
			break;
		}		
	}

//	cvReleaseCapture(&capture);
	capture->stop();
	capture->close();
	
	cvDestroyAllWindows();
}
