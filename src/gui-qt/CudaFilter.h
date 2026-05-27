#ifndef CUDAFILTER_H
#define CUDAFILTER_H

#include <stddef.h>
#include <stdint.h>

class CudaFilter {
public:
    CudaFilter();
    ~CudaFilter();

    bool init(int width, int height);

    // Applies a grayscale filter to RGB24 data
    // in_data and out_data must be pointing to Host memory, size must be width * height * 3
    bool processFrame(const uint8_t* in_data, uint8_t* out_data);

    void cleanup();

private:
    int m_width;
    int m_height;
    size_t m_bufferSize;

    uint8_t* d_in_data;
    uint8_t* d_out_data;
    bool m_initialized;
};

#endif // CUDAFILTER_H
