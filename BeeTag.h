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

#ifndef __BEE_TAG__
#define __BEE_TAG__

class BeeTag
{
    public:
        BeeTag                                 (int id,
                                                cv::Point2f initial_location,
                                                int initial_frame,
                                                int tag);

        void add_point                         (cv::Point2f new_location,
                                                int new_frame,
                                                int new_classification);

        int get_id                             (void) const;
        int get_tag_type                       (void) const;
        std::vector<int> get_tags              (void) const;
        std::vector<cv::Point2f> get_locations (void) const;
        std::vector<int> get_frames            (void) const;
        cv::Point2f get_last_location          (void) const;
        int get_last_frame                     (void) const;
        void clear                             (void)      ;
        void delete_bee                        (void)      ;
        bool is_deleted                        (void) const;

    private:
        int identity;
        std::vector<cv::Point2f> locations;
        std::vector<int> frames;
        std::vector<int> tags;
        cv::Point2f last_location;
        int last_frame;
        int tag_type;
        bool deleted = false;

};

#endif /* __BEE_TAG__ */

/* vim:ts=4:sw=4:sts=4:expandtab:
 */
