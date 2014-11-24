/**
 * JS 2014
 * This does this and that ...
 */

#include "BeeTag.h"

#define UNKNOWN_TAG                 0
#define L_TAG                       1
#define STAR_TAG                    2
#define QUEEN_TAG                   3
#define FRAMES_BEFORE_CLEAR_MEMORY  20000
#define FRAMES_BEFORE_EXTINCTION    250
#define SEARCH_SURROUNDING_AREA     200

cv::Point2f classify_locate_bee     (std::vector<cv::Point> each_contour,
                                    cv::Mat grayscale_frame,
                                    int &classify_tag);

bool identify_past_location         (std::vector<BeeTag> &allBees,
                                    cv::Point2f *tags_found[],
                                    int classify[],
                                    int iterator,
                                    int number_of_contours,
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

    cv::VideoCapture cap (argv[1]);
    cv::VideoWriter output_cap (argv[2], CV_FOURCC ('X', 'V', 'I', 'D'), 25, cv::Size (3840, 2160), true);
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
    cv::RNG rng (12345);
    std::vector<BeeTag> allBees;

    while(1)
    {
        cv::Mat frame, gray_frame, smoothed_frame, thresh_frame, output_frame;
        
        bool succeed = cap.read (frame);
        if(!succeed) 
        {
            std::cout << "Exiting at " << frame_count << std::endl;
            break;
        }

        // Tag segmentation: colour to gray conversion, smoothing,closing and blocking reflection and thresholding
        cv::cvtColor (frame, gray_frame, CV_BGR2GRAY);
        cv::GaussianBlur (gray_frame, smoothed_frame, cv::Size (3, 3), 1, 1);

        cv::threshold (smoothed_frame, thresh_frame, 90, 255, CV_THRESH_BINARY);
        cv::Mat close_element = cv::getStructuringElement (cv::MORPH_ELLIPSE, cv::Size (11, 11));
        cv::morphologyEx (thresh_frame, thresh_frame, cv::MORPH_CLOSE, close_element);

        std::vector<std::vector<cv::Point>> contours;
        std::vector<cv::Vec4i> hierarchy;
        cv::findContours (thresh_frame, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE, cv::Point (0, 0)); // CV_RETR_LIST

        cv::Point2f *tag_locations[contours.size ()];
        int tag_classifications[contours.size ()];
        for (int i = 0; i < contours.size (); i++)
        {
            if (cv::contourArea (contours[i]) > 4)
            {
                cv::Point2f new_location = classify_locate_bee (contours[i], gray_frame, tag_classifications[i]);
                tag_locations[i] = new cv::Point2f(new_location);
            }
            else
            {
                tag_locations[i] = nullptr;
                tag_classifications[i] = UNKNOWN_TAG;
            }

        }

        bool new_bee_found[contours.size ()];

        for (int i = 0; i < contours.size (); i++)
        {
            if (tag_locations[i] != nullptr)
            {
                new_bee_found[i] = identify_past_location (allBees, tag_locations, tag_classifications, i, contours.size (), frame_count);

            }
            else
            {
                new_bee_found[i] = false;
            }
        }

        for (int i = 0; i < contours.size (); i++)
        {
            if (new_bee_found[i] == true)
            {
                cv::Scalar assign_colour = cv::Scalar (rng.uniform (0, 255), rng.uniform (0, 255) , rng.uniform (0, 255));
                BeeTag newBee (bee_identities, *tag_locations[i], frame_count, assign_colour, tag_classifications[i]);
                allBees.push_back (newBee);
                bee_identities++;
                delete tag_locations[i];
            }
        }

        std::cout << frame_count << " " << allBees.size () << std::endl;

        output_frame = draw_circles (frame, allBees, frame_count);
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
                                cv::Mat grayscale_frame,
                                int &classify_tag)

{
    cv::RotatedRect surrounding_rectangle = cv::minAreaRect (cv::Mat(each_contour));
    cv::Point2f locate = surrounding_rectangle.center;
    int classification;

    if (surrounding_rectangle.size.width < 8 || surrounding_rectangle.size.height < 8)
    {
        classification = UNKNOWN_TAG;
        classify_tag = classification;
        return locate;
    }

    else
    {
        long int value = 0;
        for (int row = locate.y - 4; row < locate.y + 4; row++)
        {
            uchar *pix = grayscale_frame.ptr<uchar>(row);
            for (int col = locate.x - 4; col < locate.x + 4; col++)
            {
                value += int(pix[col]);
            }
        }
        
        int tag_identified;
        if (value> 14000)
        {
            classification = QUEEN_TAG;
        }
        else if (value < 6000)
        {
            classification = STAR_TAG;
        }
        else if (value > 7000 && value < 9500)
        {
            classification = L_TAG;
        }
        else
        {
            classification = UNKNOWN_TAG;
        }
        classify_tag = classification;
        return locate;

    }

}

bool identify_past_location     (std::vector<BeeTag> &allBees,
                                cv::Point2f *tags_found[],
                                int classify[],
                                int iterator,
                                int number_of_contours,
                                int frame_number)

{

    if (allBees.empty ())
    {
        return true;
        
    }

    int tag_number;
    int frames_since_last_seen;
    double closest_match  = 10000000.0;

    for (int i = 0; i < allBees.size (); i++)
    {
        
        cv::Point2f last_location_of_bee = allBees[i].get_last_location ();
        frames_since_last_seen = frame_number - allBees[i].get_last_frame ();
        bool better_match_available = false;
        double closeness_of_tag = distance_between_tags (*tags_found[iterator], last_location_of_bee);
        if (frames_since_last_seen < FRAMES_BEFORE_EXTINCTION && closeness_of_tag <= closest_match)
        {
            for (int j = 0; j < number_of_contours; j++)
            {
                if (tags_found[j] != nullptr)
                {
                    cv::Point2f alternate_location =  *tags_found[j];
                    float closeness_to_other_tag = distance_between_tags (alternate_location, last_location_of_bee);
                    if (*tags_found[iterator] != alternate_location && closeness_to_other_tag < closeness_of_tag)
                    {
                        better_match_available = true;
                        break;
                    }
                }

                if (!better_match_available)
                {
                    tag_number = i;
                    closest_match = closeness_of_tag;
                }

            }

        }

    }

    int search_radius_size = SEARCH_SURROUNDING_AREA;
    if (closest_match < search_radius_size)
    {
        allBees[tag_number].update_location_frame_classification (*tags_found[iterator], frame_number, classify[iterator]);
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
            cv::circle (original_frame, allBees[i].get_last_location(), 16, 
                        allBees[i].get_circle_colour (), 3, 8, 0);
            cv::putText (original_frame, std::to_string(allBees[i].get_tag_type ()), 
                        allBees[i].get_last_location (), CV_FONT_HERSHEY_COMPLEX, 2, 
                        cv::Scalar(255, 255, 255), 5, 8);
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