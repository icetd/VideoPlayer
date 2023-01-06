#ifndef LOAD_FRAME_H
#define LOAD_FRAME_H


#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavdevice/avdevice.h>

#ifdef __cplusplus
}
#endif

struct VideoReaderState {
    int width, height;
    int video_stream_index = -1;
    AVRational time_base;
    
    AVFormatContext* av_format_ctx;
    AVCodecContext* av_codec_ctx;
    AVFrame* av_frame;
    AVPacket* av_packet;
    SwsContext *sws_scaler_ctx;
};

bool video_reader_open(VideoReaderState *state, const char* filename);
bool video_reader_frame(VideoReaderState *state, uint8_t* frame_buffer, int64_t *pts);
bool video_reader_close(VideoReaderState *state);

#endif // end load_frame.h