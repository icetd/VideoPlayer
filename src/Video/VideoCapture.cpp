#include "VideoCapture.h"
#include "log.h"
#include <thread>

VideoCapture::VideoCapture()
{
}

VideoCapture::~VideoCapture()
{
}

void VideoCapture::init()
{
    is_connected = false;
    reconnect_attempts = 0;
    max_reconnect_attempts = 3;
    reconnect_interval = 1000;
    reconnecting = false;
	video_stream_index = -1;
	av_format_ctx = nullptr;
	av_codec_ctx = nullptr;
	av_frame = nullptr;
	av_packet = nullptr;
	sws_scaler_ctx = nullptr;
	avdevice_register_all();
	avformat_network_init();
	av_log_set_level(AV_LOG_FATAL);
	LOG(INFO, "libCapture init success.");
}

bool VideoCapture::open(const char* url)
{
    re_url = url;
	av_format_ctx = avformat_alloc_context();
	if (!av_format_ctx) {
		LOG(ERROR, "Couldn't create AVFormatContext");
		return false;
	}

	/* options rtsp*/
	AVDictionary* opts = nullptr;
	av_dict_set(&opts, "rtsp_transport", "tcp", 0);
	av_dict_set(&opts, "buffer_size", "1024000", 0);
	av_dict_set(&opts, "stimeout", "1000000", 0);
	av_dict_set(&opts, "max_delay", "1000000", 0);

	if (avformat_open_input(&av_format_ctx, url, NULL, &opts) != 0) {
		LOG(ERROR, "Could't open video url");
		return false;
	}
	avformat_find_stream_info(av_format_ctx, NULL);
	int totalMs = av_format_ctx->duration / (AV_TIME_BASE / 1000);
	LOG(INFO, "total time : %dms", totalMs);

	/** @brief echo format info */
	av_dump_format(av_format_ctx, NULL, NULL, false);

	AVCodecParameters* av_codec_params;
	AVCodec* av_codec;
	AVStream* av_stream;

	for (uint32_t i = 0; i < av_format_ctx->nb_streams; ++i) {
		av_stream = av_format_ctx->streams[i];
		av_codec_params = av_format_ctx->streams[i]->codecpar;
		av_codec = avcodec_find_decoder(av_codec_params->codec_id);
		if (!av_codec)
			continue;

		if (av_codec_params->codec_type == AVMEDIA_TYPE_VIDEO) {
			video_stream_index = i;
			width = av_codec_params->width;
			height = av_codec_params->height;
			time_base = av_format_ctx->streams[i]->time_base;
			break;
		}
	}

	if (video_stream_index == -1) {
		LOG(ERROR, "Couldn't find valid video stream from url");
		return false;
	}

	// Set up a codec context for the decoder
	av_codec_ctx = avcodec_alloc_context3(av_codec);
	if (!av_codec_ctx) {
		LOG(ERROR, "Couldn't create AVCodecContext");
		return false;
	}

	if (avcodec_parameters_to_context(av_codec_ctx, av_codec_params) < 0) {
		LOG(ERROR, "Couldn't initialize AVCodecContext");
		return false;
	}
	if (avcodec_open2(av_codec_ctx, av_codec, NULL) < 0) {
		LOG(ERROR, "Couldn't open codec");
		return false;
	}

	av_frame = av_frame_alloc();
	if (!av_frame) {
		LOG(ERROR, "Couldn't allocate AVFrame");
		return false;
	}
	av_packet = av_packet_alloc();
	if (!av_packet) {
		LOG(ERROR, "Couldn't allocate AVPacket");
		return false;
	}
	LOG(INFO, "libCapture open url success.");

    is_connected = true;
	return true;
}

bool VideoCapture::reconnect() {
    if (reconnecting) {
        LOG(INFO, "Already reconnecting...");
        return false;
    }

    reconnecting = true;
    close();

    while (reconnect_attempts < max_reconnect_attempts && is_enable_reconnect) {
        LOG(INFO, "Attempting to reconnect... (%d/%d)", reconnect_attempts + 1, max_reconnect_attempts);
        if (open(re_url)) { 
            reconnect_attempts = 0; // 重置重连次数
            reconnecting = false;
            LOG(INFO, "Reconnection successful.");
            return true;
        }

        reconnect_attempts++;
        std::this_thread::sleep_for(std::chrono::milliseconds(reconnect_interval)); // 线程休眠
    }

    reconnecting = false;
    LOG(ERROR, "Failed to reconnect after %d attempts.", max_reconnect_attempts);
    return false;
}

bool VideoCapture::decode(uint8_t *frame, int64_t *pts) {
    int response;
    char errStr[256] = {0};
    is_enable_reconnect = true;

    while (is_connected) {
        int ret = av_read_frame(av_format_ctx, av_packet);
        if (ret < 0) {
            if (ret == AVERROR_EOF) {
                LOG(INFO, "End of stream reached");
                if (!reconnect()) {
                    return false;
                }
                continue;
            }
            // 判断是否为连接错误
            if (ret == AVERROR(EIO) || ret == AVERROR(ECONNRESET)) {
                LOG(ERROR, "Network error detected, attempting to reconnect...");
                if (!reconnect()) {
                    return false;
                }
                continue; 
            }

            av_strerror(ret, errStr, sizeof(errStr));
            LOG(ERROR, "av_read_frame failed: %s", errStr);
            return false;
        }

        if (av_packet->stream_index != video_stream_index) {
            av_packet_unref(av_packet);
            continue;
        }

        response = avcodec_send_packet(av_codec_ctx, av_packet);
        av_packet_unref(av_packet);
        if (response < 0) {
            av_strerror(response, errStr, sizeof(errStr));
            LOG(ERROR, "Failed to send packet: %s\n", errStr);

            // 如果发送包失败，也尝试重连
            if (!reconnect()) {
                return false;
            }
            continue;
        }

        response = avcodec_receive_frame(av_codec_ctx, av_frame);
        if (response == AVERROR(EAGAIN) || response == AVERROR_EOF) {
            continue;
        } else if (response < 0) {
            av_strerror(response, errStr, sizeof(errStr));
            LOG(ERROR, "Failed to receive frame: %s\n", errStr);

            // 如果接收帧失败，也尝试重连
            if (!reconnect()) {
                return false;
            }
            continue;
        }

        *pts = av_frame->pts;

        // 初始化 SWS 上下文（仅当分辨率或像素格式变化时）
        if (!sws_scaler_ctx || av_frame->width != sws_width || av_frame->height != sws_height || av_codec_ctx->pix_fmt != sws_pix_fmt) {
            if (sws_scaler_ctx) 
                sws_freeContext(sws_scaler_ctx);

            sws_scaler_ctx = sws_getContext(av_frame->width, av_frame->height, av_codec_ctx->pix_fmt,
                                            av_frame->width, av_frame->height, AV_PIX_FMT_RGBA,
                                            SWS_BILINEAR, NULL, NULL, NULL);
            if (!sws_scaler_ctx) {
                LOG(ERROR, "Couldn't initialize SWS context\n");
                return false;
            }

            sws_width = av_frame->width;
            sws_height = av_frame->height;
            sws_pix_fmt = av_codec_ctx->pix_fmt;
        }

        uint8_t *dest[4] = {frame, NULL, NULL, NULL};
        int dest_linesize[4] = {av_frame->width * 4, 0, 0, 0};
    
        sws_scale(sws_scaler_ctx, av_frame->data, av_frame->linesize, 0, av_frame->height, dest, dest_linesize);
        return true;
    }

    // 检测到错误，尝试重连
    return false;
}

bool VideoCapture::close()
{
	if (av_codec_ctx) {
		avformat_close_input(&av_format_ctx);
		avformat_free_context(av_format_ctx);
	}
	if (av_codec_ctx) {
		avcodec_free_context(&av_codec_ctx);
	}
	if (av_packet) {
		av_free(av_packet);
	}
	if (av_frame) {
		av_frame_free(&av_frame);
	}
	avformat_network_deinit();


    av_codec_ctx = nullptr;
    av_packet = nullptr;
    av_frame = nullptr;
    is_connected = false;
	return true;
}