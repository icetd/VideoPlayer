#ifndef PACKAGEYUV_H
#define PACKAGEYUV_H

#ifdef __cplusplus
extern "C" {
#endif

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libswscale/swscale.h>
#include <libavutil/opt.h>

#ifdef __cplusplus
}
#endif

#include <string>

class VideoPackage
{
public:
	VideoPackage();
	virtual ~VideoPackage();
	void start(const char* outFilename, int in_width, int in_height);
	void saveVideo(const uint8_t* picture_buf, int in_width, int in_height);
	void stop();
private:
	int out_width = 640;
	int out_height = 480;
	int frame_rate = 30;
	int bitrate = 3000000;

    std::string m_outFilename;

	AVOutputFormat* ofmt = nullptr;
	AVFormatContext* ofmt_ctx = nullptr;
	AVStream* video_st = nullptr;
	AVCodecContext* pCodecCtx = nullptr;
	AVCodec* pCodec = nullptr;
	SwsContext* sws_ctx = nullptr;
	
	AVFrame* picture = nullptr;
	uint8_t* picture_buf = nullptr;
	AVFrame* out_picture = nullptr;
	uint8_t* out_picture_buf = nullptr;
	int picture_size = 0;
	int out_picture_size = 0;

	AVPacket enc_pkt;
	int64_t frame_pts = 0;
	size_t read_size;
	bool isrgb = false;

	static int flush_encoder(AVFormatContext* fmt_ctx, unsigned int stream_index);
};

#endif 