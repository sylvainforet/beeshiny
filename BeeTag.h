/* The BeeTag Class Header (BeeTag.h) */
#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <math.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <tbb/tbb.h>
#include <tbb/parallel_for.h>

#include <opencv2/nonfree/nonfree.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <thread>

class BeeTag
{
    private:
        int identity;
        cv::Scalar colour;
        std::vector < cv::Point2f > locations;
        std::vector < int > frame_number;
        int tag_type;
        std::vector < cv::Mat > all_tags;

    public:
        BeeTag ( int, cv::Point2f, int, cv::Scalar, int );

        int retrieve_id ( );
        int retrieve_tag_type ( );
        cv::Point2f retrieve_location ( );
        int retrieve_frame ( );
        std::vector < cv::Point2f > get_all_locations ( );
        std::vector < int > get_all_frames (  );
        cv::Scalar retrieve_colour ( );
        void update_location_frame ( cv::Point2f, int );
        void update_classification ( int );
        void append_all_locations_frames ( std::vector < cv::Point2f >, std::vector < int > );
        void store_tags ( cv::Mat );
        std::vector < cv::Mat > get_all_tags ( );

};