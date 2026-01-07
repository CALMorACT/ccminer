// Minimal stubs for testing SHA256d without full ccminer framework

#include <cuda_runtime.h>
#include <stdint.h>

// Forward declarations of CUDA functions (C++ linkage)
void sha256d_init(int thr_id);
void sha256d_free(int thr_id);
void sha256d_setBlock_80(uint32_t *pdata, uint32_t *ptarget);
void sha256d_hash_80(int thr_id, uint32_t threads, uint32_t startNonce, uint32_t *resNonces);

// Stub for cuda_get_arch (C++ linkage, called from cuda_sha256d.cu)
int cuda_get_arch(int thr_id)
{
    cudaDeviceProp props;
    cudaGetDeviceProperties(&props, 0);
    return props.major * 10 + props.minor;
}

// C linkage wrappers for test program
extern "C"
{

    void sha256d_init_c(int thr_id) { sha256d_init(thr_id); }
    void sha256d_free_c(int thr_id) { sha256d_free(thr_id); }
    void sha256d_setBlock_80_c(uint32_t *pdata, uint32_t *ptarget) { sha256d_setBlock_80(pdata, ptarget); }
    void sha256d_hash_80_c(int thr_id, uint32_t threads, uint32_t startNonce, uint32_t *resNonces)
    {
        sha256d_hash_80(thr_id, threads, startNonce, resNonces);
    }

} // extern "C"
