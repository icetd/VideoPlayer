#include "video/VideoPackage.h"
#include <log.h>

VideoPackage::VideoPackage()
{

}
/** @brief RBB0 to YUV420P */
void VideoPackage::start(const char* outFilename, int in_width, int in_height)
{
	avcodec_register_all();
	av_register_all();
	
	avformat_alloc_output_context2(&ofmt_ctx, NULL, "mp4", outFilename);
	if (!ofmt_ctx) {
		LOG(ERROR, "Couldn't create out put contex");
		return;
	}

	ofmt = ofmt_ctx->oformat;
	
	if (avio_open(&ofmt_ctx->pb, outFilename, AVIO_FLAG_READ_WRITE) < 0) {
		LOG(ERROR, "Open output file failed");
		return;
	}

	video_st = avformat_new_stream(ofmt_ctx, NULL);
	if (!video_st) {
		LOG(ERROR, "allocate output stream failed");
		return;
	}

	// set frame rate
	video_st->time_base.num = 1;
	video_st->time_base.den = frame_rate;

	pCodecCtx = video_st->codec;
	pCodecCtx->codec_id = AV_CODEC_ID_H264;
	pCodecCtx->codec_type = AVMEDIA_TYPE_VIDEO;
	pCodecCtx->pix_fmt = AV_PIX_FMT_YUV420P;
	pCodecCtx->width = out_width;
	pCodecCtx->height = out_height;

	pCodecCtx->time_base.num = 1;
	pCodecCtx->time_base.den = frame_rate;
	pCodecCtx->bit_rate = bitrate;
	pCodecCtx->gop_size = frame_rate;
	
	if (pCodecCtx->codec_id == AV_CODEC_ID_H264) {
		pCodecCtx->qmin = 10;
		pCodecCtx->qmax = 51;
		pCodecCtx->qcompress = 0.6;

		pCodecCtx->thread_count = 6;
		pCodecCtx->thread_type = FF_THREAD_FRAME;
		pCodecCtx->profile = FF_PROFILE_H264_BASELINE;
		av_opt_set(pCodecCtx->priv_data, "preset", "superfast", 0);
		av_opt_set(pCodecCtx->priv_data, "tune", "zerolatency", 0);
	}

	pCodec = avcodec_find_encoder(pCodecCtx->codec_id);
	if (!pCodec) {
		LOG(ERROR, "Can't find encoder ");
		return;
	}
	int ret;
	if ((ret = avcodec_open2(pCodecCtx, pCodec, NULL)) < 0) {
		LOG(ERROR, "Can't open encoder ");
		perror("error");
		return;
	}

	av_dump_format(ofmt_ctx, 0, outFilename, 1);

		/* 设置输入图像格式 */
	picture = av_frame_alloc();
	picture->width = in_width;
	picture->height = in_height;
	picture->format = AV_PIX_FMT_RGB0;  
	picture_size = avpicture_get_size((enum AVPixelFormat)picture->format, picture->width, picture->height);
	picture_buf = (uint8_t *)av_malloc(picture_size);
	avpicture_fill((AVPicture *)picture, (const uint8_t *)picture_buf, AV_PIX_FMT_RGB0, in_width, in_height);
 
	/* 设置输出图像(送入编码器的图像)格式 */
	out_picture = av_frame_alloc();
	out_picture->width = pCodecCtx->width;
	out_picture->height = pCodecCtx->height;
	out_picture->format = pCodecCtx->pix_fmt;
	out_picture_size = avpicture_get_size((enum AVPixelFormat)out_picture->format, out_picture->width, out_picture->height);
	out_picture_buf = (uint8_t *)av_malloc(out_picture_size);
	avpicture_fill((AVPicture *)out_picture, (const uint8_t *)out_picture_buf, AV_PIX_FMT_YUV420P, out_width, out_height);

	sws_ctx = sws_getContext(in_width, in_height, AV_PIX_FMT_RGB0,
		out_width, out_height, AV_PIX_FMT_YUV420P,
		SWS_BILINEAR, NULL, NULL, NULL);

	if(!sws_ctx)
	{
		LOG(ERROR, "Impossible to create scale context for format conversion!");
		return;
	}

	
	/* 写输出文件头 */
	avformat_write_header(ofmt_ctx, NULL);
 
	av_new_packet(&enc_pkt, out_picture_size);
    LOG(INFO, "start save %s", outFilename);
}

VideoPackage::~VideoPackage()
{
	stop();
	if(ofmt_ctx)
	{
		avio_close(ofmt_ctx->pb);
		avformat_free_context(ofmt_ctx);
	}
 
	if(video_st)
	{
		avcodec_close(video_st->codec);
		video_st = NULL;
	}
 
	if(picture)
	{
		av_frame_free(&picture);
		picture = NULL;
	}
 
	if(picture_buf)
	{
		av_freep(&picture_buf);
	}
 
	if(out_picture_buf)
	{
		av_freep(&out_picture_buf);
	}
 
	if(sws_ctx)
	{
		sws_freeContext(sws_ctx);
	}
}

/*@brief RGB0 to YUV420p && packet */
void VideoPackage::saveVideo(const uint8_t* picture_buf, int in_width, int in_height)
{
	/* 将读取到的buffer数据填充到picture结构体中 */
	avpicture_fill((AVPicture*)picture, (const uint8_t*)picture_buf, AV_PIX_FMT_RGB0, in_width, in_height);

	/* 根据编码输出要求，将输入图像格式转换成编码器需要的图像格式 */
	sws_scale(sws_ctx, picture->data, picture->linesize, 0, in_height, out_picture->data, out_picture->linesize);

	/* 设置当前帧的时间戳 */
	out_picture->pts = frame_pts++;
	int got_pkt = 0;

	// 编码一帧数据
	int ret = avcodec_encode_video2(pCodecCtx, &enc_pkt, out_picture, &got_pkt);
	if (ret < 0)
	{
		LOG(ERROR, "Encode failed!");
		return;
	}
	/* 将编码后的一包数据打上时间戳，然后写到输出文件中 */
	if (got_pkt)
	{
		enc_pkt.stream_index = video_st->index;
		av_packet_rescale_ts(&enc_pkt, pCodecCtx->time_base, video_st->time_base);  //根据timebase计算输出流的时间戳
		enc_pkt.pos = -1;
		ret = av_interleaved_write_frame(ofmt_ctx, &enc_pkt);  // Write the encoded packet to the output file
		av_free_packet(&enc_pkt);
	}
}

void VideoPackage::stop()
{
	int ret = flush_encoder(ofmt_ctx, 0);
	if(ret < 0)
	{
		LOG(ERROR, "Flushing encoder failed.");
		return ;
	}
	// Write the output file trailer
	av_write_trailer(ofmt_ctx);


    // 关闭并释放文件输出上下文
    if (ofmt_ctx) {
        if (ofmt_ctx->pb) {
            avio_close(ofmt_ctx->pb);
        }
        avformat_free_context(ofmt_ctx);
        ofmt_ctx = nullptr;
    }

    // 释放编码器和视频流
    if (video_st) {
        if (video_st->codec) {
            avcodec_close(video_st->codec);
        }
        video_st = nullptr;
    }

    // 释放帧和缓冲区
    if (picture) {
        av_frame_free(&picture);
        picture = nullptr;
    }

    if (out_picture) {
        av_frame_free(&out_picture);
        out_picture = nullptr;
    }

    if (picture_buf) {
        av_freep(&picture_buf);
        picture_buf = nullptr;
    }

    if (out_picture_buf) {
        av_freep(&out_picture_buf);
        out_picture_buf = nullptr;
    }

    // 释放格式转换上下文
    if (sws_ctx) {
        sws_freeContext(sws_ctx);
        sws_ctx = nullptr;
    }

    LOG(INFO, "Video package stopped and resources released.");
}

int VideoPackage::flush_encoder(AVFormatContext *fmt_ctx, unsigned int stream_index)
{
    int ret;
    int got_frame;
	AVPacket enc_pkt;
	
    if (!(fmt_ctx->streams[stream_index]->codec->codec->capabilities & AV_CODEC_CAP_DELAY))
        return 0;
 
    while (1) 
	{
		enc_pkt.data = NULL;
		enc_pkt.size = 0;
		av_init_packet(&enc_pkt);
		ret = avcodec_encode_video2(fmt_ctx->streams[stream_index]->codec, &enc_pkt, NULL, &got_frame);
 
		av_frame_free(NULL);
			
        if (ret < 0)
        	break;

        if (!got_frame) {
        	ret = 0;
			break;
        }
 
		enc_pkt.stream_index = stream_index;
		av_packet_rescale_ts(&enc_pkt, fmt_ctx->streams[stream_index]->codec->time_base, 
			fmt_ctx->streams[stream_index]->time_base);
		
		ret = av_interleaved_write_frame(fmt_ctx, &enc_pkt);
		if(ret < 0) 
			break;
    }
	
    return ret;
}

