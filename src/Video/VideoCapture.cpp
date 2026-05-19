#include "VideoCapture.h"
#include "../Core/log.h"
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

bool VideoCapture::open(const char *url)
{
    re_url = url;
    av_format_ctx = avformat_alloc_context();
    if (!av_format_ctx) {
        LOG(ERRO, "Couldn't create AVFormatContext");
        return false;
    }

    /* options rtsp*/
    AVDictionary *opts = nullptr;
    av_dict_set(&opts, "rtsp_transport", "tcp", 0);
    av_dict_set(&opts, "buffer_size", "1024000", 0);
    av_dict_set(&opts, "stimeout", "1000000", 0);
    av_dict_set(&opts, "max_delay", "1000000", 0);

    if (avformat_open_input(&av_format_ctx, url, NULL, &opts) != 0) {
        LOG(ERRO, "Could't open video url %s", url);
        return false;
    }
    avformat_find_stream_info(av_format_ctx, NULL);
    int totalMs = av_format_ctx->duration / (AV_TIME_BASE / 1000);
    LOG(INFO, "total time : %dms", totalMs);

    input_type = av_format_ctx->iformat->name;
    LOG(INFO, "input type : %s", input_type.c_str());

    /** @brief echo format info */
    av_dump_format(av_format_ctx, NULL, NULL, false);

    AVCodecParameters *av_codec_params;
    AVCodec *av_codec;
    AVStream *av_stream;

    for (uint32_t i = 0; i < av_format_ctx->nb_streams; ++i) {
        av_stream = av_format_ctx->streams[i];
        av_codec_params = av_format_ctx->streams[i]->codecpar;
        av_codec = avcodec_find_decoder(av_codec_params->codec_id);
        // av_codec = avcodec_find_decoder_by_name("h264_cuvid");
        if (!av_codec)
            continue;

        if (av_codec_params->codec_type == AVMEDIA_TYPE_VIDEO) {
            video_stream_index = i;
            width = av_codec_params->width;
            height = av_codec_params->height;
            fps = av_stream->avg_frame_rate.num / av_stream->avg_frame_rate.den;
            time_base = av_format_ctx->streams[i]->time_base;
            break;
        }
    }

    if (video_stream_index == -1) {
        LOG(ERRO, "Couldn't find valid video stream from url");
        return false;
    }

    // Set up a codec context for the decoder
    av_codec_ctx = avcodec_alloc_context3(av_codec);
    if (!av_codec_ctx) {
        LOG(ERRO, "Couldn't create AVCodecContext");
        return false;
    }

    if (avcodec_parameters_to_context(av_codec_ctx, av_codec_params) < 0) {
        LOG(ERRO, "Couldn't initialize AVCodecContext");
        return false;
    }
    if (avcodec_open2(av_codec_ctx, av_codec, NULL) < 0) {
        LOG(ERRO, "Couldn't open codec");
        return false;
    }

    av_frame = av_frame_alloc();
    if (!av_frame) {
        LOG(ERRO, "Couldn't allocate AVFrame");
        return false;
    }
    av_packet = av_packet_alloc();
    if (!av_packet) {
        LOG(ERRO, "Couldn't allocate AVPacket");
        return false;
    }
    LOG(INFO, "libCapture open url success.");

    is_connected = true;
    return true;
}

bool VideoCapture::reconnect()
{
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
    LOG(ERRO, "Failed to reconnect after %d attempts.", max_reconnect_attempts);
    return false;
}

bool VideoCapture::decode(uint8_t *frame, int64_t *pts)
{
    // ========== 1. Parameter validation ==========
    if (!frame || !pts) {
        LOG(ERRO, "decode: Invalid parameters - frame=%p, pts=%p", frame, pts);
        return false;
    }

    // ========== 2. State validation ==========
    if (!is_connected) {
        LOG(WARN, "decode: Not connected");
        return false;
    }

    if (!av_format_ctx || !av_codec_ctx || !av_frame || !av_packet) {
        LOG(ERRO, "decode: FFmpeg contexts not properly initialized");
        if (is_enable_reconnect && reconnect()) {
            return decode(frame, pts);
        }
        return false;
    }

    int response;
    char errStr[256] = {0};
    is_enable_reconnect = true;

    // ========== 3. Main decode loop ==========
    while (is_connected) {
        // ----- 3.1 Check context validity -----
        if (!av_format_ctx || !av_packet) {
            LOG(ERRO, "decode: Context became null during loop");
            if (is_enable_reconnect && reconnect()) {
                continue;
            }
            return false;
        }

        // ----- 3.2 Read packet from stream -----
        int ret = av_read_frame(av_format_ctx, av_packet);

        if (ret < 0) {
            av_packet_unref(av_packet);

            if (ret == AVERROR_EOF) {
                LOG(INFO, "decode: End of stream reached");
                if (is_enable_reconnect && reconnect()) {
                    continue;
                }
                return false;
            }

            if (ret == AVERROR(EIO) || ret == AVERROR(ECONNRESET) || ret == AVERROR(ETIMEDOUT) || 
                ret == AVERROR(ENETRESET) || ret == AVERROR(ENETUNREACH) || ret == -110) {
                LOG(ERRO, "decode: Network error (err=%d), reconnecting...", ret);
                if (is_enable_reconnect && reconnect()) {
                    continue;
                }
                return false;
            }

            av_strerror(ret, errStr, sizeof(errStr));
            LOG(ERRO, "decode: av_read_frame failed: %s (err=%d)", errStr, ret);
            if (is_enable_reconnect && reconnect()) {
                continue;
            }
            return false;
        }

        // ----- 3.3 Filter non-video streams -----
        if (av_packet->stream_index != video_stream_index) {
            av_packet_unref(av_packet);
            continue;
        }

        // ----- 3.4 Validate packet data -----
        if (av_packet->data == nullptr || av_packet->size <= 0 || av_packet->size > 10 * 1024 * 1024) {
            LOG(WARN, "decode: Invalid packet");
            av_packet_unref(av_packet);
            continue;
        }

        // ----- 3.5 Send packet to decoder -----
        response = avcodec_send_packet(av_codec_ctx, av_packet);
        av_packet_unref(av_packet);

        if (response < 0) {
            if (response == AVERROR(EAGAIN)) {
                continue;
            }

            av_strerror(response, errStr, sizeof(errStr));
            LOG(ERRO, "decode: Failed to send packet: %s", errStr);

            if (response == AVERROR(EINVAL) || response == AVERROR(ENOMEM) || response == AVERROR_INVALIDDATA) {
                avcodec_flush_buffers(av_codec_ctx);
            }

            if (is_enable_reconnect && reconnect()) {
                continue;
            }
            return false;
        }

        // ----- 3.6 Receive decoded frame -----
        response = avcodec_receive_frame(av_codec_ctx, av_frame);

        if (response == AVERROR(EAGAIN)) {
            continue;
        } else if (response == AVERROR_EOF) {
            LOG(INFO, "decode: Decoder EOF");
            continue;
        } else if (response < 0) {
            av_strerror(response, errStr, sizeof(errStr));
            LOG(ERRO, "decode: Failed to receive frame: %s", errStr);
            if (is_enable_reconnect && reconnect()) {
                continue;
            }
            return false;
        }

        // ========== 4. Successfully got a frame ==========

        // ----- 4.1 Validate frame data integrity -----
        if (!av_frame->data[0] || !av_frame->buf[0]) {
            LOG(ERRO, "decode: Invalid frame data");
            av_frame_unref(av_frame);
            continue;
        }

        if (av_frame->linesize[0] <= 0 || av_frame->width <= 0 || av_frame->height <= 0) {
            LOG(ERRO, "decode: Invalid frame dimensions");
            av_frame_unref(av_frame);
            continue;
        }

        if (av_frame->width > 4096 || av_frame->height > 2160) {
            LOG(ERRO, "decode: Unreasonable resolution: %dx%d", av_frame->width, av_frame->height);
            av_frame_unref(av_frame);
            continue;
        }

        // ----- 4.2 Generate PTS (simple counter) -----
        *pts = m_frame_counter++;

        // ----- 4.3 Handle resolution changes -----
        bool resolution_changed = (av_frame->width != sws_width || av_frame->height != sws_height);
        bool pix_fmt_changed = (av_codec_ctx->pix_fmt != sws_pix_fmt);

        if (resolution_changed) {
            LOG(INFO, "decode: Resolution changed: %dx%d -> %dx%d",
                sws_width, sws_height, av_frame->width, av_frame->height);
            width = av_frame->width;
            height = av_frame->height;
        }

        // ----- 4.4 Initialize/rebuild SWS context -----
        if (!sws_scaler_ctx || resolution_changed || pix_fmt_changed) {
            if (sws_scaler_ctx) {
                sws_freeContext(sws_scaler_ctx);
                sws_scaler_ctx = nullptr;
            }

            sws_scaler_ctx = sws_getContext(
                av_frame->width, av_frame->height, av_codec_ctx->pix_fmt,
                av_frame->width, av_frame->height, AV_PIX_FMT_RGBA,
                SWS_BILINEAR, nullptr, nullptr, nullptr);

            if (!sws_scaler_ctx) {
                LOG(ERRO, "decode: Failed to create SWS context");
                av_frame_unref(av_frame);
                if (is_enable_reconnect && reconnect()) {
                    continue;
                }
                return false;
            }

            sws_width = av_frame->width;
            sws_height = av_frame->height;
            sws_pix_fmt = av_codec_ctx->pix_fmt;
        }

        // ----- 4.5 Perform pixel format conversion -----
        uint8_t *dest[4] = {frame, nullptr, nullptr, nullptr};
        int dest_linesize[4] = {av_frame->width * 4, 0, 0, 0};

        int scale_ret = sws_scale(sws_scaler_ctx,
                                  av_frame->data, av_frame->linesize,
                                  0, av_frame->height,
                                  dest, dest_linesize);

        if (scale_ret != av_frame->height) {
            LOG(ERRO, "decode: sws_scale failed");
            av_frame_unref(av_frame);
            if (sws_scaler_ctx) {
                sws_freeContext(sws_scaler_ctx);
                sws_scaler_ctx = nullptr;
            }
            continue;
        }

        // ----- 4.6 Cleanup and return success -----
        av_frame_unref(av_frame);
        return true;
    }

    LOG(WARN, "decode: Exited loop without frame");
    return false;
}

bool VideoCapture::close()
{
    if (av_packet) {
        av_packet_free(&av_packet);
    }

    if (av_frame) {
        av_frame_free(&av_frame);
    }

    if (av_codec_ctx) {
        avcodec_free_context(&av_codec_ctx);
    }

    if (av_format_ctx) {
        avformat_close_input(&av_format_ctx);
    }

    av_format_ctx = nullptr;
    av_codec_ctx = nullptr;
    av_packet = nullptr;
    av_frame = nullptr;

    sws_width = 0;
    sws_height = 0;
    sws_pix_fmt = AV_PIX_FMT_NONE;

    video_stream_index = -1;
    reconnect_attempts = 0;
    m_frame_counter = 0;

    LOG(INFO, "VideoCapture closed successfully");

    return true;
}