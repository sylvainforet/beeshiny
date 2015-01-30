/**
 * JS 2014
 * This does this and that ...
 */

#include <time.h>
#include <pthread.h>
#include <sys/time.h>

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "BeeTag.h"

#define VERSION                     1.1
#define UNKNOWN_TAG                 0
#define O_TAG                       1
#define I_TAG                       2
#define QUEEN_TAG                   3
#define MIN_CONTOUR_AREA            60
#define MORPH_TRANSFORM_SIZE        15
#define FRAMES_BEFORE_CLEAR_MEMORY  20000
#define FRAMES_BEFORE_EXTINCTION    500
#define SEARCH_SURROUNDING_AREA     500
#define SEARCH_EXPANSION_BY_FRAME   50
#define MIN_TAG_CLASSIFICATION_SIZE 22
#define MIN_CLOSENESS_BEFORE_DELETE 40

#define N_THREADS                   4 //// FIXME This should be a command line option


// Data structure to pass data to individual threads

struct FindCountoursArgs
{
    cv::Mat frame;
    std::vector<cv::Point2f> locations;
    std::vector<int> classifications;
};


static cv::Point2f classify_locate_bee  (const std::vector<cv::Point> &each_contour,
                                         const cv::Mat &thresh_shape_frame,
                                         int &classify_tag);

static bool identify_past_location      (std::vector<BeeTag> &all_bees,
                                         std::vector<cv::Point2f> tag_locations,
                                         std::vector<int> tag_classifications,
                                         int iterator,
                                         int frame_number);

static float distance_between_tags      (const cv::Point2f &first_tag,
                                         const cv::Point2f &second_tag);

static void write_csv_header            (const char *csv,
                                         const char *input);

static void write_csv_chunk             (const char *csv,
                                         std::vector<BeeTag> &all_bees,
                                         int frame_count);

static void* find_countours             (void *p);

int
main (int argc, char *argv[])
{
    if (argc != 4)
    {
        std::cerr << "Usage: beeshiny VIDEO_IN VIDEO_OUT CSV_OUT" << std::endl;
        //// Why output video?
        //// Also, could use a proper command line processing system at some stage in the future ...
        return -1;
    }

    // Open the input video
    cv::VideoCapture cap (argv[1]);
    if (!cap.isOpened ())
    {
        std::cerr << "Couldn't open the video" << std::endl;
        return -1;
    }

    write_csv_header (argv[3], argv[2]);

    int frame_count = 0;
    int bee_identities = 0;
    std::vector<BeeTag> all_bees;
    std::vector<FindCountoursArgs> args (N_THREADS);

    // Threads configuration
    std::vector<pthread_t> threads (N_THREADS);
    pthread_attr_t thread_attr;
    pthread_attr_init(&thread_attr);
    pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_JOINABLE);

    struct timeval t0s;
    struct timeval t0e;
    struct timeval t0d;
    struct timeval t0t = {0, 0};

    struct timeval t1s;
    struct timeval t1e;
    struct timeval t1d;
    struct timeval t1t = {0, 0};

    struct timeval t2s;
    struct timeval t2e;
    struct timeval t2d;
    struct timeval t2t = {0, 0};

    while (1)
    {
        // Read frames
        bool succeed = false;
        int frames_read = 0;

        gettimeofday (&t0s, NULL);
        for (auto &arg : args)
        {
            succeed = cap.read (arg.frame);
            if (!succeed)
            {
                break;
            }
            frames_read++;
        }
        if (!succeed)
        {
            std::cerr << "Exiting at " << frame_count + frames_read << std::endl;
            break;
        }
        gettimeofday (&t0e, NULL);
        timersub (&t0e, &t0s, &t0d);
        timeradd (&t0d, &t0t, &t0t);

        gettimeofday (&t1s, NULL);
        // Launch the threads to find the tags in each frame
        for (int i = 0; i < frames_read; i++)
        {
            if (pthread_create (threads.data() + i,
                                &thread_attr,
                                find_countours,
                                (void*) (args.data() + i)))
            {
                std::cerr << "Failed to create thread " << i << std::endl;
                exit (1);
            }
        }
        // Wait to threads to complete
        for (auto &thread : threads)
        {
            pthread_join (thread, NULL);
        }
        gettimeofday (&t1e, NULL);
        timersub (&t1e, &t1s, &t1d);
        timeradd (&t1d, &t1t, &t1t);

        gettimeofday (&t2s, NULL);
        // Track bees accross frames
        for (int i = 0; i < frames_read; i++)
        {
            std::vector<cv::Point2f> &tag_locations = args[i].locations;
            std::vector<int> &tag_classifications = args[i].classifications;

            for (int j = 0; j < tag_locations.size (); j++)
            {
                bool new_bee_found = identify_past_location (all_bees,
                                                             tag_locations,
                                                             tag_classifications,
                                                             j, frame_count);
                if (new_bee_found)
                {
                    BeeTag new_bee (bee_identities,
                                    tag_locations[j],
                                    frame_count,
                                    tag_classifications[j]);
                    all_bees.push_back (new_bee);
                    bee_identities++;
                }
            }

            // Done for this frame, clear the vectors
            tag_locations.clear ();
            tag_classifications.clear ();

            //// This could be useful information but it should go to a log file and
            //// not be printed on every  iterations
            //// only if verbose mode or debug mode
            std::cerr << frame_count << " " << all_bees.size () << std::endl;
            frame_count++;

            // Write results
            if (frame_count % FRAMES_BEFORE_CLEAR_MEMORY == 0)
            {
                write_csv_chunk (argv[3], all_bees, frame_count);
            }
        }
        gettimeofday (&t2e, NULL);
        timersub (&t2e, &t2s, &t2d);
        timeradd (&t2d, &t2t, &t2t);
    }

    std::cout << "Time spent in reading     : " << t0t.tv_sec << "s " << t0t.tv_usec << "us" << std::endl;
    std::cout << "Time spent in segmentation: " << t1t.tv_sec << "s " << t1t.tv_usec << "us" << std::endl;
    std::cout << "Time spent in tracking    : " << t2t.tv_sec << "s " << t2t.tv_usec << "us" << std::endl;

    // Write results
    write_csv_chunk (argv[3], all_bees, frame_count);

    // Close the movie
    cap.release ();

    return 0;
}


static cv::Point2f
classify_locate_bee (const std::vector<cv::Point> &each_contour,
                     const cv::Mat &thresh_shape_frame,
                     int &classify_tag)

{
    cv::RotatedRect surrounding_rectangle = cv::minAreaRect (cv::Mat(each_contour));
    cv::Point2f locate = surrounding_rectangle.center;

    if (surrounding_rectangle.size.width < MIN_TAG_CLASSIFICATION_SIZE ||
        surrounding_rectangle.size.height < MIN_TAG_CLASSIFICATION_SIZE)
    {
        classify_tag = UNKNOWN_TAG;
        return locate;
    }

    else
    {
        if (locate.x - 10 < 0 ||
            locate.x + 10 > thresh_shape_frame.cols ||
            locate.y - 10 < 0 ||
            locate.y + 10 > thresh_shape_frame.rows)
        {
            classify_tag = UNKNOWN_TAG;
            return locate;
        }
        else
        {
            cv::Rect roi = cv::Rect (locate.x - 10, locate.y - 10, 20, 20);
            cv::Mat tag_area = thresh_shape_frame (roi);
            std::vector<std::vector<cv::Point>> contours2;
            std::vector<cv::Vec4i> hierarchy2;
            cv::findContours (tag_area, contours2, hierarchy2,
                              CV_RETR_LIST, CV_CHAIN_APPROX_NONE,
                              cv::Point (0, 0)); // CV_RETR_EXTERNAL
            int largest_area = 0;
            int largest_contour_index;
            for (int i = 0; i < contours2.size (); i++)
            {
                //  Find the area of contour
                float a = contourArea (contours2[i]);
                if (a > largest_area)
                {
                    largest_area = a;
                    largest_contour_index = i;
                }
            }
            if (largest_area > 1)
            {
                cv::RotatedRect surrounding_rectangle2 = cv::minAreaRect (cv::Mat(contours2[largest_contour_index]));
                if (surrounding_rectangle2.size.width >= surrounding_rectangle2.size.height)
                {
                    if (surrounding_rectangle2.size.width < 10)
                    {
                        classify_tag = O_TAG;
                        return locate;
                    }
                    else if (surrounding_rectangle2.size.width > 14 &&
                             surrounding_rectangle2.size.width < 34)
                    {
                        classify_tag = I_TAG;
                        return locate;
                    }
                    else
                    {
                        classify_tag = UNKNOWN_TAG;
                        return locate;
                    }
                }
                else 
                {
                    if (surrounding_rectangle2.size.height < 10)
                    {
                        classify_tag = O_TAG;
                        return locate;
                    }
                    else if (surrounding_rectangle2.size.height > 14 &&
                             surrounding_rectangle2.size.height < 34)
                    {
                        classify_tag = I_TAG;
                        return locate;
                    }
                    else
                    {
                        classify_tag = UNKNOWN_TAG;
                        return locate;
                    }
                }
            }
            else
            {
                classify_tag = QUEEN_TAG;
                return locate;
            }
        }
    }

}

static bool
identify_past_location (std::vector<BeeTag> &all_bees,
                        std::vector<cv::Point2f> tag_locations,
                        std::vector<int> tag_classifications,
                        int iterator,
                        int frame_number)
{
    if (all_bees.empty ())
    {
        return true;
    }

    cv::Point2f current_tag_contour = tag_locations[iterator];
    int tag_best_match_position;
    int best_match_frames_since_last_seen = 1000;
    float closest_match_distance = 100000;
    bool found_bee_previously = false;
    bool have_to_delete_bee = false;

    for (int i = 0; i < all_bees.size (); i++)
    {
        cv::Point2f last_location_of_bee = all_bees[i].get_last_location ();
        int frames_since_last_seen = frame_number - all_bees[i].get_last_frame ();
        bool better_match_available = false;
        bool bee_too_close_to_other = false;
        float closeness_of_tag_to_current_contour = distance_between_tags (current_tag_contour,
                                                                           last_location_of_bee);

        if (closeness_of_tag_to_current_contour < SEARCH_SURROUNDING_AREA &&
            frames_since_last_seen < FRAMES_BEFORE_EXTINCTION &&
            !all_bees[i].is_deleted ())
        {
            for (int j = 0; j < tag_locations.size (); j++)
            {
                if (iterator !=j)
                {
                    float closeness_to_other_tag = distance_between_tags (tag_locations[j],
                                                                          last_location_of_bee);
                    if (closeness_to_other_tag < closeness_of_tag_to_current_contour)
                    {
                        better_match_available = true;
                        break;
                    }
                    else if (closeness_to_other_tag < MIN_CLOSENESS_BEFORE_DELETE)
                    {
                        bee_too_close_to_other = true;
                    }
                }
                
            }
            if (!better_match_available &&
                closeness_of_tag_to_current_contour < closest_match_distance)
            {
                tag_best_match_position = i;
                closest_match_distance = closeness_of_tag_to_current_contour;
                best_match_frames_since_last_seen = frames_since_last_seen;
                found_bee_previously = true;
                if (bee_too_close_to_other)
                {
                    have_to_delete_bee = true;
                    bee_too_close_to_other = false;
                }
                else
                {
                    have_to_delete_bee = false;
                }
            }
        }
    }

    int expand_search_radius = (best_match_frames_since_last_seen * SEARCH_EXPANSION_BY_FRAME) + SEARCH_EXPANSION_BY_FRAME;
    if (found_bee_previously && expand_search_radius > closest_match_distance)
    {
        all_bees[tag_best_match_position].add_point (tag_locations[iterator],
                                                     frame_number,
                                                     tag_classifications[iterator]);
        if (have_to_delete_bee)
        {
            all_bees[tag_best_match_position].delete_bee ();
        }
        return false;
    }
    return true;
}

static float
distance_between_tags (const cv::Point2f &first_tag,
                       const cv::Point2f &second_tag)
{
    const float delta_x = first_tag.x - second_tag.x;
    const float delta_y = first_tag.y - second_tag.y;
    return sqrt (pow (delta_x, 2) + pow (delta_y, 2));
}


static void*
find_countours (void *p)
{
    FindCountoursArgs* args = (FindCountoursArgs*)p;
    cv::Mat gray_frame;
    cv::Mat smooth_frame;
    cv::Mat thresh_frame;
    cv::Mat thresh_shape;
    cv::Mat close_element = cv::getStructuringElement (cv::MORPH_RECT,
                                                       cv::Size (MORPH_TRANSFORM_SIZE,
                                                                 MORPH_TRANSFORM_SIZE)); //ELLIPSE

    // Tag segmentation: colour to gray conversion, smoothing,closing and blocking reflection and thresholding
    cv::cvtColor (args->frame, gray_frame, CV_BGR2GRAY);
    cv::blur (gray_frame, smooth_frame, cv::Size (3, 3));
    cv::threshold (smooth_frame, thresh_frame, 90, 255, CV_THRESH_BINARY);
    cv::morphologyEx (thresh_frame, thresh_frame, cv::MORPH_CLOSE, close_element);
    cv::threshold (gray_frame, thresh_shape, 150, 255, CV_THRESH_BINARY_INV);

    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> hierarchy;

    cv::findContours (thresh_frame, contours, hierarchy,
                      CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE,
                      cv::Point (0, 0)); // CV_RETR_LIST

    for (auto &contour : contours)
    {
        if (cv::contourArea (contour) > MIN_CONTOUR_AREA)
        {
            int classified;

            cv::Point2f located = classify_locate_bee (contour, thresh_shape, classified);
            args->locations.push_back (located);
            args->classifications.push_back (classified);
        }
    }
    return NULL;
}

static void
write_csv_header (const char *csv,
                  const char *input)
{
    std::ofstream output_csv;
    output_csv.open (csv);
    time_t _tm =time(NULL );
    struct tm * curtime = localtime ( &_tm );

    // Metadata as comment
    output_csv
        << "# Version: "
        << VERSION
        << " File: "
        << input
        << " Video date: "
        << "XXX" //// NEED to extract this from the video
        << " Processing date: "
        << asctime (curtime)
        << std::endl;
    // Header
    output_csv
        << "BeeID,"
        << "Tag,"
        << "Frame,"
        << "X,"
        << "Y"
        << std::endl;
    output_csv.close ();
}

static void
write_csv_chunk (const char *csv,
                 std::vector<BeeTag> &all_bees,
                 int frame_count)
{
    std::ofstream output_csv;
    output_csv.open (csv, std::ios_base::app);

    for (int i = 0; i < all_bees.size (); i++)
    {

        std::vector<cv::Point2f> every_location_of_bee = all_bees[i].get_locations ();
        std::vector<int> all_frames_bee_present = all_bees[i].get_frames ();
        std::vector<int> all_tags_classified = all_bees[i].get_tags ();

        for (int j = 0; j < all_frames_bee_present.size (); j++)
        {
            output_csv
                << all_bees[i].get_id ()
                << ","
                << all_tags_classified[j]
                << ","
                << all_frames_bee_present[j]
                << ","
                << every_location_of_bee[j].x
                << ","
                << every_location_of_bee[j].y
                << std::endl;
        }

    }

    output_csv.close ();

    all_bees.erase (std::remove_if (all_bees.begin (), all_bees.end (), [frame_count](BeeTag &bee) -> bool
        {
            int frames_since_last_seen = frame_count - bee.get_last_frame ();

            if (frames_since_last_seen > FRAMES_BEFORE_EXTINCTION)
            {
                return true;
            }
            else
            {
                bee.clear ();
                return false;
            }
        }
    ), all_bees.end ());
}

/* vim:ts=4:sw=4:sts=4:expandtab:
 */
