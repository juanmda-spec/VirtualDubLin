#include "CudaFilter.h"
#include <iostream>

// CUDA Kernel to convert RGB24 to Grayscale
__global__ void grayscaleKernel(const uint8_t* d_in, uint8_t* d_out, int width, int height) {
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;

    if (x < width && y < height) {
        int idx = (y * width + x) * 3;

        uint8_t r = d_in[idx];
        uint8_t g = d_in[idx + 1];
        uint8_t b = d_in[idx + 2];

        // standard grayscale conversion
        uint8_t gray = (uint8_t)(0.299f * r + 0.587f * g + 0.114f * b);

        d_out[idx] = gray;
        d_out[idx + 1] = gray;
        d_out[idx + 2] = gray;
    }
}

CudaFilter::CudaFilter()
    : m_width(0), m_height(0), m_bufferSize(0), d_in_data(nullptr), d_out_data(nullptr), m_initialized(false)
{
}

CudaFilter::~CudaFilter() {
    cleanup();
}

void CudaFilter::cleanup() {
    if (m_initialized) {
        cudaFree(d_in_data);
        cudaFree(d_out_data);
        d_in_data = nullptr;
        d_out_data = nullptr;
        m_initialized = false;
    }
}

bool CudaFilter::init(int width, int height) {
    cleanup();

    m_width = width;
    m_height = height;
    m_bufferSize = width * height * 3 * sizeof(uint8_t);

    cudaError_t err1 = cudaMalloc((void**)&d_in_data, m_bufferSize);
    cudaError_t err2 = cudaMalloc((void**)&d_out_data, m_bufferSize);

    if (err1 != cudaSuccess || err2 != cudaSuccess) {
        std::cerr << "CUDA malloc failed!" << std::endl;
        cleanup();
        return false;
    }

    m_initialized = true;
    return true;
}

bool CudaFilter::processFrame(const uint8_t* in_data, uint8_t* out_data) {
    if (!m_initialized) return false;

    // 1. Copy data from Host (CPU) to Device (GPU)
    cudaError_t err = cudaMemcpy(d_in_data, in_data, m_bufferSize, cudaMemcpyHostToDevice);
    if (err != cudaSuccess) return false;

    // 2. Setup execution parameters
    dim3 threadsPerBlock(16, 16);
    dim3 blocksPerGrid((m_width + threadsPerBlock.x - 1) / threadsPerBlock.x,
                       (m_height + threadsPerBlock.y - 1) / threadsPerBlock.y);

    // 3. Launch kernel
    grayscaleKernel<<<blocksPerGrid, threadsPerBlock>>>(d_in_data, d_out_data, m_width, m_height);

    // Wait for GPU to finish
    cudaDeviceSynchronize();

    // 4. Copy data from Device (GPU) to Host (CPU)
    err = cudaMemcpy(out_data, d_out_data, m_bufferSize, cudaMemcpyDeviceToHost);
    if (err != cudaSuccess) return false;

    return true;
}
