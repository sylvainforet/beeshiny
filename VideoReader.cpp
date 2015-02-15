/**
 *
 */

#include <iostream>

#include <opencv2/core/core_c.h>

#define __STDC_CONSTANT_MACROS

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
}

#include "VideoReader.h"

/**
 * CvCapture_FFMPEG, adapted from opencv code
 */

#if defined (__APPLE__)
#define AV_NOPTS_VALUE_ ((int64_t)0x8000000000000000LL)
#else
#define AV_NOPTS_VALUE_ ((int64_t)AV_NOPTS_VALUE)
#endif

#define MAX_ATTEMPTS (1 << 16)

static bool _initialized = false;

class InternalFFMpegRegister
{
public:
    InternalFFMpegRegister ()
    {
        if (!_initialized)
        {
            //avformat_network_init ();
            /* register all codecs, demux and protocols */
            av_register_all ();
            //av_log_set_level (AV_LOG_ERROR);
            _initialized = true;
        }
    }

    ~InternalFFMpegRegister ()
    {
        _initialized = false;
    }
};

static InternalFFMpegRegister _init;


struct Image_FFMPEG
{
    unsigned char  *data;
    int             step;
    int             width;
    int             height;
    int             cn;
};

struct CvCapture_FFMPEG
{
    bool open           (const char *filename);
    void close          (void);
    void init           (void);

    bool grab_frame     (void);
    bool retrieve_frame (cv::Mat &mat);

    AVFormatContext    *ic;
    AVCodec            *avcodec;
    int                 video_stream;
    unsigned int        thread_count;
    AVStream           *video_st;
    AVFrame            *picture;
    AVFrame             rgb_picture;
    AVPacket            packet;
    Image_FFMPEG        frame;
    struct SwsContext  *img_convert_ctx;
    int64_t             frame_number;
};

void
CvCapture_FFMPEG::init (void)
{
    ic = 0;
    video_stream = -1;
    video_st = 0;
    picture = 0;
    img_convert_ctx = 0;
    avcodec = 0;
    frame_number = 0;

    memset (&rgb_picture, 0, sizeof (rgb_picture));
    memset (&frame, 0, sizeof (frame));
    memset (&packet, 0, sizeof (packet));
    av_init_packet (&packet);
}

void
CvCapture_FFMPEG::close (void)
{
    if (img_convert_ctx)
    {
        sws_freeContext (img_convert_ctx);
        img_convert_ctx = 0;
    }

    if (picture)
        av_free (picture);

    if (video_st)
    {
        avcodec_close (video_st->codec);
        video_st = NULL;
    }

    if (ic)
    {
        avformat_close_input (&ic);
        ic = NULL;
    }

    if (rgb_picture.data[0])
    {
        free (rgb_picture.data[0]);
        rgb_picture.data[0] = 0;
    }

    // free last packet if exist
    if (packet.data)
    {
        av_free_packet (&packet);
        packet.data = NULL;
    }

    init ();
}

bool
CvCapture_FFMPEG::open (const char* filename)
{
    unsigned i;
    bool valid = false;

    close ();

    int err = avformat_open_input (&ic, filename, NULL, NULL);
    if (err < 0)
    {
        std::cerr << "Error opening file" << std::endl;
        goto exit_func;
    }

    err = avformat_find_stream_info (ic, NULL);
    if (err < 0)
    {
        std::cerr << "Could not find codec parameters" << std::endl;
        goto exit_func;
    }
    for (i = 0; i < ic->nb_streams; i++)
    {
        AVCodecContext *enc = ic->streams[i]->codec;
        enc->thread_count = thread_count;

        if (AVMEDIA_TYPE_VIDEO == enc->codec_type && video_stream < 0)
        {
            // backup encoder' width/height
            int enc_width = enc->width;
            int enc_height = enc->height;

            AVCodec *codec = avcodec_find_decoder (enc->codec_id);
            if (!codec || avcodec_open2 (enc, codec, NULL) < 0)
            {
                goto exit_func;
            }

            // checking width/height (since decoder can sometimes alter it, eg. vp6f)
            if (enc_width && (enc->width != enc_width))
            {
                enc->width = enc_width;
            }
            if (enc_height && (enc->height != enc_height))
            {
                enc->height = enc_height;
            }

            video_stream = i;
            video_st = ic->streams[i];
#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(55,28,1)
            picture = av_frame_alloc ();
#else
            picture = avcodec_alloc_frame();
#endif
            int pic_size = avpicture_get_size (PIX_FMT_BGR24,
                                               enc->width,
                                               enc->height);
            rgb_picture.data[0] = (uint8_t*)malloc (pic_size);
            avpicture_fill ((AVPicture*)&rgb_picture,
                            rgb_picture.data[0],
                            PIX_FMT_BGR24,
                            enc->width,
                            enc->height);

            frame.width = enc->width;
            frame.height = enc->height;
            frame.cn = 3;
            frame.step = rgb_picture.linesize[0];
            frame.data = rgb_picture.data[0];
            break;
        }
    }

    if (video_stream >= 0)
    {
        valid = true;
    }

exit_func:
    if (!valid)
        close();

    return valid;
}

bool
CvCapture_FFMPEG::grab_frame (void)
{
    bool valid = false;
    int count_errs = 0;
    const int max_number_of_attempts = MAX_ATTEMPTS;

    if (!ic || !video_st)
    {
        return false;
    }

    if (ic->streams[video_stream]->nb_frames > 0 &&
        frame_number > ic->streams[video_stream]->nb_frames)
    {
        return false;
    }

    av_free_packet (&packet);

    // get the next frame
    while (!valid)
    {
        int ret = av_read_frame (ic, &packet);
        int got_picture;

        if (ret == AVERROR (EAGAIN))
        {
            continue;
        }
        if (packet.stream_index != video_stream)
        {
            av_free_packet (&packet);
            count_errs++;
            if (count_errs > max_number_of_attempts)
            {
                break;
            }
            continue;
        }
        // Decode video frame
        avcodec_decode_video2 (video_st->codec,
                               picture,
                               &got_picture,
                               &packet);
        // Did we get a video frame?
        if (got_picture)
        {
            frame_number++;
            valid = true;
        }
        else
        {
            count_errs++;
            if (count_errs > max_number_of_attempts)
            {
                break;
            }
        }

        av_free_packet (&packet);
    }

    // return if we have a new picture or not
    return valid;
}

bool
CvCapture_FFMPEG::retrieve_frame (cv::Mat &mat)
{
    if (!video_st || !picture->data[0])
    {
        return false;
    }

    //// TODO probably dont need this call (already done in the open function)
    avpicture_fill ((AVPicture*)&rgb_picture,
                    rgb_picture.data[0],
                    PIX_FMT_RGB24,
                    video_st->codec->width,
                    video_st->codec->height);

    if (img_convert_ctx == NULL ||
        frame.width != video_st->codec->width ||
        frame.height != video_st->codec->height)
    {
        if (img_convert_ctx)
        {
            sws_freeContext (img_convert_ctx);
        }

        frame.width = video_st->codec->width;
        frame.height = video_st->codec->height;

        img_convert_ctx = sws_getCachedContext (NULL,
                                                video_st->codec->width,
                                                video_st->codec->height,
                                                video_st->codec->pix_fmt,
                                                video_st->codec->width,
                                                video_st->codec->height,
                                                PIX_FMT_BGR24,
                                                SWS_BICUBIC,
                                                NULL, NULL, NULL);

        if (img_convert_ctx == NULL)
        {
            return false;
        }
    }

    sws_scale (img_convert_ctx,
               picture->data,
               picture->linesize,
               0, video_st->codec->height,
               rgb_picture.data,
               rgb_picture.linesize);

    IplImage ipl_image;
    cvInitImageHeader (&ipl_image, cvSize (frame.width, frame.height), 8, frame.cn);
    cvSetData (&ipl_image, frame.data, frame.step);
    cv::cvarrToMat (&ipl_image).copyTo (mat);

    return true;
}

/**
 * Video Reader
 */

VideoReader::VideoReader (void)
{
    ffmpeg = new CvCapture_FFMPEG;
}

VideoReader::~VideoReader (void)
{
    ffmpeg->close ();
    delete ffmpeg;
}

bool
VideoReader::open (const std::string &file)
{
    opened = ffmpeg->open (file.c_str ());
    return opened;
}

void
VideoReader::close (void)
{
    ffmpeg->close ();
    opened = false;
}

void
VideoReader::set_thread_count (unsigned int thread_count)
{
    if (!opened)
    {
        ffmpeg->thread_count = thread_count;
    }
}

bool
VideoReader::read (cv::Mat &mat)
{
    if (!ffmpeg->grab_frame ())
    {
        return false;
    }
    if (!ffmpeg->retrieve_frame (mat))
    {
        return false;
    }
    return true;
}

/* vim:ts=4:sw=4:sts=4:expandtab:cinoptions=(0
 */
