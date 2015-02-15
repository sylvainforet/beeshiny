/**
 *
 */

#include <iostream>
#include <thread>

#include <math.h>
#include <sys/time.h>
#include <time.h>

#include <boost/timer/timer.hpp>

#include <opencv2/imgproc/imgproc.hpp>

#include "BeeTracker.h"

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

#define MAX_THREADS                 64
#define MAX_FRAMES_PER_THREAD       128


///////////////
// Utilities //
///////////////


static float euclidian_distance (const cv::Point2f &first_tag,
                                 const cv::Point2f &second_tag);

static float
euclidian_distance (const cv::Point2f &first_tag,
                    const cv::Point2f &second_tag)
{
    const float delta_x = first_tag.x - second_tag.x;
    const float delta_y = first_tag.y - second_tag.y;
    return sqrt (pow (delta_x, 2) + pow (delta_y, 2));
}

////////////////
// BeeTracker //
////////////////

BeeTracker::BeeTracker (const std::string &input_video,
                        const std::string &output_file,
                        unsigned int threads,
                        unsigned int frames) :
    input_path (input_video),
    output_path (output_file),
    n_threads (threads),
    frames_per_thread (frames)
{
    if (n_threads > 1)
    {
        if (n_threads > MAX_THREADS)
        {
            std::cerr
                << "More than "
                << MAX_THREADS
                << " have been requested ("
                << n_threads
                << ") this is probably not intended, exiting."
                << std::endl;
            exit (1);
        }
    }
    video_reader.set_thread_count (n_threads > 0 ? n_threads : 1);
    if (frames_per_thread < 1)
    {
        std::cerr
            << "Need at least one frame per thread ("
            << frames_per_thread
            << " provided)"
            << std::endl;
        exit (1);
    }
    if (frames_per_thread > MAX_FRAMES_PER_THREAD)
    {
        std::cerr
            << "More than "
            << MAX_FRAMES_PER_THREAD
            << " have been requested ("
            << frames_per_thread
            << ") this is probably not intended, exiting."
            << std::endl;
        exit (1);
    }
}

const std::string&
BeeTracker::get_input_path (void) const
{
    return input_path;
}

const std::string&
BeeTracker::get_output_path (void) const
{
    return output_path;
}

unsigned int
BeeTracker::get_n_threads (void) const
{
    return n_threads;
}

int
BeeTracker::track_bees (void)
{
    // Open the input video
    if (!video_reader.open (input_path))
    {
        std::cerr << "Couldn't open the video" << std::endl;
        return -1;
    }
    write_csv_header ();

    segmentation_data.push_back (SegmentationData (frames_per_thread));
    for (unsigned int i = 1; i < n_threads; i++)
    {
        segmentation_data.push_back (SegmentationData (frames_per_thread));
    }

    frame_count = 0;

    boost::timer::cpu_timer timer1; timer1.stop();
    boost::timer::cpu_timer timer2; timer2.stop();
    boost::timer::cpu_timer timer3; timer3.stop();

    while (true)
    {
        // Read frames
        timer1.resume ();
        bool keep_loading = load_frames ();
        timer1.stop ();

        timer2.resume ();
        segment_frames ();
        timer2.stop ();

        timer3.resume ();
        track_frames ();
        timer3.stop ();
        if (!keep_loading)
        {
            break;
        }
    }

    std::cout << "Time spent in reading     : " << timer1.format ();
    std::cout << "Time spent in segmentation: " << timer2.format ();
    std::cout << "Time spent in tracking    : " << timer3.format ();
    // Write results
    write_csv_chunk ();
    // Close the movie
    video_reader.close ();

    return 0;
}

bool
BeeTracker::load_frames (void)
{
    bool success = true;
    unsigned int frames_read = 0;

    for (auto &data : segmentation_data)
    {
        data.n_frames = 0;
        if (!success)
        {
            continue;
        }
        for (unsigned int i = 0; i < frames_per_thread; i++)
        {
            success = video_reader.read (data.frames[i]);
            if (!success)
            {
                break;
            }
            data.n_frames++;
            frames_read++;
        }
    }
    return success;
}

void
BeeTracker::segment_frames (void)
{
    if (n_threads == 0)
    {
        find_countours (0);
    }
    else
    {
        // Launch the threads to find the tags in each frame
        std::vector<std::thread> threads;
        for (unsigned int i = 0; i < n_threads; i++)
        {
            threads.push_back (std::thread (&BeeTracker::find_countours,
                                            this, i));
        }
        // Wait for threads to complete
        for (auto &thread : threads)
        {
            thread.join ();
        }
    }
}

void
BeeTracker::track_frames (void)
{
        // Track bees accross frames
        for (auto &data : segmentation_data)
        {
            if (data.n_frames == 0)
            {
                break;
            }
            for (size_t i = 0; i < data.n_frames; i++)
            {
                std::vector<cv::Point2f> &tag_locations = data.locations[i];
                std::vector<int> &tag_classifications = data.classifications[i];

                for (size_t j = 0; j < tag_locations.size (); j++)
                {
                    bool new_bee_found = identify_past_location (tag_locations,
                                                                 tag_classifications,
                                                                 j, frame_count);
                    if (new_bee_found)
                    {
                        BeeTag new_bee (bee_ids,
                                        tag_locations[j],
                                        frame_count,
                                        tag_classifications[j]);
                        all_bees.push_back (new_bee);
                        bee_ids++;
                    }
                }

                // Done for this frame, clear the vectors
                tag_locations.clear ();
                tag_classifications.clear ();

                //// This could be useful information but it should go to a log file and
                //// not be printed on every  iterations
                //// only if verbose mode or debug mode
                //std::cerr << frame_count << " " << all_bees.size () << std::endl;
                frame_count++;

                // Write results
                if (frame_count % FRAMES_BEFORE_CLEAR_MEMORY == 0)
                {
                    write_csv_chunk ();
                }
            }
        }
}

void
BeeTracker::find_countours (unsigned int thread_id)
{
    cv::Mat gray_frame;
    cv::Mat smooth_frame;
    cv::Mat thresh_frame;
    cv::Mat thresh_shape;
    cv::Mat close_element = cv::getStructuringElement (cv::MORPH_RECT,
                                                       cv::Size (MORPH_TRANSFORM_SIZE,
                                                                 MORPH_TRANSFORM_SIZE)); //ELLIPSE

    SegmentationData &data = segmentation_data[thread_id];
    // Tag segmentation: colour to gray conversion, smoothing,closing and blocking reflection and thresholding
    for (size_t i = 0; i < data.n_frames; i++)
    {
        cv::cvtColor (data.frames[i], gray_frame, CV_BGR2GRAY);
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
                data.locations[i].push_back (located);
                data.classifications[i].push_back (classified);
            }
        }
    }
}

void
BeeTracker::write_csv_header (void)
{
    time_t _tm = time (NULL );
    struct tm * curtime = localtime ( &_tm );

    output_stream.open (output_path);
    // Metadata as comment
    output_stream
        << "# Version: "
        << VERSION
        << " File: "
        << input_path
        << " Video date: "
        << "XXX" //// NEED to extract this from the video
        << " Processing date: "
        << asctime (curtime)
    // Header
        << "BeeID,"
        << "Tag,"
        << "Frame,"
        << "X,"
        << "Y"
        << std::endl;
}

void
BeeTracker::write_csv_chunk (void)
{
    for (size_t i = 0; i < all_bees.size (); i++)
    {

        std::vector<cv::Point2f> every_location_of_bee = all_bees[i].get_locations ();
        std::vector<int> all_frames_bee_present = all_bees[i].get_frames ();
        std::vector<int> all_tags_classified = all_bees[i].get_tags ();

        for (size_t j = 0; j < all_frames_bee_present.size (); j++)
        {
            output_stream
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

    // TODO Move this code to another function?
    int count = frame_count;
    all_bees.erase (std::remove_if (all_bees.begin (), all_bees.end (), [count](BeeTag &bee) -> bool
        {
            int frames_since_last_seen = count - bee.get_last_frame ();

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

cv::Point2f
BeeTracker::classify_locate_bee (const std::vector<cv::Point> &each_contour,
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
            for (size_t i = 0; i < contours2.size (); i++)
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

bool
BeeTracker::identify_past_location (std::vector<cv::Point2f> &tag_locations,
                                    std::vector<int> &tag_classifications,
                                    size_t iterator,
                                    size_t frame_number)
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

    for (size_t i = 0; i < all_bees.size (); i++)
    {
        cv::Point2f last_location_of_bee = all_bees[i].get_last_location ();
        int frames_since_last_seen = frame_number - all_bees[i].get_last_frame ();
        bool better_match_available = false;
        bool bee_too_close_to_other = false;
        float closeness_of_tag_to_current_contour = euclidian_distance (current_tag_contour,
                                                                        last_location_of_bee);

        if (closeness_of_tag_to_current_contour < SEARCH_SURROUNDING_AREA &&
            frames_since_last_seen < FRAMES_BEFORE_EXTINCTION &&
            !all_bees[i].is_deleted ())
        {
            for (size_t j = 0; j < tag_locations.size (); j++)
            {
                if (iterator != j)
                {
                    float closeness_to_other_tag = euclidian_distance (tag_locations[j],
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

/* vim:ts=4:sw=4:sts=4:expandtab:cinoptions=(0:
 */
