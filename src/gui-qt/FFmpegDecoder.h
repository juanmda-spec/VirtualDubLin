#ifndef FFMPEGDECODER_H
#define FFMPEGDECODER_H

#include <QString>
#include <QImage>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
}

class FFmpegDecoder {
public:
    FFmpegDecoder();
    ~FFmpegDecoder();

    bool openFile(const QString &filename);
    bool decodeNextFrame(QImage &outImage);
    bool decodeNextAudioFrame(uint8_t* audioBuffer, int& outSize);
    void close();

    int getVideoWidth() const { return pCodecContext ? pCodecContext->width : 0; }
    int getVideoHeight() const { return pCodecContext ? pCodecContext->height : 0; }

private:
    AVFormatContext *pFormatContext = nullptr;
    AVCodecContext *pCodecContext = nullptr;
    int videoStreamIndex = -1;
    AVFrame *pFrame = nullptr;
    AVFrame *pFrameRGB = nullptr;
    AVFrame *pAudioFrame = nullptr;
    AVCodecContext *pAudioCodecContext = nullptr;
    int audioStreamIndex = -1;
    AVPacket *pPacket = nullptr;
    SwsContext *swsCtx = nullptr;
    uint8_t *buffer = nullptr;
};

#endif // FFMPEGDECODER_H
