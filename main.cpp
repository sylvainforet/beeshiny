/**
 * JS 2014
 * This does this and that ...
 */

#include "BeeTag.h"

#define UNKNOWN_TAG                 0
#define O_TAG                       1
#define I_TAG                       2
#define QUEEN_TAG                   3
#define MIN_CONTOUR_AREA            20
#define MORPH_TRANSFORM_SIZE        27
#define FRAMES_BEFORE_CLEAR_MEMORY  20001
#define FRAMES_BEFORE_EXTINCTION    500
#define SEARCH_SURROUNDING_AREA     500
#define SEARCH_EXPANSION_BY_FRAME   80
#define MIN_TAG_CLASSIFICATION_SIZE 22

cv::Point2f classify_locate_bee     (std::vector<cv::Point> each_contour,
                                    cv::Mat thresh_shape_frame,
                                    int &classify_tag);

bool identify_past_location         (std::vector<BeeTag> &allBees,
                                    std::vector<cv::Point2f> tags_found_location,
                                    std::vector<int> classify,
                                    int iterator,
                                    int frame_number);

static float distance_between_tags  (cv::Point2f first_tag,
                                    cv::Point2f second_tag);

static cv::Mat draw_circles         (cv::Mat original_frame,
                                    std::vector<BeeTag> allBees,
                                    int frame_number);

static void write_csv               (std::vector<BeeTag> &allBees,
                                    int all_frames,
                                    char *csv_argument);

int main (int argc, char *argv[])
{
    if (argc != 4)
    {
        std::cout << "Requires location to video file, output video file and output csv file" << std::endl;
        return -1;
    }

    char answer;
    std::cout << "Have you changed the name of the input and output video and csv file so you do not overwrite previous analysis? (Y/N) ";
    //std::cin >> answer;
    answer = 'Y'; // delete later
    if (answer != 'Y')
    {
        std::cout << "Exiting program" << std::endl;
        return -1;
    }

    cv::VideoCapture cap (argv[1]);
    cv::VideoWriter output_cap (argv[2], CV_FOURCC ('X', 'V', 'I', 'D'), 25, cv::Size (3840, 2160), false);
    cv::namedWindow ("video", CV_WINDOW_NORMAL);

    std::ofstream output_csv;
    output_csv.open (argv[3]);
    output_csv << "BeeID" << "," << "Tag" << "," << "frame" << "," << "X" << "," << "Y";
    output_csv.close ();
    
    if (!cap.isOpened ())
    {
        std::cout << "Couldn't open the video" << std::endl;
        return -1;
    }

    int frame_count = 0;
    int bee_identities = 0;
    std::vector<BeeTag> allBees;
    cv::Mat frame, gray_frame, smooth_frame, thresh_frame, thresh_shape, output_frame;
    cv::Mat close_element = cv::getStructuringElement (cv::MORPH_RECT, cv::Size (MORPH_TRANSFORM_SIZE, MORPH_TRANSFORM_SIZE)); //ELLIPSE

    while (1)
    {
        bool succeed = cap.read (frame);
        if(!succeed) 
        {
            std::cout << "Exiting at " << frame_count << std::endl;
            break;
        }
/*
        if (frame_count % 5 != 0)
        {
            frame_count++;
            continue;
        }
        6155
*/

        // Tag segmentation: colour to gray conversion, smoothing,closing and blocking reflection and thresholding
        cv::cvtColor (frame, gray_frame, CV_BGR2GRAY);
        //cv::GaussianBlur (gray_frame, smooth_frame, cv::Size (5, 5), 1, 1);
        //cv::blur (gray_frame, smooth_frame, cv::Size (7, 7));
        //cv::medianBlur (gray_frame, smooth_frame, 5);
        //output_cap.write (smooth_frame);
        /*
        cv::rectangle (smooth_frame, 
                       cv::Point (gray_frame.cols - 550, 0),
                       cv::Point (gray_frame.cols, gray_frame.rows), 0, -1);

        cv::rectangle (smooth_frame, 
                       cv::Point (gray_frame.cols - 1560, gray_frame.rows - 30),
                       cv::Point (gray_frame.cols - 1525, gray_frame.rows), 0, -1);
        // covers base
        cv::rectangle (smooth_frame, 
                       cv::Point (0, gray_frame.rows - 100),
                       cv::Point (gray_frame.cols, gray_frame.rows), 0, -1);
        */
        //cv::blur (gray_frame, smooth_frame, cv::Size (5, 5));
        cv::threshold (gray_frame, thresh_frame, 90, 255, CV_THRESH_BINARY);
        cv::blur (thresh_frame, thresh_frame, cv::Size (19, 19));
        cv::threshold (thresh_frame, thresh_frame, 90, 255, CV_THRESH_BINARY);
        //cv::morphologyEx (thresh_frame, thresh_frame, cv::MORPH_CLOSE, close_element);
        //output_cap.write (thresh_frame);
        //
        //cv::blur (thresh_frame, thresh_frame, cv::Size (19, 19));
        //cv::threshold (thresh_frame, thresh_frame, 50, 255, CV_THRESH_BINARY);
        cv::threshold (gray_frame, thresh_shape, 180, 255, CV_THRESH_BINARY_INV);
        //cv::morphologyEx (thresh_frame, thresh_frame, cv::MORPH_CLOSE, close_element);
        
        cv::Mat ero = cv::getStructuringElement (cv::MORPH_CROSS, cv::Size (5, 5)); //ELLIPSE
        //cv::morphologyEx (thresh_shape, thresh_shape, cv::MORPH_OPEN, ero);
        cv::erode (thresh_shape, thresh_shape, ero);
        //output_cap.write (thresh_shape);
        
        std::vector<std::vector<cv::Point>> contours;
        std::vector<cv::Vec4i> hierarchy;
 
        cv::findContours (thresh_frame, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE, cv::Point (0, 0)); // CV_RETR_LIST

        std::vector<cv::Point2f> tag_locations;
        std::vector<int> tag_classifications;
        //std::cout << "Good1" << std::endl;
        for (int i = 0; i < contours.size (); i++)
        {
            if (cv::contourArea (contours[i]) > MIN_CONTOUR_AREA)
            {
                //cv::Point2f new_location = classify_locate_bee (contours[i], thresh_shape, tag_classifications[i]);
                //tag_locations[i] = new cv::Point2f(new_location);
                //std::cout << "working\n" << std::endl;
                int classified; // passed by reference
                cv::Point2f located = classify_locate_bee (contours[i], thresh_shape, classified);
                tag_locations.push_back (located);
                tag_classifications.push_back (classified);
                //std::cout << cv::contourArea (contours[i]) << std::endl;
            }

        }

        //std::cout << "Good2" << std::endl;

        //std::cout << tag_locations.size () << std::endl;

        //std::vector<bool> new_bee_found;
        //bool new_bee_found[contours.size ()];
        
        for (int i = 0; i < tag_locations.size (); i++)
        {
            bool new_bee_found = identify_past_location (allBees, tag_locations, tag_classifications, i, frame_count);
            //std::cout << new_bee_found << std::endl;
            if (new_bee_found == true)
            {
                BeeTag newBee (bee_identities, tag_locations[i], frame_count, tag_classifications[i]);
                allBees.push_back (newBee);
                bee_identities++;
            }
        }
        
        std::cout << frame_count << " " << allBees.size () << std::endl;
        

        output_frame = draw_circles (gray_frame, allBees, frame_count);
        output_cap.write (output_frame);

        if (cv::waitKey (1) == 27)
        {
            std::cout << "Exiting" << std::endl;
            break;
        }
        
        frame_count++;

        if (frame_count % FRAMES_BEFORE_CLEAR_MEMORY == 0)
        {
            std::cout << "Writing CSV" << std::endl;
            write_csv (allBees, frame_count, argv[3]);
        }

    }
    cap.release ();
    output_cap.release ();
    std::cout << "Writing CSV" << std::endl;
    write_csv (allBees, frame_count, argv[3]);
    return 0;
}


cv::Point2f classify_locate_bee (std::vector<cv::Point> each_contour,
                                cv::Mat thresh_shape_frame,
                                int &classify_tag)

{
    cv::RotatedRect surrounding_rectangle = cv::minAreaRect (cv::Mat(each_contour));
    cv::Point2f locate = surrounding_rectangle.center;
    //int classification;

    if (surrounding_rectangle.size.width < MIN_TAG_CLASSIFICATION_SIZE || surrounding_rectangle.size.height < MIN_TAG_CLASSIFICATION_SIZE)
    {
        //classification = UNKNOWN_TAG;
        //classify_tag = classification;
        classify_tag = 1000;
        return locate;
    }

    else
    {
        //std::cout << "Good3" << std::endl;

        
/*
        long int value = 0;
        for (int row = locate.y - 15; row < locate.y + 15; row++)
        {
            uchar *pix = thresh_shape_frame.ptr<uchar>(row);
            for (int col = locate.x - 15; col < locate.x + 15; col++)
            {
                value += int(pix[col]);
            }
        }

        */
        cv::Rect roi = cv::Rect (locate.x - 10, locate.y - 10, 20, 20);
        cv::Mat tag_area = thresh_shape_frame (roi);
        

        std::vector<std::vector<cv::Point>> contours2;
        std::vector<cv::Vec4i> hierarchy2;
        cv::findContours (tag_area, contours2, hierarchy2, CV_RETR_LIST, CV_CHAIN_APPROX_NONE, cv::Point (0, 0)); // CV_RETR_EXTERNAL
        int largest_area = 0;
        int largest_contour_index;
        //std::cout << contours2.size() << std::endl;
        for (int i = 0; i < contours2.size (); i++) // iterate through each contour. 
        {
            float a = contourArea (contours2[i]);  //  Find the area of contour
            if (a > largest_area)
            {
                largest_area = a;
                largest_contour_index = i;
            }
        }
        if (largest_area > 3)
        {
            cv::RotatedRect surrounding_rectangle2 = cv::minAreaRect (cv::Mat(contours2[largest_contour_index]));
            int found;
            //std::cout << surrounding_rectangle2.size.width << " " << surrounding_rectangle2.size.height << "\n" << std::endl;
            if (surrounding_rectangle2.size.width >= surrounding_rectangle2.size.height)
            {
                classify_tag = surrounding_rectangle2.size.width;
                return locate;
            }
            else 
            {
                classify_tag = surrounding_rectangle2.size.height;
                return locate;
            }
        }
        else
        {
            classify_tag = 1000;
            return locate;
        }
        
    

        //cv::imwrite ("/Users/u5305887/Movies/Output/train_test.jpg", tag_area);
        //std::cout << "writing" << std::endl;
        //classify_tag = cv::countNonZero (tag_area);
        //classify_tag = value;
        /*
        int tag_identified;
        if (value > 10000)
        {
            classification = QUEEN_TAG;
        }
        else if (value < 4500)
        {
            classification = O_TAG;
        }
        else if (value > 7000 && value < 9500)
        {
            classification = I_TAG;
        }
        else
        {
            classification = UNKNOWN_TAG;
        }
        */

    }

}

bool identify_past_location     (std::vector<BeeTag> &allBees,
                                std::vector<cv::Point2f> tags_found_location,
                                std::vector<int> classify,
                                int iterator,
                                int frame_number)
{

    if (allBees.empty ())
    {
        return true;
    }

    cv::Point2f current_tag_contour =  tags_found_location[iterator];
    int tag_best_match_position;
    int frames_since_last_seen;
    float closest_match_distance = 100000;
    bool found_bee_previously = false;

    for (int i = 0; i < allBees.size (); i++)
    {
        cv::Point2f last_location_of_bee = allBees[i].get_last_location ();
        frames_since_last_seen = frame_number - allBees[i].get_last_frame ();
        bool better_match_available = false;
        float closeness_of_tag_to_current_contour = distance_between_tags (current_tag_contour, last_location_of_bee);
        //std::cout << closeness_of_tag_to_current_contour << std::endl;

        if (frames_since_last_seen < FRAMES_BEFORE_EXTINCTION && closeness_of_tag_to_current_contour < closest_match_distance)
        {
            for (int j = 0; j < tags_found_location.size (); j++)
            {
                float closeness_to_other_tag = distance_between_tags (tags_found_location[j], last_location_of_bee);
                if (closeness_to_other_tag < closeness_of_tag_to_current_contour && iterator != j)
                {
                    better_match_available = true;
                    break;
                }
            }
            if (!better_match_available)
            {
                tag_best_match_position = i;
                closest_match_distance = closeness_of_tag_to_current_contour;
                found_bee_previously = true;
            }
            //found_bee_previously = true;
        }


    }

    //int expand_search_radius = (frames_since_last_seen * SEARCH_EXPANSION_BY_FRAME) + SEARCH_EXPANSION_BY_FRAME;
    //found_bee_previously && //  && expand_search_radius > closest_match_distance
    if (found_bee_previously && closest_match_distance < SEARCH_SURROUNDING_AREA)
    {
        allBees[tag_best_match_position].update_location_frame_classification (tags_found_location[iterator], frame_number, classify[iterator]);
        return false;
    }
    else
    {
        return true;
    }

}

float distance_between_tags     (cv::Point2f first_tag,
                                cv::Point2f second_tag)
{
    float distance = sqrt (pow ((first_tag.x - second_tag.x), 2) + pow ((first_tag.y - second_tag.y), 2));
    return distance;
}

cv::Mat draw_circles            (cv::Mat original_frame,
                                std::vector<BeeTag> allBees,
                                int frame_number)
{
    for (int i = 0; i < allBees.size (); i++)
    {
        int frames_since_last_seen = frame_number - allBees[i].get_last_frame ();
        if( frames_since_last_seen < FRAMES_BEFORE_EXTINCTION) 
        {
            cv::putText (original_frame, std::to_string(allBees[i].get_tag_type ()), //get_id ()
                         cv::Point(allBees[i].get_last_location ().x + 20, allBees[i].get_last_location ().y + 20), CV_FONT_HERSHEY_COMPLEX, 2, 
                         255, 5, 8);
        }
    }
    return original_frame;
}

void write_csv                  (std::vector<BeeTag> &allBees,
                                int all_frames,
                                char *csv_argument)
{
    std::ofstream output_csv;
    output_csv.open (csv_argument, std::ios_base::app);

    for (int i = 0; i < allBees.size (); i++)
    {

        std::vector<cv::Point2f> every_location_of_bee = allBees[i].get_all_locations ();
        std::vector<int> all_frames_bee_present = allBees[i].get_all_frames ();
        std::vector<int> all_tags_classified = allBees[i].get_all_tags ();

        for (int j = 0; j < all_frames_bee_present.size (); j++)
        {
            output_csv << "\n" << allBees[i].get_id () << "," << all_tags_classified[j] << "," << all_frames_bee_present[j] << "," <<every_location_of_bee[j].x << "," << every_location_of_bee[j].y;
        }

    }

    output_csv.close ();

    allBees.erase (std::remove_if (allBees.begin (), allBees.end (), [all_frames](BeeTag bee)->bool
        {
            int frames_since_last_seen = all_frames - bee.get_last_frame ();
            if (frames_since_last_seen > FRAMES_BEFORE_EXTINCTION)
            {
                return true;
            }
            else
            {
                bee.clear_stored_objects();
                return false;
            }
        }
    ), allBees.end () );
}