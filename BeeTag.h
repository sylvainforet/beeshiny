/* The BeeTag Class Header (BeeTag.h) */
#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <math.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

class BeeTag
{
    private:
        int identity;
        cv::Scalar colour;
        std::vector < cv::Point2f > locations;
        std::vector < int > frame_number;
        std::vector < int > tag_type;
        std::vector < cv::Mat > all_tags;
        cv::Point2f last_location;
        int last_frame;

    public:
        BeeTag ( int, cv::Point2f, int, cv::Scalar, int );

        int retrieve_id ( );
        int retrieve_tag_type ( );
        std::vector < int > retrieve_all_tags ( );
        cv::Point2f retrieve_location ( );
        int retrieve_frame ( );
        std::vector < cv::Point2f > get_all_locations ( );
        std::vector < int > get_all_frames (  );
        cv::Scalar retrieve_colour ( );
        void update_location_frame_classification ( cv::Point2f, int, int );
        void append_all_locations_frames ( std::vector < cv::Point2f >, std::vector < int > );
        void store_tags ( cv::Mat );
        std::vector < cv::Mat > get_all_tags ( );

};