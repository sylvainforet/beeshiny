#include <iostream>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/nonfree/nonfree.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/core/core.hpp>

using namespace cv;
using namespace std;

int main(){

	Mat input_img = imread("bee.jpg", CV_LOAD_IMAGE_COLOR);
	Mat output;


	//equalizeHist(input_img, input_img);
	
	//medianBlur(input_img,input_img,5);
	vector<Mat> rgb;
	//Scalar color = Scalar(0, 255, 0);
	split(input_img, rgb);
	//rgb[0]=Mat::zeros(input_img.rows, input_img.cols, CV_8UC1);
	//rgb[2]=Mat::zeros(input_img.rows, input_img.cols, CV_8UC1);
	rgb[0].setTo(Scalar(150));
	//rgb[1].setTo(Scalar(255));
	rgb[2].setTo(Scalar(150));
	merge(rgb,input_img);
	//imwrite("test2.jpg",rgb[2]);
	imwrite("test.jpg",input_img);
	


	return 0;
}
