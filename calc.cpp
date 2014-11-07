/* The BeeTag Class Header (BeeTag.h) */



#include <vector>
#include <fstream>
#include <string>
#include <math.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <iostream>
#include <tbb/tbb.h>
#include <tbb/parallel_for.h>

int main(int argc, char const *argv[])
{
    cv::Mat input_img;
    std::cout << "test" << std::endl;
    input_img = cv::imread("~/Dropbox/footage.jpg", CV_LOAD_IMAGE_GRAYSCALE);
int width = 10;
int height =12;
    //#pragma omp parallel for
    tbb::parallel_for(0, width, [=](int x)
{

        std::cout << x << std::endl;
});
    return 0;
}