#include <iostream>
#include <sstream>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/nonfree/nonfree.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/objdetect/objdetect.hpp>

using namespace cv;
using namespace std;

/*
compute difference between current image and background
cv::absdiff(backgroundImage,currentImage,foreground);
*/

int main()
{
	vector<Rect> all_rects;
	VideoCapture cap("/Users/u5305887/Movies/Ajay/test.avi");

	string video_name = "output.avi";

	VideoWriter output_cap(video_name, 
               cap.get(CV_CAP_PROP_FOURCC),
               cap.get(CV_CAP_PROP_FPS),
               Size(cap.get(CV_CAP_PROP_FRAME_WIDTH),
               cap.get(CV_CAP_PROP_FRAME_HEIGHT)),true); // false enables output in grayscale video


	if (!cap.isOpened()){
		cout << "Couldn't open the video" << endl;
		return -1;
	}

	namedWindow("video", CV_WINDOW_AUTOSIZE);
	int i=0;
	//Mat erosion = getStructuringElement(MORPH_ELLIPSE,Size(5,5));//MORPH_ELLIPSE MORPH_CROSS MORPH_RECT
	//Mat dilation = getStructuringElement(MORPH_ELLIPSE,Size(35,35));//MORPH_ELLIPSE MORPH_CROSS MORPH_RECT
	while(1){
		Mat frame, grayframe;
		bool succeed = cap.read(frame);
		if(!succeed){
			cout << "Done" << endl;
			break;
		}

		
		//medianBlur(frame, frame,9);
		
		Mat newframe;
		int rows=frame.rows;
		int cols = frame.cols;
		Size framesize = frame.size();
		rows = framesize.height;
		cols=framesize.width;
		
		
		
		

		//Covering rectangles: middle, top. bottom, left, right
		//rectangle(frame, Point(cols,930), Point(0,1150), 0, -1);
		rectangle(frame, Point(cols,0), Point(0,390), 0, -1);
		rectangle(frame, Point(cols,rows-175), Point(0,rows), 0, -1);
		rectangle(frame, Point(400,0), Point(0,rows), 0, -1);
		rectangle(frame, Point(cols-320,0), Point(cols,rows), 0, -1);

		cvtColor(frame,newframe, CV_BGR2GRAY);

		//bilateralFilter(frame, newframe, 3, 210, 0);
		
		//bilateralFilter(frame, newframe, 3, 220, 10);

		GaussianBlur(newframe,newframe, Size(3, 3), 1, 1);
		//blur(frame,frame,Size(11,11),Point(-1,-1));
		//medianBlur(grayframe,grayframe, 147);

		//adaptiveBilateralFilter(frame, newframe, Size(5,5),50);
		//equalizeHist(newframe,newframe);
		//imwrite("test.tif",newframe);
		//threshold(newframe,newframe, 194, 255,CV_THRESH_TOZERO);
		threshold(newframe,newframe, 120, 255,CV_THRESH_BINARY);

	vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;
    RNG rng(12345);
    findContours(newframe, contours, hierarchy, CV_RETR_TREE, CV_RETR_LIST, Point(0, 0) );
    /// Draw contours
    //Mat drawing = Mat::zeros(filtered_frame.size(), CV_8UC3 );
    //Mat drawing = Mat::zeros(newframe.size(), CV_8UC3);
    for( int i = 0; i< contours.size(); i++ )
    {
    	all_rects.push_back(boundingRect(contours.at(i)));
    	//double area = contourArea(contours[i]);
		//Rect hi = boundingRect(contours.at(i));
    	//rectangle(frame, hi,255,2,8,0);

    		//cout << "this " << i << endl;
    	    //Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255));
    		//int color = 255;
    	        //drawContours(drawing, contours, i, color, -1, 1, hierarchy, 0, Point(0,0));
    	    }
   	for( int i = 0; i< all_rects.size(); i++ )
   	{
   		rectangle(frame, all_rects.at(i),255,2,8,0);
   	}

    

		//adaptiveThreshold(newframe,newframe,255,ADAPTIVE_THRESH_GAUSSIAN_C, CV_THRESH_BINARY,33,2);
		
		//adaptiveThreshold(newframe,newframe,255,ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY,3,1);
		//GaussianBlur(newframe,newframe, Size(5, 5), 2, 2 );

		//Laplacian(newframe, newframe, CV_8UC1,1,1, BORDER_DEFAULT );
		//dilate(newframe,newframe,dilation);
		//erode(newframe,newframe,erosion);
		//bitwise_and(frame,newframe,frame);

		//CV_ADAPTIVE_THRESH_MEAN_C
		//Sobel(grayframe, grayframe, CV_8U, 1, 0, 3,1,0);
		imshow("video",frame);
		//string image_name= "image"+to_string(i)+".tif";
		cout << i << endl;
		//imwrite("thresh2.tif",grayframe);
		output_cap.write(frame);
		if(waitKey(1)==27){
			cout << "Exiting" << endl;
			break;
		}
		i++;
	}
	cap.release();
	output_cap.release();
	return 0;
}