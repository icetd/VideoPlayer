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
	bool open(const char* url);
	bool decode(uint8_t* frame, int64_t* pts);
	bool close();

    bool reconnect();

	int getWidth() { return width; }
	int getHeight() { return height; }

    void setReconnect(bool status) { is_enable_reconnect = status; }
    void setReconnectParams(int maxAttempts, int intervalMs)
    {
        max_reconnect_attempts = maxAttempts;
        reconnect_interval = intervalMs;
    }

private:
	int width;
	int height;

	int video_stream_index;
	AVRational time_base;
	AVFormatContext* av_format_ctx;
	AVCodecContext* av_codec_ctx;
	AVFrame* av_frame;
	AVPacket* av_packet;
    SwsContext *sws_scaler_ctx;
    int sws_width = 0;
    int sws_height = 0;
    AVPixelFormat sws_pix_fmt;
    
    int reconnect_attempts;     // 当前重连尝试次数
    int max_reconnect_attempts; // 最大重连次数
    int reconnect_interval;     // 重连间隔（毫秒）
    bool reconnecting;          // 是否正在重连
    bool is_enable_reconnect;
    bool is_connected;

    const char *re_url;
};

#endif