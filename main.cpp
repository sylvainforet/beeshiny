#include "BeeTag.h"

#define UNKNOWN_TAG 1

int attempt_classification ( std::vector < cv::Point >, cv::Mat, cv::Point2f &, int );
float distance_between_tags ( cv::Point2f, cv::Point2f );
void add_bee ( std::vector < BeeTag > &, int, cv::Point2f, int, cv::Scalar, int );
bool identify_past_location ( std::vector < BeeTag > &, cv::Point2f, int, int, std::vector < cv::Point2f > );
std::vector < BeeTag > merge_previously_tracked ( std::vector < BeeTag > );
cv::Mat draw_circles ( cv::Mat, std::vector < BeeTag > );
void write_csv ( std::vector < BeeTag >, int );

int main ( )
{
    cv::VideoCapture cap ( "/Users/u5305887/Movies/C0019.MP4" );

    cv::VideoWriter output_cap ( "/Users/u5305887/Movies/Output/parallel.avi", CV_FOURCC( 'X', 'V', 'I', 'D' ), 25, cv::Size(3840, 2160), true );

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
        gray_frame = cv::imread("/Users/u5305887/Movies/Output/test.jpg", CV_LOAD_IMAGE_GRAYSCALE);
        cv::GaussianBlur ( gray_frame, smoothed_frame, cv::Size ( 3, 3 ), 1, 1 );
        
        
        // right black rectangle to block reflection
        // use for printed_tags
        /*
        cv::rectangle ( smoothed_frame, 
            cv::Point ( smoothed_frame.cols - 2160, 0 ),
            cv::Point ( smoothed_frame.cols, smoothed_frame.rows ), 0, -1 );

        cv::rectangle ( smoothed_frame, 
            cv::Point ( 0, 0 ),
            cv::Point ( 1658, smoothed_frame.rows ), 0, -1 );
        */
        //cv::resize(smoothed_frame, smoothed_frame, cv::Size(smoothed_frame.cols/2, smoothed_frame.rows/2)); 
        //cv::imshow ( "video", smoothed_frame ); ////////////////////////////////////////////////////////////
        

        //cv::imwrite ( "/Users/u5305887/Movies/Output/th.jpg", frame );
        
        // extracting 
        cv::threshold ( smoothed_frame, thresh_frame, 130, 255, CV_THRESH_BINARY );
        cv::Mat structuringElement = cv::getStructuringElement( cv::MORPH_RECT, cv::Size(5, 5));
        cv::morphologyEx ( thresh_frame, thresh_frame, cv::MORPH_CLOSE, structuringElement );
        /*if (allBees.size ( ) == 5){
            std::string image_name = "/Users/u5305887/Movies/Output/tags/breaking.jpg";
            cv::imwrite ( image_name, thresh_frame );
            break;
        }*/
        /*
        if (frame_count == 13){
            cv::imwrite("/Users/u5305887/Movies/Output/hmm.jpg", thresh_frame);
            break;
        } */
        //thresh_frame = gray_frame.clone ( ); // copying as applying contours function modifies thresholded image

        // extracting contours and storing each contours points in a vector
        std::vector < std::vector < cv::Point > > contours;
        std::vector < cv::Vec4i > hierarchy;
        cv::findContours ( thresh_frame, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE, cv::Point( 0, 0 ) ); // CV_RETR_LIST
        //std::cout << "CONTOURS " << contours.size ( ) << std::endl;
        std::vector < cv::Point2f > frame_tag_locations;
        std::vector < int > tag_classifications;
        // incrementing over each contour identified
        
        for ( int i = 0; i < contours.size ( ); i++ )
        {
            double area = cv::contourArea ( contours[i] );
            if ( area >= 10 ) // filter out small reflective regions of honey
            {
                // attempts to identify the tag type, if area is too small, returns unknown
                // Returns an integer as tag types are defined as a number
                // Passes the location variable by reference so that tag centre can be found (more efficient)
                cv::Point2f location;
                int classify = attempt_classification ( contours[i], gray_frame, location, frame_count );
                tag_classifications.push_back ( classify );
                frame_tag_locations.push_back ( location );
            }

        }

        // separated this loop from contours because I need to be able to find nearest matching tag and 
        //ensure an identified tag nearest to 2 tags isn't applied twice
        
        for ( int i = 0; i < tag_classifications.size ( ); i++ )
        {
            // use found_new_bee boolean to determine if I need to initialise a new bee object
            // identify_past_location tests to see if I've found a tag in a nearby location previously
            bool found_new_bee;
            found_new_bee = identify_past_location ( allBees, frame_tag_locations[i], frame_count, tag_classifications[i], frame_tag_locations );
            // if true, I initialise a new bee object, with a random colour and append to the bee vector
            if ( found_new_bee )
            {
                cv::Scalar assign_colour = cv::Scalar( rng.uniform ( 0, 255 ), rng.uniform ( 0, 255 ) , rng.uniform ( 0, 255 ) );
                add_bee ( allBees, bee_identities, frame_tag_locations[i], frame_count, assign_colour, tag_classifications[i] );
                bee_identities++;
            }

        }

        allBees = merge_previously_tracked ( allBees );
        std::cout << frame_count << " " << allBees.size ( ) << std::endl;
        //update_bee_locations ( allBees, frame_count );


        // setting location for bees not in frame at Point ( 0, 0 )

        //cv::imshow ( "video", frame );
        //std::string image_name = "/Users/u5305887/Movies/Output/frames/frame" + std::to_string ( frame_count ) + ".jpg";
        //cv::imwrite ( image_name, frame );
        //std::cout << allBees.size ( ) << std::endl;
        //write_csv ( allBees, frame_count );
        //std::cout << frame_count << std::endl;
        break;
        output_frame = draw_circles ( frame, allBees );
        output_cap.write ( output_frame );
        //cv::imwrite ( "/Users/u5305887/Movies/Output/test.jpg", output_frame );
        //break;
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

int attempt_classification ( std::vector < cv::Point > each_contour, cv::Mat grayscale_frame, cv::Point2f &locate, int frame_num )
{
    //std::vector < cv::Point > contours_poly;
    //cv::approxPolyDP ( cv::Mat( each_contour ), contours_poly, arcLength ( cv::Mat( each_contour ), true ) * 0.001, true );
    //cv::minEnclosingCircle ( cv::Mat ( contours_poly ), centre, radius );
    //ellipse = fitEllipse( cv::Mat ( contours[i] ) );
    cv::RotatedRect surrounding_rectangle = cv::minAreaRect( cv::Mat( each_contour ) );
    //cv::Point2f rectangle_points[4];

    //surrounding_rectangle.points ( rectangle_points );
    //std::cout << surrounding_rectangle.size << " " << surrounding_rectangle.size.width << " " << surrounding_rectangle.size.height << std::endl;
    //std::cout << rectangle_points[0] << " " << rectangle_points[1] << " " << rectangle_points[2] << " " << rectangle_points[3] <<  std::endl;
    int hw_diff = abs ( surrounding_rectangle.size.width - surrounding_rectangle.size.height );
    //std::cout << hw << std::endl;
    locate = surrounding_rectangle.center;
    //std::cout << radius << std::endl;
    if ( surrounding_rectangle.size.width < 10 || hw_diff > 2 )
    {
        return UNKNOWN_TAG;
    }

    else
    {
        cv::Rect roi = cv::Rect ( locate.x - 6, locate.y - 6, 12, 12 );
        cv::Mat tag_area = grayscale_frame ( roi );
        cv::Mat masking = cv::Mat::ones ( 12, 12, CV_8UC1 );
        masking *= 255;
        cv::circle ( masking, cv::Point ( 6, 6 ), 6 - 1, 0, -1, 8, 0 ); //previously radius - 1
        cv::Mat tag_character;
        cv::bitwise_or ( tag_area, masking, tag_character );
        cv::imwrite ( "/Users/u5305887/Movies/Output/train_test.jpg", tag_character );
        for ( int ii = 0; ii < 360; ii++ )
        {
            break;
            cv::Mat res;
            cv::Point2f pt(tag_character.cols/2., tag_character.rows/2.);
            cv::Mat r = cv::getRotationMatrix2D(pt, ii, 1.0);
            cv::warpAffine(tag_character, res, r, cv::Size(tag_character.cols, tag_character.rows), cv::INTER_CUBIC, cv::BORDER_CONSTANT, 255);
            std::string image_name = "/Users/u5305887/Movies/Output/tags/y_l/" + std::to_string( frame_num ) + "_" + std::to_string( ii ) + ".jpg";
            cv::imwrite ( image_name, res );
        }
        /*
        int value;
        for( int x=0; x < tag_character.rows; x++ )
        {
            for( int y = 0; y < tag_character.cols; y++ ) 
            {
                value += int ( tag_character.at<uchar>( x, y ) );
                
            }
        }
        std::cout << value << std::endl;
        */
        
        

/*
        cv::Mat descriptors;
        cv::SiftFeatureDetector detector;
        //cv::SiftDescriptorExtractor detector(3); 
        //cv::DenseFeatureDetector detector(true);
        std::vector<cv::KeyPoint> keypoints;
        detector.detect(tag_character, keypoints);
        cv::SiftDescriptorExtractor extractor;
        extractor.compute(tag_character, keypoints, descriptors);
        cv::Mat output;
        //cv::drawKeypoints(tag_character, keypoints, output);
        //cv::imwrite("/Users/u5305887/Movies/Output/tags/sift_result.jpg", output);
        if (keypoints.size() != 0){
            std::cout << "THIS " << descriptors.size() << std::endl;
        }
        else{
            cv::imwrite("/Users/u5305887/Movies/Output/tags/sift_result.jpg", tag_character);
        }
        //std::cout << "NUMBER " << keypoints.size() << std::endl;
        
*/

        //std::string image_name = "/Users/u5305887/Movies/Output/tags/" + std::to_string ( value ) + ".jpg";
        //cv::imwrite ( image_name, tag_character );

        /*
        // moments
        CvMoments moments;
        CvHuMoments humoments;
        //First calculate object moments
        cvMoments(contourLow, &moments, 0);
        //Now calculate hu moments
        cvGetHuMoments(&moments, &humoments);
        */
        return UNKNOWN_TAG; // change later once humoments sorted
    }

}

void add_bee ( std::vector < BeeTag > &allBees, int id, cv::Point2f initial_location, int initial_frame, cv::Scalar tag_colour, int tag_type )
{
    BeeTag newBee ( id, initial_location, initial_frame, tag_colour, tag_type );
    allBees.push_back( newBee );
}

bool identify_past_location ( std::vector < BeeTag > &allBees, cv::Point2f location, int frame_number, int classification, std::vector < cv::Point2f > all_tag_locations )

{
    bool new_bee_identified = true;
    if ( allBees.empty ( ) )
    {
        return new_bee_identified;
    }

    int tag_number;
    int frames_since_last_seen;
    float closest_match  = 10000000; // big number to start with
    for ( int i = 0; i < allBees.size ( ); i++ )
    {
        cv::Point2f last_location_of_bee = allBees[i].retrieve_location ( );
        frames_since_last_seen = frame_number - allBees[i].retrieve_frame ( );
        bool better_match_available = false;
        float closeness_of_tag = distance_between_tags ( location, last_location_of_bee );
        if ( frames_since_last_seen < 100 && closeness_of_tag < closest_match )
        {

            if ( classification != UNKNOWN_TAG && allBees[i].retrieve_tag_type ( ) == classification )
            {
                tag_number = i;
                closest_match = closeness_of_tag;
                break;
            }

            for ( int j = 0; j < all_tag_locations.size ( ); j++ )
            {
                float closeness_to_other_tag = distance_between_tags ( all_tag_locations[j], last_location_of_bee );
                if ( location != all_tag_locations[j] && closeness_to_other_tag < closeness_of_tag )
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

    int search_radius_size;
    if ( frames_since_last_seen == 0 )
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
        if ( classification != UNKNOWN_TAG && allBees[tag_number].retrieve_tag_type ( ) == UNKNOWN_TAG )
        {
            allBees[tag_number].update_classification ( classification );
        }
    }

    return new_bee_identified;
}

std::vector < BeeTag > merge_previously_tracked ( std::vector < BeeTag > allBees ) // worth running over x number of frames?
{
    std::vector < int > tag_types_found;
    std::vector < BeeTag > updated_all_bees;
    for ( int i = 0; i < allBees.size ( ); i++ )
    {
        int classification = allBees[i].retrieve_tag_type ( );
        if ( tag_types_found.empty ( ) || classification == UNKNOWN_TAG )
        {
            tag_types_found.push_back ( classification );
            updated_all_bees.push_back ( allBees[i] );
        }
        else
        {
            bool tag_found_previously = false;
            for ( int j = 0; j < tag_types_found.size ( ); j++ )
            {
                if ( classification == tag_types_found[j] )
                {
                    updated_all_bees[j].append_all_locations_frames ( allBees[j].get_all_locations ( ), allBees[j].get_all_frames ( ) );
                    tag_types_found.push_back ( 0 );
                    tag_found_previously = true;
                    break;
                }
            }

            if ( !tag_found_previously )
            {
                tag_types_found.push_back ( classification );
                updated_all_bees.push_back ( allBees[i] );
            }


        }


    }

    return updated_all_bees;

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

void free_ram ( int frame_number )
{
    if ( frame_number != 0 && frame_number % 10000 == 0 )
    {
        std::cout << "Clear every 10k frames";
    }

}