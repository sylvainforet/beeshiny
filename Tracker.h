#include <iostream>
#include <fstream>
#include <string>
#include <math.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

int attempt_classification ( std::vector < cv::Point >, cv::Mat, cv::Point2f & );
float distance_between_tags ( cv::Point2f, cv::Point2f );
void add_bee ( std::vector < BeeTag > &, int, cv::Point2f, int, cv::Scalar, int );
bool identify_past_location ( std::vector < BeeTag > &, cv::Point2f, int, int );
//void update_bee_locations ( std::vector < BeeTag > &, int );
cv::Mat draw_circles ( cv::Mat, std::vector < BeeTag > );
void write_csv ( std::vector < BeeTag >, int );