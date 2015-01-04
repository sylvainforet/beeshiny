#ifndef BEETAG_H
#include "BeeTag.h"

BeeTag::BeeTag                                          (int id, 
                                                        cv::Point2f initial_location, 
                                                        int initial_frame,
                                                        int tag_classified)
{
    identity = id;
    all_frame_numbers.push_back (initial_frame);
    all_tag_locations.push_back (initial_location);
    last_frame_found = initial_frame;
    last_location_found = initial_location;
    all_tags_identified.push_back (tag_classified);
    tag_type = tag_classified;
}

void BeeTag::update_location_frame_classification       (cv::Point2f new_location,
                                                        int new_frame,
                                                        int new_classification)
{
    all_tag_locations.push_back (new_location);
    all_frame_numbers.push_back (new_frame);
    last_location_found = new_location;
    last_frame_found = new_frame;
    all_tags_identified.push_back (new_classification);
    tag_type = new_classification;
}

void BeeTag::clear_stored_objects                       (void)
{
    all_tag_locations.clear ();
    all_frame_numbers.clear ();
    all_tags_identified.clear ();
}

//BeeTag::~BeeTag                                         (void)
//{
 //   std::cout << "Deleting Object" << std::endl;
//}

int BeeTag::get_id (void)
{
    return identity;
}

int BeeTag::get_tag_type                                (void)
{
    return tag_type;
}

std::vector<int> BeeTag::get_all_tags                   (void)
{
    return all_tags_identified;
}

cv::Point2f BeeTag::get_last_location                   (void)
{
    return last_location_found;
}

int BeeTag::get_last_frame                              (void)
{
    return last_frame_found;
}

std::vector<cv::Point2f> BeeTag::get_all_locations      (void)
{
    return all_tag_locations;
}

std::vector<int> BeeTag::get_all_frames                 (void)
{
    return all_frame_numbers;
}

#endif