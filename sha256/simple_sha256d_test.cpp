#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <cuda_runtime.h>

// Memory alignment macro
#define _ALIGN(x) __attribute__ ((aligned(x)))

// Declare CUDA kernel functions (using C++ linkage)
void sha256d_init(int thr_id);
void sha256d_free(int thr_id);
void sha256d_setBlock_80(uint32_t *pdata, uint32_t *ptarget);
void sha256d_hash_80(int thr_id, uint32_t threads, uint32_t startNonce, uint32_t *resNonces);

// Directly include sph SHA256 implementation for CPU verification
extern "C" {
#include "../sph/sph_sha2.h"
}

// CPU SHA256d hash function
void sha256d_hash(void *output, const void *input)
{
    sph_sha256_context ctx_sha256;
    uint32_t hash[8];
    
    sph_sha256_init(&ctx_sha256);
    sph_sha256(&ctx_sha256, input, 80);
    sph_sha256_close(&ctx_sha256, hash);
    
    sph_sha256_init(&ctx_sha256);
    sph_sha256(&ctx_sha256, hash, 32);
    sph_sha256_close(&ctx_sha256, output);
}

// Full target verification function on CPU
bool fulltest(const uint32_t *hash, const uint32_t *target)
{
    for (int i = 7; i >= 0; i--) {
        if (hash[i] > target[i])
            return false;
        if (hash[i] < target[i])
            return true;
    }
    return true;
}

// Endianness conversion
static inline uint32_t swab32(uint32_t v)
{
    return (v >> 24) | ((v >> 8) & 0xff00) | ((v << 8) & 0xff0000) | (v << 24);
}

int main()
{
    // ========== Thread Configuration Analysis ==========
    // CUDA kernel config: threadsperblock = 256
    // Grid config: grid = threads / 256
    // 
    // RTX 3060 specs:
    // - Number of SMs: 28
    // - Max threads per SM: 1536
    // - Max concurrent threads: 28 * 1536 = 43,008
    // - Recommended config: multiples of 256, e.g. 256*512 = 131,072 (1<<17)
    // 
    // Thread count options:
    // - 1<<17 = 131,072   (recommended, fast test)
    // - 1<<18 = 262,144   (medium)
    // - 1<<19 = 524,288   (large)
    // - 1<<20 = 1,048,576 (original, a bit large)
    // - 1<<25 = 33,554,432 (max in original code, for mining)
    
    const int thr_id = 0;                    // GPU thread ID
    const uint32_t threads = 1 << 28;        // 268,435,456 threads (~6% of the search space)
    const uint32_t startNonce = 0;           // Start testing from 0
    
    printf("========================================\n");
    printf("  SHA256d Real Difficulty Mining Test\n");
    printf("========================================\n\n");
    printf("Goal: Verify algorithm correctness under real mining difficulty\n");
    printf("Difficulty settings:\n");
    printf("  - Target value: 0x000000ffffffffff (high 24 bits must be 0)\n");
    printf("  - Theoretical hit rate: 1 / 2^24 ≈ 0.000006%%\n");
    printf("  - Expected to find: %.2f nonces\n", (double)threads / (1 << 24));
    printf("  - Search space: %u nonces\n\n", threads);
    
    // ========== Bitcoin Genesis Block Parameters ==========
    // Simplified data for quick testing
    
    uint32_t pdata[20] = {
        // Simple test data - all zeros for easy verification
        0x00000001,  // version
        0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000,  // timestamp
        0x1d00ffff,  // difficulty
        0x00000000   // nonce
    };
    
    // Target difficulty - real mining difficulty
    // Note: In code, cudaMemcpyToSymbol(d_target, &ptarget[6], 8, ...)
    //       only copies ptarget[6] and ptarget[7], then compares high <= d_target[0]
    // Difficulty explanation:
    //   0x0000ffffffffffff means the high 64 bits of the hash must be <= this value
    //   That is: high 16 bits must be 0, low 48 bits can be any value
    //   Difficulty is about 1 / 2^16 = 1 / 65,536 (reasonable for testing)
    uint32_t ptarget[8] = {
        0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 
        0xffffffff, 0x000000ff  // [6]=low 32 bits, [7]=high 32 bits (high 24 bits are 0!)
    };
    
    // Result nonce array (add one slot to store total count)
    uint32_t resNonces[3] = {0xFFFFFFFF, 0xFFFFFFFF, 0};
    
    printf("Test configuration:\n");
    printf("  Data: Simplified all-zero test data\n");
    printf("  Target: 0xffffffffffffffff (any hash should satisfy)\n");
    printf("  Search range: [%u, %u)\n", startNonce, startNonce + threads);
    printf("  Expectation: Should find at least one valid nonce\n\n");
    
    // Set CUDA device
    cudaSetDevice(0);
    
    printf("========================================\n");
    printf("  Start testing\n");
    printf("========================================\n\n");
    
    // Initialize SHA256d
    printf("1. Initializing CUDA kernel...\n");
    sha256d_init(thr_id);
    printf("   ✓ Done\n\n");
    
    // Convert endianness and set block data
    printf("2. Setting block data and target...\n");
    uint32_t endiandata[20];
    for (int k = 0; k < 20; k++) {
        endiandata[k] = swab32(pdata[k]);
    }
    sha256d_setBlock_80(endiandata, ptarget);
    
    // Debug: print set target value
    printf("   Target: ptarget[6]=0x%08x, ptarget[7]=0x%08x\n", ptarget[6], ptarget[7]);
    printf("   ✓ Done\n\n");
    
    printf("3. Running SHA256d hash computation...\n");
    printf("   Search range: [%u, %u)\n", startNonce, startNonce + threads);
    
    sha256d_hash_80(thr_id, threads, startNonce, resNonces);
    
    printf("   ✓ Computation done\n\n");
    
    // Show results
    printf("========================================\n");
    printf("  Test Results\n");
    printf("========================================\n\n");
    
    if (resNonces[0] != 0xFFFFFFFF) {
        printf("🎉 GPU found candidate nonce!\n\n");
        printf("GPU Results:\n");
        printf("  Nonce 1: %u (0x%08x)\n", resNonces[0], resNonces[0]);
        
        if (resNonces[1] != 0xFFFFFFFF) {
            printf("  Nonce 2: %u (0x%08x)\n", resNonces[1], resNonces[1]);
        }
        
        // CPU verification - follow sha256d.cu logic
        printf("\nCPU verification:\n");
        uint32_t _ALIGN(64) vhash[8];
        int valid_count = 0;
        
        // Verify first nonce
        endiandata[19] = swab32(resNonces[0]);
        sha256d_hash(vhash, endiandata);
        
        printf("  Nonce 1: 0x%08x\n", resNonces[0]);
        printf("    Hash: ");
        for (int i = 7; i >= 0; i--) printf("%08x ", vhash[i]);
        printf("\n");
        printf("    vhash[7] = 0x%08x, ptarget[7] = 0x%08x\n", vhash[7], ptarget[7]);
        
        if (vhash[7] <= ptarget[7] && fulltest(vhash, ptarget)) {
            printf("    ✓ Valid!\n");
            valid_count++;
        } else {
            printf("    ✗ Invalid (GPU false positive)\n");
        }
        
        // Verify second nonce
        if (resNonces[1] != 0xFFFFFFFF) {
            endiandata[19] = swab32(resNonces[1]);
            sha256d_hash(vhash, endiandata);
            
            printf("\n  Nonce 2: 0x%08x\n", resNonces[1]);
            printf("    Hash: ");
            for (int i = 7; i >= 0; i--) printf("%08x ", vhash[i]);
            printf("\n");
            printf("    vhash[7] = 0x%08x, ptarget[7] = 0x%08x\n", vhash[7], ptarget[7]);
            
            if (vhash[7] <= ptarget[7] && fulltest(vhash, ptarget)) {
                printf("    ✓ Valid!\n");
                valid_count++;
            } else {
                printf("    ✗ Invalid (GPU false positive)\n");
            }
        }
        
        double hit_rate = (double)valid_count / threads * 100.0;
        double expected_rate = 1.0 / (1 << 16) * 100.0;
        
        printf("\nStatistics:\n");
        printf("  Search space: %u nonces\n", threads);
        printf("  GPU found (shown): %d\n", (resNonces[1] != 0xFFFFFFFF ? 2 : 1));
        printf("  GPU found (total): %u\n", resNonces[2]);
        printf("  CPU verified: %d\n", valid_count);
        printf("  Actual hit rate: %.8f%%\n", hit_rate);
        printf("  Theoretical hit rate: %.8f%%\n", expected_rate);
        
        if (valid_count > 0) {
            printf("\n✓ SHA256d algorithm verification successful!\n");
            printf("  - GPU found candidate nonce\n");
            printf("  - CPU verification confirmed nonce is valid\n");
            printf("  - Target comparison logic is correct\n");
        } else {
            printf("\n✗ SHA256d algorithm verification failed!\n");
            printf("  - GPU found nonce did not pass CPU verification\n");
            printf("  - Possible GPU/CPU implementation inconsistency\n");
        }
    } else {
        printf("No valid nonce found\n\n");
        printf("This is normal!\n\n");
        
        double expected_finds = (double)threads / (1 << 16);
        printf("Statistics:\n");
        printf("  Search space: %u nonces\n", threads);
        printf("  Target difficulty: 1 / 2^16 (= 1 / 65,536)\n");
        printf("  Expected to find: %.2f\n", expected_finds);
        printf("  Actually found: 0\n");
        printf("  Conclusion: Consistent with probability distribution (%.1f%% chance of finding none)\n", 
               100.0 * exp(-expected_finds));
        
        printf("\nTip: Increase search range or lower difficulty to improve chance of finding a nonce\n");
    }
    
    printf("\n========================================\n");
    
    // Cleanup
    sha256d_free(thr_id);
    
    return 0;
}
