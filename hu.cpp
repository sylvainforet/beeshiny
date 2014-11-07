#include <iostream>
#include <string>
#include <math.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

int main ( )
{


    cv::Mat image = cv::imread ( "1.jpg", CV_LOAD_IMAGE_GRAYSCALE );
    //cv::GaussianBlur( image, image, cv::Size ( 3, 3 ), 0);
    cv::Mat thresh_image;
    cv::adaptiveThreshold ( image, thresh_image, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY, 75, 10);
    //cv::bitwise_not(img, img);

    cv::imwrite( "out.jpg", thresh_image );
    cv::namedWindow ( "Show", cv::WINDOW_AUTOSIZE );
    cv::imshow( "Show", thresh_image );
    cv::waitKey(0);
    return 0;


}

// moments
CvMoments moments;
CvHuMoments humoments;
//First calculate object moments
cvMoments(contourLow, &moments, 0);
//Now calculate hu moments
cvGetHuMoments(&moments, &humoments);


Mat mask = Mat::zeros(image.size(), CV_8UC1);
drawContours(mask, contours_poly, i, Scalar(255), CV_FILLED);

 Mat extractPic;
image.copyTo(extractPic,mask);
Mat resizedPic = extractPic(r);

