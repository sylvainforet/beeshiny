#ifndef BEETAG_H
#include "BeeTag.h"

BeeTag::BeeTag ( int id, cv::Point2f initial_location, int initial_frame, cv::Scalar tag_colour, int tag )
{
    identity = id;
    frame_number.push_back ( initial_frame );
    locations.push_back ( initial_location );
    last_frame = initial_frame;
    last_location = initial_location;
    colour = tag_colour;
    tag_type.push_back ( tag );
}

int BeeTag::retrieve_id ( )
{
    return identity;
}

int BeeTag::retrieve_tag_type ( )
{
    return tag_type.back ( );
}

std::vector < int > BeeTag::retrieve_all_tags ( )
{
    return tag_type;
}

cv::Point2f BeeTag::retrieve_location ( )
{
    return last_location;
}

int BeeTag::retrieve_frame ( )
{
    return last_frame;
}

std::vector < cv::Point2f > BeeTag::get_all_locations ( )
{
    return locations;
}

std::vector < int > BeeTag::get_all_frames ( )
{
    return frame_number;
}


cv::Scalar BeeTag::retrieve_colour ( )
{
    return colour;
}

void BeeTag::update_location_frame_classification ( cv::Point2f location, int frame, int classify )
{
    locations.push_back ( location );
    frame_number.push_back ( frame );
    last_location = location;
    last_frame = frame;
    tag_type.push_back ( classify ) ;
}

void BeeTag::append_all_locations_frames ( std::vector < cv::Point2f > all_new_locations, std::vector < int > all_new_frames )
{
    locations.insert ( locations.end ( ), all_new_locations.begin ( ), all_new_locations.end ( ) );
    frame_number.insert ( frame_number.end ( ), all_new_frames.begin ( ),  all_new_frames.end ( ) );
}

#endif