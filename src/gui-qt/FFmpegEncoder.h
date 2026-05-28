#ifndef FFMPEGENCODER_H
#define FFMPEGENCODER_H

#include <QString>
#include <QImage>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
}

class FFmpegEncoder {
public:
    FFmpegEncoder();
    ~FFmpegEncoder();

    bool init(const QString &filename, int width, int height, const QString &codecName, int bitrate);
    bool encodeFrame(const QImage &image);
    bool encodeAudioFrame(const uint8_t *audioData, int dataSize);
    void close();

private:
    AVFormatContext *pFormatContext = nullptr;
    AVCodecContext *pCodecContext = nullptr;
    AVStream *videoStream = nullptr;
    AVStream *audioStream = nullptr;
    AVCodecContext *pAudioCodecContext = nullptr;
    AVFrame *pFrameYUV = nullptr;
    AVPacket *pPacket = nullptr;
    SwsContext *swsCtx = nullptr;
    int frameCounter = 0;
};

#endif // FFMPEGENCODER_H
