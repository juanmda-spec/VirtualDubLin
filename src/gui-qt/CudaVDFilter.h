#ifndef CUDA_VDFILTER_H
#define CUDA_VDFILTER_H

#include <vd2/plugin/vdvideofactory.h>
#include "CudaFilter.h"

// Definition of a Native VirtualDub Video Filter that delegates to CUDA
class VDCudaFilter : public VDFilter {
public:
    VDCudaFilter() {}
    ~VDCudaFilter() override {
        cudaFilter.cleanup();
    }

    // Core VD Filter API implementations
    virtual uint32 GetParams() { return 0; }
    virtual void Start() {
        cudaFilter.init(m_width, m_height);
    }
    virtual void Run() {
        // Run the filter using CudaFilter
        if (mpSource && mpDest) {
            cudaFilter.processFrame((const uint8_t*)mpSource, (uint8_t*)mpDest);
        }
    }

protected:
    void* mpSource = nullptr;
    void* mpDest = nullptr;
    virtual void End() {}

private:
    CudaFilter cudaFilter;
    int m_width = 0;
    int m_height = 0;
};

// VDFilterDefinition structure to register this filter in VD's engine
extern VDFilterDefinition fdef_cuda_grayscale;

#endif // CUDA_VDFILTER_H
