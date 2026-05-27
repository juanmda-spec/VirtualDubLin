#include "CudaFilter.h"
#include <cstring>
#include <iostream>

#ifndef HAS_CUDA

CudaFilter::CudaFilter() : m_width(0), m_height(0), m_bufferSize(0), d_in_data(nullptr), d_out_data(nullptr), m_initialized(false) {}

CudaFilter::~CudaFilter() {}

void CudaFilter::cleanup() { m_initialized = false; }

bool CudaFilter::init(int width, int height) {
    m_width = width;
    m_height = height;
    m_bufferSize = width * height * 3;
    m_initialized = true;
    return true;
}

bool CudaFilter::processFrame(const uint8_t* in_data, uint8_t* out_data) {
    if (!m_initialized) return false;

    // Fallback: Just copy the memory (or we could do a CPU grayscale here if we wanted)
    // For now, let's do a CPU grayscale so the user sees something happens!
    for (size_t i = 0; i < m_bufferSize; i += 3) {
        uint8_t r = in_data[i];
        uint8_t g = in_data[i+1];
        uint8_t b = in_data[i+2];
        uint8_t gray = (uint8_t)(0.299f * r + 0.587f * g + 0.114f * b);
        out_data[i] = gray;
        out_data[i+1] = gray;
        out_data[i+2] = gray;
    }

    return true;
}

#endif
