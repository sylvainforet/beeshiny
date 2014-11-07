#include <iostream>
#include <fstream>
#include <string>
#include <math.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#define CIRCLE_TAG 0
#define CROISSANT_TAG 1
#define UNKNOWN_TAG 2

int attempt_classification ( std::vector < cv::Point > each_contour, cv::Mat thresh_frame, cv::Point2f &locate )
{
    std::vector < cv::Point > contours_poly;
    cv::Point2f centre;
    float radius;
    cv::approxPolyDP ( cv::Mat( each_contour ), contours_poly, arcLength ( cv::Mat( each_contour ), true ) * 0.001, true );
    cv::minEnclosingCircle ( cv::Mat ( contours_poly ), centre, radius );
    locate = centre;
    
    if ( radius < 4.6 )
    {
        return UNKNOWN_TAG;
    }

    else
    {
        int count_zero = 0;

        for ( int row = centre.y - 2; row < int( centre.y + 3 ); ++row )
        {
            if ( int( thresh_frame.at<uchar>( row, centre.x - 2 ) == 0 ) ) 
            {
                count_zero++;
            }

            if ( int( thresh_frame.at<uchar>( row, centre.x + 2 ) == 0 ) ) 
            {
                count_zero++;
            }

        } // end incrementing over left and right cols

        for ( int col = centre.x - 2; col < int( centre.x + 3 ); ++col )
        {
            //std::cout << row << std::endl;
            if ( int( thresh_frame.at<uchar>( centre.y - 2, col ) == 0 ) ) 
            {
                count_zero++;
            }

            if ( int( thresh_frame.at<uchar>( centre.y + 2, col ) == 0 ) ) 
            {
                count_zero++;
            }

        } // end incrementing over top and bottom rows

        if ( count_zero == 0 )
        {
            return CIRCLE_TAG;
        }

        else 
        {
            return CROISSANT_TAG;
        }
        

    }

}

void add_bee ( std::vector < BeeTag > &allBees, int id, cv::Point2f initial_location, int initial_frame, cv::Scalar tag_colour, int tag_type )
{
    BeeTag newBee ( id, initial_location, initial_frame, tag_colour, tag_type );
    allBees.push_back( newBee );
}

bool identify_past_location ( std::vector < BeeTag > &allBees, cv::Point2f location, int frame_number, int classification )

/////////////////////////////////////////// FIND CLUSTERS
//    std::vector < std::vector < int > > clustered;
    // start finding clusters of bees
    //allBees[i].retrieve_location ( );

{
    bool new_bee_identified = true;
    if ( allBees.empty ( ) )
    {
        return new_bee_identified;
    }

    int tag_number;
    float closest_match = 0;
    bool first_comparison = true;
    for ( int i = 0; i < allBees.size ( ); i++ )
    {
        cv::Point2f last_location_of_bee = allBees[i].retrieve_location ( );
        float closeness_of_tag = distance_between_tags ( location, last_location_of_bee );
        if ( first_comparison or closeness_of_tag < closest_match )
        {
            tag_number = i;
            closest_match = closeness_of_tag;
            first_comparison = false;
        }
    }

    int frames_since_last_seen = frame_number - allBees[tag_number].retrieve_frame ( );
    int search_radius_size;
    if ( frames_since_last_seen == 0 || frames_since_last_seen > 200 ) ////////////// update second condition
    {
        search_radius_size = 15;
    }
    else
    {
        search_radius_size = frames_since_last_seen * 15;
    }
    
    if ( closest_match < search_radius_size )
    {
        new_bee_identified = false;
        allBees[tag_number].update_location_frame ( location, frame_number );
        if ( classification != UNKNOWN_TAG )
        {
            allBees[tag_number].update_classification ( classification );
        }
    }

    return new_bee_identified;
}

float distance_between_tags ( cv::Point2f first_tag, cv::Point2f second_tag )
{
    float distance = sqrt( pow ( ( first_tag.x - second_tag.x ), 2 ) + pow ( ( first_tag.y - second_tag.y ), 2 ) );
    return distance;
}

cv::Mat draw_circles ( cv::Mat original_frame, std::vector < BeeTag > allBees )
{
    for ( int i = 0; i < allBees.size ( ); i++ )
    {
        cv::circle ( original_frame, allBees[i].retrieve_location ( ), 16, allBees[i].retrieve_colour ( ), 3, 8, 0 );
        cv::putText ( original_frame, std::to_string ( allBees[i].retrieve_id ( ) ), allBees[i].retrieve_location ( ), CV_FONT_HERSHEY_COMPLEX, 2, cv::Scalar ( 255, 255, 255 ), 5, 8 );
        
    }
    return original_frame;
}

void write_csv ( std::vector < BeeTag > allBees, int all_frames )
{
    std::ofstream output_csv;
    std::vector < std::vector < cv::Point2f > > bee_location_every_frame ( allBees.size ( ) );
    output_csv.open ( "/Users/u5305887/Movies/Output/locations.csv" );
    output_csv << "Bee ID" << "," << "frame" << "," << "X" << "," << "Y";

    for ( int i = 0; i < allBees.size ( ); i++ )
    {

        std::vector < cv::Point2f > every_location_of_bee = allBees[i].get_all_locations ( );
        std::vector < int > all_frames_bee_present = allBees[i].get_all_frames ( );

        for ( int j = 0; j < all_frames_bee_present.size ( ); j ++ )
        {
            output_csv << "\n" << allBees[i].retrieve_id ( ) << "," << all_frames_bee_present[j] << "," << every_location_of_bee[j].x << "," << every_location_of_bee[j].y;
        }

    }
    output_csv.close ( );
}

/*


        std::cout << "Writing bee number " << allBees[i].retrieve_id ( ) << " out of " << allBees.size ( ) - 1 << std::endl;
        output_csv << allBees[i].retrieve_id ( ) << "," << allBees[i].retrieve_tag_type ( );
        std::vector < cv::Point2f > every_location_of_bee = allBees[i].get_all_locations ( );

        for ( int j = 0; j < every_location_of_bee.size ( ); j ++ )
        {
            if ( every_location_of_bee[j] == cv::Point2f ( 0.f, 0.f ) )
            {
                output_csv << "," << "NA";
            }
            else
            {
                output_csv << "," << every_location_of_bee[j].x << " " << every_location_of_bee[j].y;
            }
        }

        output_csv << "\n";

    }
    
    output_csv.close ( );
}
*/

/*
void update_bee_locations ( std::vector < BeeTag > &allBees, int frame_number )
{
    for ( int i = 0; i < allBees.size ( ); i++ )
    {
        bool not_present;
        if ( allBees[i].retrieve_frame ( ) != frame_number )
        {
            not_present = true;
            allBees[i].update_location_frame ( cv::Point2f ( 0.f, 0.f ), frame_number );
        }
        else
        {
            not_present = false;
        }

        allBees[i].update_frames_since_last_seen ( not_present );

    }

}
*/

void reset_class_vectors ( )
{
    std::cout << "Clear every 10k frames";
}