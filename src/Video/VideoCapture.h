#ifndef VIDEO_CAPTURE_H
#define VIDEO_CAPTURE_H

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

class VideoCapture {
public:
	VideoCapture();
	~VideoCapture();
	void init();
	bool open(const char *url);
	bool decode(uint8_t *frame, int64_t *pts);
	bool close();

	int getWidth() { return width; }
	int getHeight() { return height; }

private:
	int width;
	int height;

	int video_stream_index;
	AVRational time_base;
	AVFormatContext *av_format_ctx;
	AVCodecContext *av_codec_ctx;
	AVFrame *av_frame;
	AVPacket *av_packet;
	SwsContext *sws_scaler_ctx;
};

#endif