#pragma once
#include <iostream>
#include <string>

// include all the necessary files from OpenCV
#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/video.hpp>

using namespace cv;
using namespace std;

// Write a utility function which checks if a string is a number
bool isNumber(string s) {
	for (int i = 0; i < s.length(); i++)
		if (isdigit(s[i]) == false)
			// If any item in the string is not a number, return false
			return false;
	return true;
}

// Write another utility function to handle VideoCapture. VideoCapture can take either a string or int,
// so this will make it easier to deal with attributes
VideoCapture capture(string s) {
	if (isNumber(s)) {
		VideoCapture cap(stoi(s));
		return cap;
	}
	else {
		VideoCapture cap(s);
		return cap;
	}
}


// Declare base class for just a video. It's kept very basic here as further additions may be required
class Video {
protected:
	// Must be initialized with a file name. Use a number in quotes to use a video stream, ie, "0"
	string filename;

public:
	explicit Video(string file) {
		filename = file;
	}
};

class BackgroundSubtraction : public Video {
public:

	// Use the same constuctor as the video class
	using Video::Video;

	// Declare function for CV's built-in background removal
	int removeBackgroundCV(string method);

	// Declare function for custom background removal
	int removeBackground(int canny_high, int canny_low, float max_area, float min_area, int dilte_iter = 10, int erode_iter = 10, int blur = 3);
};

int BackgroundSubtraction::removeBackgroundCV(string method) {
	
	// Create pointer for background subtractor, as the method isn't known, yet
	Ptr<BackgroundSubtractor> subtraction_method;

	// Test to make sure one the method keywords were used
	if (method == "MOG2") {
		subtraction_method = cv::createBackgroundSubtractorMOG2();
	}
	else if (method == "KNN") {
		subtraction_method = cv::createBackgroundSubtractorKNN();
	}
	else {
		std::cerr << "The method must be 'MOG2' or 'KNN'. Any other method is not considered valid" << endl;
		return -1;
	}

	// Capture the video stream
	VideoCapture cap = capture(filename);

	while (true) {

		cv::Mat frame, mask, output;

		// Capture the current frame of the video
		cap >> frame;

		// Check the frame is not empty
		if (frame.empty()) {
			break;
		}

		// Apply subtraction method
		subtraction_method->apply(frame, mask);
		
		// Coppy the frame and apply the mask
		frame.copyTo(output, mask);

		cv::imshow("CV Background Subtraction", output);
		int keyboard = cv::waitKey(30);
		if (keyboard == 'q' || keyboard == 27)
			break;
	}

	return 0;
}


int BackgroundSubtraction::removeBackground(int canny_high, int canny_low, float max_area, float min_area, int dilte_iter, int erode_iter, int blur) {

	// Capture the video stream
	VideoCapture cap = capture(filename);

	while (true) {

		cv::Mat frame, frame_gray, edges, mask, masked;

		// Capture the current frame of the video
		cap >> frame;

		// Check the frame is not empty
		if (frame.empty()) {
			break;
		}

		// Get the size of the frame
		cv::Size size = frame.size();
		int width = size.width;
		int height = size.height;
		int area = width * height;

		// Now the min and max areas can be calculated for the image
		float max_area_px = max_area * area;
		float min_area_px = min_area * area;

		// Convert frame to grayscale
		cv::cvtColor(frame, frame_gray, cv::COLOR_BGR2GRAY);

		// Apply canny edge detection
		cv::Canny(frame, edges, canny_low, canny_high);

		// Apply dilate/erode 
		cv::dilate(edges, edges, Mat());
		cv::erode(edges, edges, Mat());

		// find the contours
		std::vector<std::vector<cv::Point> > contours;
		vector<Vec4i> hierarchy;
		cv::findContours(edges, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);

		// Create a mask
		mask = cv::Mat(frame.size().height, frame.size().width, CV_64F, float(0));

		// iterate through the contours and find the ones with the applicable areas
		for (int i = 0; i < contours.size(); i++) {
			// Find the area of the contour
			float contour_area = cv::contourArea(contours[i]);

			// If the contour is above the min, but below the max, add it to the mask
			if (contour_area > min_area_px && contour_area < max_area_px) {
				cv::fillConvexPoly(mask, contours[i], cv::Scalar(255, 255, 255));
			}
		}

		// Apply dilate/erode to the mask
		cv::dilate(mask, mask, Mat());
		cv::erode(mask, mask, Mat());
		cv::GaussianBlur(mask, mask, cv::Size(blur, blur), 0);

		// Convert the mask and frame matrices into the proper data type
		mask.convertTo(mask, CV_8U);

		// Apply the mask to the farame
		cv::bitwise_and(frame, frame, masked, mask = mask);

		// Display the results
		cv::imshow("Background Subtraction", masked);

		// If "Q" is pressed on the keyboard, quit the program
		int keyboard = cv::waitKey(30);
		if (keyboard == 'q' || keyboard == 27)
			break;
	}

	return 0;
}