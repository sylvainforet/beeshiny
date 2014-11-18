#include "BeeTag.h"

#define UNKNOWN_TAG 0
#define L_TAG 1
#define STAR_TAG 2
#define QUEEN_TAG 3

int classify_locate_bee ( std::vector < cv::Point >, cv::Mat, int, cv::Scalar, std::vector < BeeTag > &, int &, std::vector < std::vector < cv::Point > > );
float distance_between_tags ( cv::Point2f, cv::Point2f );
//void add_bee ( std::vector < BeeTag > &, int, cv::Point2f, int, cv::Scalar, int );
//bool identify_past_location ( std::vector < BeeTag > &, std::vector < BeeTag >, BeeTag, int );
cv::Mat draw_circles ( cv::Mat, std::vector < BeeTag >, int );
void write_csv ( std::vector < BeeTag >, int );


int main ( )
{
    //std::thread all_threads[num_threads];

    cv::VideoCapture cap ( "/Users/u5305887/Movies/C0003.MP4" );

    cv::VideoWriter output_cap ( "/Users/u5305887/Movies/Output/testing.avi", CV_FOURCC( 'X', 'V', 'I', 'D' ), 25, cv::Size ( 3840, 2160 ), true );

    if ( !cap.isOpened ( ) )
    {
        std::cout << "Couldn't open the video" << std::endl;
        return -1;
    }

    cv::namedWindow ( "video", CV_WINDOW_NORMAL );
    // seed random colour generator 
    cv::RNG rng ( 12345 );
    int frame_count = 0;
    int bee_identities = 0;
    std::vector < BeeTag > allBees;

    while ( 1 )
    {
        cv::Mat frame, gray_frame, smoothed_frame, thresh_frame, output_frame;
        
        bool succeed = cap.read( frame );
        if ( !succeed ) 
        {
            std::cout << "Exiting at " << frame_count << std::endl;
            break;
        }

        // Tag segmentation: colour to gray conversion, smoothing, blocking reflection and thresholding
        cv::cvtColor ( frame, gray_frame, CV_BGR2GRAY );
        cv::GaussianBlur ( gray_frame, smoothed_frame, cv::Size ( 3, 3 ), 1, 1 );

        cv::threshold ( smoothed_frame, thresh_frame, 90, 255, CV_THRESH_BINARY );
        cv::Mat structuringElement = cv::getStructuringElement( cv::MORPH_ELLIPSE, cv::Size( 11, 11 ) );
        cv::morphologyEx ( thresh_frame, thresh_frame, cv::MORPH_CLOSE, structuringElement );

        // extracting contours and storing each contour point in a vector
        std::vector < std::vector < cv::Point > > contours;
        std::vector < cv::Vec4i > hierarchy;
        cv::findContours ( thresh_frame, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE, cv::Point( 0, 0 ) ); // CV_RETR_LIST

        //std::vector < BeeTag > newBees;

        for ( int i = 0; i < contours.size ( ); i++ )
        {
            double area = cv::contourArea ( contours[i] );
            if ( area > 4 ) // filter out small reflective regions of honey
            {
                cv::Scalar assign_colour = cv::Scalar( rng.uniform ( 0, 255 ), rng.uniform ( 0, 255 ) , rng.uniform ( 0, 255 ) );
                int success = classify_locate_bee ( contours[i], gray_frame, frame_count, assign_colour, allBees, bee_identities, contours );
            }

        }

        std::cout << frame_count << " " << allBees.size ( ) << std::endl;

        output_frame = draw_circles ( frame, allBees, frame_count );
        output_cap.write ( output_frame );

        if ( cv::waitKey ( 1 ) == 27 )
        {
            std::cout << "Exiting" << std::endl;
            break;
        }
        
        frame_count++;

    }
    cap.release ( );
    output_cap.release ( );
    std::cout << "Writing CSV" << std::endl;
    write_csv ( allBees, frame_count );
    return 0;
}


int classify_locate_bee ( std::vector < cv::Point > each_contour, cv::Mat grayscale_frame, int frame_number, cv::Scalar newcolour, std::vector < BeeTag > &allBees, int &beeID, std::vector < std::vector < cv::Point > > all_contours )
{
    cv::RotatedRect surrounding_rectangle = cv::minAreaRect( cv::Mat( each_contour ) );
    cv::Point2f locate = surrounding_rectangle.center;
    int classification;

    if ( surrounding_rectangle.size.width < 8 || surrounding_rectangle.size.height < 8 )
    {
        classification = UNKNOWN_TAG;
    }

    else
    {
        long int value = 0;
        for( int row = locate.y - 4; row < locate.y + 4; row++ )
        {
            uchar *pix = grayscale_frame.ptr<uchar>(row);
            for (int col = locate.x - 4 ; col < locate.x + 4 ; col++ )
            {
                value += int ( pix[col] );
            }
        }
        
        int tag_identified;
        if ( value > 14000 )
        {
            classification = QUEEN_TAG;
        }
        else if ( value < 6000 )
        {
            classification = STAR_TAG;
        }
        else if ( value > 7000 && value < 9500 )
        {
            classification = L_TAG;
        }
        else
        {
            classification = UNKNOWN_TAG;
        }

    }

    if ( allBees.empty ( ) )
    {
        BeeTag newBee ( beeID, locate, frame_number, newcolour, classification );
        allBees.push_back( newBee );
        beeID++;
        return 0;
    }

    int tag_number;
    int frames_since_last_seen;
    double closest_match  = 10000000.0; // big number to start with
    for ( int i = 0; i < allBees.size ( ); i++ )
    {
        
        cv::Point2f last_location_of_bee = allBees[i].retrieve_location ( );
        frames_since_last_seen = frame_number - allBees[i].retrieve_frame ( );
        bool better_match_available = false;
        double closeness_of_tag = distance_between_tags ( locate, last_location_of_bee );
        if ( frames_since_last_seen < 125 && closeness_of_tag <= closest_match )
        {
            for ( int j = 0; j < all_contours.size ( ); j++ )
            {
                cv::RotatedRect alternate_surrounding_rectangle = cv::minAreaRect( cv::Mat( each_contour ) );
                cv::Point2f alternate_location = surrounding_rectangle.center;
                float closeness_to_other_tag = distance_between_tags ( alternate_location, last_location_of_bee );
                if ( locate != alternate_location && closeness_to_other_tag < closeness_of_tag )
                {
                    better_match_available = true;
                    break;
                }
            }

            if ( !better_match_available )
            {
                tag_number = i;
                closest_match = closeness_of_tag;
            }
        }
    }

    int search_radius_size = (frames_since_last_seen * 2) + 25;
    if ( closest_match < search_radius_size )
    {
        allBees[tag_number].update_location_frame ( locate, frame_number );
        allBees[tag_number].update_classification ( classification ); // append to vector
    }
    else
    {
        BeeTag newBee ( beeID, locate, frame_number, newcolour, classification );
        allBees.push_back( newBee );
        beeID++;
    }
    return 0;
}

float distance_between_tags ( cv::Point2f first_tag, cv::Point2f second_tag )
{
    float distance = sqrt( pow ( ( first_tag.x - second_tag.x ), 2 ) + pow ( ( first_tag.y - second_tag.y ), 2 ) );
    return distance;
}

cv::Mat draw_circles ( cv::Mat original_frame, std::vector < BeeTag > allBees, int frame_number )
{
    for ( int i = 0; i < allBees.size ( ); i++ )
    {
        int frames_since_last_seen = frame_number - allBees[i].retrieve_frame ( );
        if ( frames_since_last_seen < 125 ) {
            cv::circle ( original_frame, allBees[i].retrieve_location ( ), 16, allBees[i].retrieve_colour ( ), 3, 8, 0 );
            cv::putText ( original_frame, std::to_string ( allBees[i].retrieve_tag_type ( ) ), allBees[i].retrieve_location ( ), CV_FONT_HERSHEY_COMPLEX, 2, cv::Scalar ( 255, 255, 255 ), 5, 8 );
        }
    }
    return original_frame;
}

void write_csv ( std::vector < BeeTag > allBees, int all_frames )
{
    std::ofstream output_csv;
    std::vector < std::vector < cv::Point2f > > bee_location_every_frame ( allBees.size ( ) );
    output_csv.open ( "/Users/u5305887/Movies/Output/locations.csv", std::ios_base::app ); // std::ios_base::app appends to opened CSV file
    output_csv << "BeeID" << "," << "Tag" << "," << "frame" << "," << "X" << "," << "Y";

    for ( int i = 0; i < allBees.size ( ); i++ )
    {

        std::vector < cv::Point2f > every_location_of_bee = allBees[i].get_all_locations ( );
        std::vector < int > all_frames_bee_present = allBees[i].get_all_frames ( );
        std::vector < int > all_tags_classified = allBees[i].retrieve_all_tags ( );

        for ( int j = 0; j < all_frames_bee_present.size ( ); j ++ )
        {
            output_csv << "\n" << allBees[i].retrieve_id ( ) << "," << all_tags_classified[j] << "," << all_frames_bee_present[j] << "," << every_location_of_bee[j].x << "," << every_location_of_bee[j].y;
        }

    }
    output_csv.close ( );
}