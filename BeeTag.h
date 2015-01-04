/*
 * The BeeTag Class Header (BeeTag.h)
 * Copyright
 */

/*
 * Documentation
 */

#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <math.h>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#ifndef __BEE_TAG__
#define __BEE_TAG__

class BeeTag
{
    public:
        BeeTag                                      (int id,
                                                     cv::Point2f initial_location,
                                                     int initial_frame,
                                                     int tag_classified);

        void update_location_frame_classification   (cv::Point2f new_location,
                                                     int new_frame,
                                                     int new_classification);

        //virtual ~BeeTag                             (void);
        int get_id                                  (void);
        int get_tag_type                            (void);
        std::vector<int> get_all_tags               (void);
        cv::Point2f get_last_location               (void);
        int get_last_frame                          (void);
        std::vector<cv::Point2f> get_all_locations  (void);
        std::vector<int> get_all_frames             (void);
        cv::Scalar get_circle_colour                (void);
        void clear_stored_objects                   (void);
        
    private:
        int identity;
        std::vector<cv::Point2f> all_tag_locations;
        std::vector<int> all_frame_numbers;
        std::vector<int> all_tags_identified;
        int tag_type;
        cv::Point2f last_location_found;
        int last_frame_found;

};

#endif /* __BEE_TAG__ */