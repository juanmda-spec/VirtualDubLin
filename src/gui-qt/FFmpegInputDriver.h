#ifndef FFMPEGINPUTDRIVER_H
#define FFMPEGINPUTDRIVER_H

#include "FFmpegDecoder.h"
#include <vd2/plugin/vdinputdriver.h>

class FFmpegVideoStreamSource : public IVDXVideoStreamSource {
public:
    FFmpegVideoStreamSource(FFmpegDecoder* dec) : m_decoder(dec) {}

    int VDXAPIENTRY AddRef() override { return 1; }
    int VDXAPIENTRY Release() override { return 1; }
    void *VDXAPIENTRY AsInterface(uint32 iid) override { return nullptr; }

    int VDXAPIENTRY GetError() override { return 0; }
    void VDXAPIENTRY ReadHeader(void *buffer, int size) override {}
    bool VDXAPIENTRY ReadFrame(sint64 frameNum, void *buffer, int bufsize, int& bytesRead, int& isKeyFrame) override {
        // Implementation that calls m_decoder->decodeNextFrame() and copies to buffer
        return false;
    }

private:
    FFmpegDecoder* m_decoder;
};

class FFmpegInputDriver : public IVDXInputFileDriver {
public:
    FFmpegInputDriver() {}
    virtual ~FFmpegInputDriver() {}

    int VDXAPIENTRY AddRef() override { return 1; }
    int VDXAPIENTRY Release() override { return 1; }
    void *VDXAPIENTRY AsInterface(uint32 iid) override { return nullptr; }

    void VDXAPIENTRY Init(const char *szFile, IVDXInputOptions *pOptions) override {
        decoder.openFile(QString::fromUtf8(szFile));
    }
    void VDXAPIENTRY Close() override {
        decoder.close();
    }

    int VDXAPIENTRY GetStreamCount() override { return 2; /* Video and Audio */ }
    IVDXStreamSource *VDXAPIENTRY GetStreamSource(int index) override {
        if (index == 0) return new FFmpegVideoStreamSource(&decoder);
        return nullptr; // Audio stream source placeholder
    }

    const char *VDXAPIENTRY GetError() override { return nullptr; }

private:
    FFmpegDecoder decoder;
};

#endif // FFMPEGINPUTDRIVER_H
