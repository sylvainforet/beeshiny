/**
 * The bee tracker
 */

#ifndef __BEE_TRACKER_H__
#define __BEE_TRACKER_H__

#include <fstream>
#include <string>

#include <opencv2/highgui/highgui.hpp>

#include "BeeTag.h"
#include "VideoReader.h"


#define VERSION             1.1


class BeeTracker
{
    public:
        BeeTracker                                  (const std::string &input_video,
                                                     const std::string &output_file,
                                                     unsigned int threads,
                                                     unsigned int frames_per_thread);

        const std::string  &get_input_path          (void) const;

        const std::string  &get_output_path         (void) const;

        int                 track_bees              (void);

        unsigned int        get_n_threads           (void) const;

    private:

        struct SegmentationData
        {
            SegmentationData (unsigned int frames_per_thread) :
                frames (frames_per_thread),
                locations (frames_per_thread),
                classifications (frames_per_thread) {}

            std::vector<cv::Mat>                  frames;
            std::vector<std::vector<cv::Point2f>> locations;
            std::vector<std::vector<int>>         classifications;
            unsigned int n_frames = 0;
        };

        void                write_csv_header        (void);

        void                write_csv_chunk         (void);

        bool                load_frames             (void);

        void                segment_frames          (void);

        void                track_frames            (void);

        void                find_countours          (unsigned int thread_id);

        cv::Point2f         classify_locate_bee     (const std::vector<cv::Point> &each_contour,
                                                     const cv::Mat &thresh_shape_frame,
                                                     int &classify_tag);

        bool                identify_past_location  (std::vector<cv::Point2f> &tag_locations,
                                                     std::vector<int> &tag_classifications,
                                                     size_t iterator,
                                                     size_t frame_number);

        // Options
        std::string         input_path = "";
        std::string         output_path = "";
        unsigned int        n_threads = 1;
        unsigned int        frames_per_thread = 10;
        // Frame loading stuff
        VideoReader         video_reader;
        std::ofstream       output_stream;
        int                 frame_count = 0;
        // Currently tracked bees
        std::vector<BeeTag> all_bees;
        int                 bee_ids = 0;

        // Segmentation
        std::vector<SegmentationData> segmentation_data;
};

#endif /* __BEE_TRACKER_H__ */

/* vim:ts=4:sw=4:sts=4:expandtab:cinoptions=(0:
 */
