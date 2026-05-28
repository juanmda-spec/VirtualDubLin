#include "FFmpegDecoder.h"
#include <QDebug>

FFmpegDecoder::FFmpegDecoder() {
    pPacket = av_packet_alloc();
    pFrame = av_frame_alloc();
    pFrameRGB = av_frame_alloc();
    pAudioFrame = av_frame_alloc();
}

FFmpegDecoder::~FFmpegDecoder() {
    close();
    if (pPacket) av_packet_free(&pPacket);
    if (pFrame) av_frame_free(&pFrame);
    if (pFrameRGB) av_frame_free(&pFrameRGB);
    if (pAudioFrame) av_frame_free(&pAudioFrame);
}

bool FFmpegDecoder::openFile(const QString &filename) {
    close(); // Ensure any previous state is closed

    QByteArray ba = filename.toUtf8();
    const char *filePath = ba.data();

    // 1. Open the file
    if (avformat_open_input(&pFormatContext, filePath, nullptr, nullptr) != 0) {
        qDebug() << "Failed to open input file" << filename;
        return false;
    }

    // 2. Find stream information
    if (avformat_find_stream_info(pFormatContext, nullptr) < 0) {
        qDebug() << "Failed to find stream info";
        return false;
    }

    // 3. Find the first video stream
    const AVCodec *pCodec = nullptr;
    for (unsigned int i = 0; i < pFormatContext->nb_streams; i++) {
        AVStream *stream = pFormatContext->streams[i];
        AVCodecParameters *codecParams = stream->codecpar;
        if (codecParams->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoStreamIndex = i;
            pCodec = avcodec_find_decoder(codecParams->codec_id);
            break;
        }
    }

    if (videoStreamIndex == -1 || pCodec == nullptr) {
        qDebug() << "Failed to find video stream or suitable codec";
        return false;
    }

    // 4. Initialize the Codec Context
    pCodecContext = avcodec_alloc_context3(pCodec);
    avcodec_parameters_to_context(pCodecContext, pFormatContext->streams[videoStreamIndex]->codecpar);

    if (avcodec_open2(pCodecContext, pCodec, nullptr) < 0) {
        qDebug() << "Failed to open codec";
        return false;
    }


    // Find audio stream
    const AVCodec *pAudioCodec = nullptr;
    for (unsigned int i = 0; i < pFormatContext->nb_streams; i++) {
        AVStream *stream = pFormatContext->streams[i];
        if (stream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            audioStreamIndex = i;
            pAudioCodec = avcodec_find_decoder(stream->codecpar->codec_id);
            break;
        }
    }

    if (audioStreamIndex != -1 && pAudioCodec) {
        pAudioCodecContext = avcodec_alloc_context3(pAudioCodec);
        avcodec_parameters_to_context(pAudioCodecContext, pFormatContext->streams[audioStreamIndex]->codecpar);
        if (avcodec_open2(pAudioCodecContext, pAudioCodec, nullptr) < 0) {
            qDebug() << "Failed to open audio codec";
        } else {
            qDebug() << "Audio stream found and codec opened successfully";
        }
    }

    // 5. Setup SwsContext for color conversion (YUV -> RGB24)
    swsCtx = sws_getContext(
        pCodecContext->width, pCodecContext->height, pCodecContext->pix_fmt,
        pCodecContext->width, pCodecContext->height, AV_PIX_FMT_RGB24,
        SWS_BILINEAR, nullptr, nullptr, nullptr
    );

    // 6. Allocate buffer for RGB frame
    int numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGB24, pCodecContext->width, pCodecContext->height, 1);
    buffer = (uint8_t *)av_malloc(numBytes * sizeof(uint8_t));
    av_image_fill_arrays(pFrameRGB->data, pFrameRGB->linesize, buffer, AV_PIX_FMT_RGB24, pCodecContext->width, pCodecContext->height, 1);

    return true;
}

bool FFmpegDecoder::decodeNextFrame(QImage &outImage) {
    if (!pFormatContext || !pCodecContext) return false;

    while (av_read_frame(pFormatContext, pPacket) >= 0) {
        // Is this a packet from the video stream?
        if (pPacket->stream_index == videoStreamIndex) {
            // Send the packet to the decoder
            int response = avcodec_send_packet(pCodecContext, pPacket);
            if (response < 0 && response != AVERROR(EAGAIN)) {
                av_packet_unref(pPacket);
                return false;
            }

            // Receive frame from the decoder
            response = avcodec_receive_frame(pCodecContext, pFrame);
            if (response == AVERROR(EAGAIN) || response == AVERROR_EOF) {
                av_packet_unref(pPacket);
                continue; // Need more packets
            } else if (response < 0) {
                av_packet_unref(pPacket);
                return false; // Error during decoding
            }

            // Successfully received a frame, convert to RGB
            sws_scale(swsCtx, (uint8_t const *const *)pFrame->data, pFrame->linesize, 0, pCodecContext->height,
                      pFrameRGB->data, pFrameRGB->linesize);

            // Convert to QImage
            QImage img(pFrameRGB->data[0], pCodecContext->width, pCodecContext->height, pFrameRGB->linesize[0], QImage::Format_RGB888);
            outImage = img.copy(); // Deep copy so data isn't overwritten

            av_packet_unref(pPacket);
            return true;
        }
        av_packet_unref(pPacket);
    }

    return false; // End of stream or error
}

void FFmpegDecoder::close() {
    if (buffer) { av_freep(&buffer); buffer = nullptr; }
    if (swsCtx) { sws_freeContext(swsCtx); swsCtx = nullptr; }
    if (pCodecContext) { avcodec_free_context(&pCodecContext); pCodecContext = nullptr; }
    if (pFormatContext) { avformat_close_input(&pFormatContext); pFormatContext = nullptr; }
    videoStreamIndex = -1;
    audioStreamIndex = -1;
    if (pAudioCodecContext) { avcodec_free_context(&pAudioCodecContext); pAudioCodecContext = nullptr; }
}

bool FFmpegDecoder::decodeNextAudioFrame(uint8_t* audioBuffer, int& outSize) {
    if (!pFormatContext || !pAudioCodecContext) return false;

    // Simplistic audio decoding loop
    while (av_read_frame(pFormatContext, pPacket) >= 0) {
        if (pPacket->stream_index == audioStreamIndex) {
            int response = avcodec_send_packet(pAudioCodecContext, pPacket);
            if (response < 0 && response != AVERROR(EAGAIN)) {
                av_packet_unref(pPacket);
                return false;
            }

            response = avcodec_receive_frame(pAudioCodecContext, pAudioFrame);
            if (response == AVERROR(EAGAIN) || response == AVERROR_EOF) {
                av_packet_unref(pPacket);
                continue;
            } else if (response < 0) {
                av_packet_unref(pPacket);
                return false;
            }

            // In a real scenario we'd use libswresample to convert to target output format here.
            // For MVP audio support, we just copy raw PCM data if planar/packed fits.
            int data_size = av_get_bytes_per_sample(pAudioCodecContext->sample_fmt);
            if (data_size > 0) {
                outSize = pAudioFrame->nb_samples * pAudioCodecContext->ch_layout.nb_channels * data_size;
                memcpy(audioBuffer, pAudioFrame->data[0], outSize);
            }

            av_packet_unref(pPacket);
            return true;
        }
        av_packet_unref(pPacket);
    }
    return false;
}
