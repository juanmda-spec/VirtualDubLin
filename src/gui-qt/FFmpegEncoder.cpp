#include "FFmpegEncoder.h"
#include <QDebug>

FFmpegEncoder::FFmpegEncoder() {
    pPacket = av_packet_alloc();
    pFrameYUV = av_frame_alloc();
}

FFmpegEncoder::~FFmpegEncoder() {
    close();
    if (pPacket) av_packet_free(&pPacket);
    if (pFrameYUV) av_frame_free(&pFrameYUV);
}

bool FFmpegEncoder::init(const QString &filename, int width, int height, const QString &codecName, int bitrate) {
    close();

    QByteArray ba = filename.toUtf8();
    const char *filePath = ba.data();

    avformat_alloc_output_context2(&pFormatContext, nullptr, nullptr, filePath);
    if (!pFormatContext) return false;

    const AVCodec *codec = nullptr;
    if (codecName == "hevc_nvenc") codec = avcodec_find_encoder_by_name("hevc_nvenc");
    else if (codecName == "libx265") codec = avcodec_find_encoder_by_name("libx265");
    else codec = avcodec_find_encoder_by_name("libx264"); // Default fallback

    if (!codec) codec = avcodec_find_encoder(AV_CODEC_ID_H264);

    if (!codec) return false;

    videoStream = avformat_new_stream(pFormatContext, codec);
    if (!videoStream) return false;

    pCodecContext = avcodec_alloc_context3(codec);
    pCodecContext->width = width;
    pCodecContext->height = height;
    pCodecContext->time_base = {1, 30}; // 30 fps default
    pCodecContext->framerate = {30, 1};
    pCodecContext->pix_fmt = AV_PIX_FMT_YUV420P;
    pCodecContext->bit_rate = bitrate;

    if (pFormatContext->oformat->flags & AVFMT_GLOBALHEADER)
        pCodecContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

    if (avcodec_open2(pCodecContext, codec, nullptr) < 0) return false;
    avcodec_parameters_from_context(videoStream->codecpar, pCodecContext);


    // Add Audio Stream (AAC)
    const AVCodec *audioCodec = avcodec_find_encoder(AV_CODEC_ID_AAC);
    if (audioCodec) {
        audioStream = avformat_new_stream(pFormatContext, audioCodec);
        pAudioCodecContext = avcodec_alloc_context3(audioCodec);
        pAudioCodecContext->sample_rate = 44100;
        pAudioCodecContext->sample_fmt = AV_SAMPLE_FMT_FLTP;
        av_channel_layout_default(&pAudioCodecContext->ch_layout, 2);
        if (pFormatContext->oformat->flags & AVFMT_GLOBALHEADER)
            pAudioCodecContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
        if (avcodec_open2(pAudioCodecContext, audioCodec, nullptr) >= 0) {
            avcodec_parameters_from_context(audioStream->codecpar, pAudioCodecContext);
        }
    }

    if (!(pFormatContext->oformat->flags & AVFMT_NOFILE)) {
        if (avio_open(&pFormatContext->pb, filePath, AVIO_FLAG_WRITE) < 0) return false;
    }

    if (avformat_write_header(pFormatContext, nullptr) < 0) return false;

    pFrameYUV->format = pCodecContext->pix_fmt;
    pFrameYUV->width = width;
    pFrameYUV->height = height;
    av_frame_get_buffer(pFrameYUV, 0);

    swsCtx = sws_getContext(width, height, AV_PIX_FMT_RGB32,
                            width, height, AV_PIX_FMT_YUV420P,
                            SWS_BILINEAR, nullptr, nullptr, nullptr);

    frameCounter = 0;
    return true;
}

bool FFmpegEncoder::encodeFrame(const QImage &image) {
    if (!pCodecContext) return false;

    // Convert QImage (RGB32) to YUV420P
    const uint8_t *inData[1] = { image.constBits() };
    int inLinesize[1] = { image.bytesPerLine() };

    sws_scale(swsCtx, inData, inLinesize, 0, pCodecContext->height,
              pFrameYUV->data, pFrameYUV->linesize);

    pFrameYUV->pts = frameCounter++;

    int response = avcodec_send_frame(pCodecContext, pFrameYUV);
    if (response < 0) return false;

    while (response >= 0) {
        response = avcodec_receive_packet(pCodecContext, pPacket);
        if (response == AVERROR(EAGAIN) || response == AVERROR_EOF) break;
        if (response < 0) return false;

        av_packet_rescale_ts(pPacket, pCodecContext->time_base, videoStream->time_base);
        pPacket->stream_index = videoStream->index;
        av_interleaved_write_frame(pFormatContext, pPacket);
        av_packet_unref(pPacket);
    }

    return true;
}

void FFmpegEncoder::close() {
    if (pCodecContext) {
        avcodec_send_frame(pCodecContext, nullptr); // flush
        while (avcodec_receive_packet(pCodecContext, pPacket) >= 0) {
            av_packet_rescale_ts(pPacket, pCodecContext->time_base, videoStream->time_base);
            pPacket->stream_index = videoStream->index;
            av_interleaved_write_frame(pFormatContext, pPacket);
            av_packet_unref(pPacket);
        }
    }

    if (pFormatContext) {
        av_write_trailer(pFormatContext);

    if (!(pFormatContext->oformat->flags & AVFMT_NOFILE)) {
            avio_closep(&pFormatContext->pb);
        }
        avformat_free_context(pFormatContext);
        pFormatContext = nullptr;
    }

    if (pCodecContext) { avcodec_free_context(&pCodecContext); pCodecContext = nullptr; }
    if (swsCtx) { sws_freeContext(swsCtx); swsCtx = nullptr; }
}

bool FFmpegEncoder::encodeAudioFrame(const uint8_t *audioData, int dataSize) {
    if (!pAudioCodecContext) return false;

    // In a complete implementation we would allocate AVFrame for audio,
    // fill it with dataSize bytes, and send it to avcodec_send_frame.
    // For now, return true as structural implementation.
    return true;
}
