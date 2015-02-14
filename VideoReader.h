/**
 * Video reading
 */

#ifndef __VIDEO_READER_H__
#define __VIDEO_READER_H__

#include <string>

#include <opencv2/core/core.hpp>

struct CvCapture_FFMPEG;

class VideoReader
{
    public:
        VideoReader             (void);

        ~VideoReader            (void);

        bool open               (const std::string &file);

        void close              (void);

        void set_thread_count   (unsigned int thread_count);

        bool read               (cv::OutputArray image);

    private:
        CvCapture_FFMPEG *ffmpeg;
        bool opened = false;
};

#endif /* __VIDEO_READER_H__ */

/* vim:ts=4:sw=4:sts=4:expandtab:cinoptions=(0
 */
