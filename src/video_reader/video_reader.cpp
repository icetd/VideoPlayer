#include "video_reader.h"
#include <iostream>

bool video_reader_open(VideoReaderState *state, const char* filename)
{
    avdevice_register_all();
    
    avformat_network_init();

    //Open the file using libavformat
    auto& width = state->width;
    auto& height = state->height;
    auto& time_base = state->time_base;
    auto& av_format_ctx = state->av_format_ctx;
    auto& av_codec_ctx = state->av_codec_ctx;
    auto& av_frame = state->av_frame;
    auto& av_packet = state->av_packet;
    auto& video_stream_index = state->video_stream_index;
    auto& sws_scaler_ctx = state->sws_scaler_ctx;
    
    av_format_ctx = avformat_alloc_context();
    if (!av_format_ctx) {
        printf("Couldn't created AVFormatContext\n");
        return false;
    }
    
	/* options, such as rtsp. */
	AVDictionary* opts = nullptr;
	av_dict_set(&opts, "rtsp_transport", "tcp", 0);
	av_dict_set(&opts, "max_delay", "500", 0);
    
    /*@bfief: open in put*/
    if(avformat_open_input(&av_format_ctx, filename, NULL, &opts) != 0) {
        printf("Couldn't open video file\n");
        return false;
    }
    
    /*@brief find the format info */
	avformat_find_stream_info(av_format_ctx, NULL);
	int totalMs = av_format_ctx->duration / (AV_TIME_BASE / 1000);
	std::cout << "total time = "<< totalMs << " ms"<< std::endl;
	std::cout << "=================================================================" <<std::endl;

	/*@brief echo the format info */
	av_dump_format(av_format_ctx, NULL, NULL, false);
	std::cout << "=================================================================" <<std::endl;

    // Find the first valid video stream inside the file
    AVCodecParameters* av_codec_params;
    AVCodec* av_codec;
    AVStream *av_stream;
    for (uint32_t i = 0; i < av_format_ctx->nb_streams; ++i) {
        av_stream = av_format_ctx->streams[i];
        av_codec_params = av_format_ctx->streams[i]->codecpar;
        av_codec = avcodec_find_decoder(av_codec_params->codec_id);

        if (!av_codec) {
            continue;
        }
        if (av_codec_params->codec_type == AVMEDIA_TYPE_VIDEO) {
            video_stream_index = i;
            width = av_codec_params->width;
            height = av_codec_params->height;
            time_base = av_format_ctx->streams[i]->time_base;
            break;            
        }
    }
    if (video_stream_index == -1) {
        printf("Couldn't find valid video stream inside file\n");
        return false;
    }

    // Set up a codec context for the decoder
    av_codec_ctx = avcodec_alloc_context3(av_codec);
    if (!av_codec_ctx) {
        printf("Couldn't create AVCodecContext\n");
        return false;
    }
    if (avcodec_parameters_to_context(av_codec_ctx, av_codec_params) < 0) {
        printf("Couldn't initialize AVCodecContext\n");
        return false;
    }
    if (avcodec_open2(av_codec_ctx, av_codec, NULL) < 0) {
        printf("Coudldn't open codec");
        return false;
    }

    av_frame = av_frame_alloc();
    if (!av_frame) {
        printf("Couldn't allocate ACFrame\n");
        return false;
    }
    av_packet = av_packet_alloc();
    if (!av_packet) {
        printf("Couldn't allocate ACPacket\n");
        return false;
    }

    return true;
}

bool video_reader_frame(VideoReaderState *state, uint8_t* frame_buffer, int64_t *pts)
{
    auto& width = state->width;
    auto& height = state->height;
    auto& av_format_ctx = state->av_format_ctx;
    auto& av_codec_ctx = state->av_codec_ctx;
    auto& av_frame = state->av_frame;
    auto& av_packet = state->av_packet;
    auto& video_stream_index = state->video_stream_index;
    auto& sws_scaler_ctx = state->sws_scaler_ctx;
    
    int response;
    char errStr[256] = { 0 };
    
    while(av_read_frame(av_format_ctx, av_packet) >= 0) {
        if (av_packet->stream_index != video_stream_index) {
            av_packet_unref(av_packet);
            continue;
        }
        response = avcodec_send_packet(av_codec_ctx, av_packet);
        if (response < 0) {
            av_strerror(response, errStr, sizeof(errStr));
            printf("Failed to decode packet: %s\n", errStr);
            return false;
        } 
        response = avcodec_receive_frame(av_codec_ctx, av_frame);
        if (response == AVERROR(EAGAIN) || response == AVERROR_EOF) {
            av_packet_unref(av_packet);
            continue;
        } else if (response < 0) {
            av_strerror(response, errStr, sizeof(errStr));
            printf("Failed to decode packet: %s\n", errStr);
            return false;
        }
        av_packet_unref(av_packet);
        break;
    }

    *pts = av_frame->pts;

    sws_scaler_ctx = sws_getContext(av_frame->width, av_frame->height, av_codec_ctx->pix_fmt,
                                                av_frame->width, av_frame->height, AV_PIX_FMT_RGB0,
                                                SWS_BILINEAR, NULL, NULL, NULL);

    if (!sws_scaler_ctx) {
        printf("Coudldn't initialize sw scaler\n");
        return false;
    }

    uint8_t* dest[4] = {frame_buffer, NULL, NULL, NULL};
    int dest_linesize[4] = {av_frame->width * 4, 0, 0, 0 };
    sws_scale(sws_scaler_ctx, av_frame->data, av_frame->linesize, 0, av_frame->height, dest, dest_linesize);
    sws_freeContext(sws_scaler_ctx);

    return true;
}


bool video_reader_close(VideoReaderState *state)
{
    avformat_close_input(&state->av_format_ctx);
    avformat_free_context(state->av_format_ctx);
    av_frame_free(&state->av_frame);
    av_packet_free(&state->av_packet);
    avcodec_free_context(&state->av_codec_ctx);

    return true;
}
