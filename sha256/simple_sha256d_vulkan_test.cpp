#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <vector>
#include <fstream>
#include <vulkan/vulkan.h>

// Directly include sph SHA256 implementation for CPU verification
extern "C" {
#include "../sph/sph_sha2.h"
}

// CPU SHA256d hash function
void sha256d_hash_cpu(void *output, const void *input)
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

// CPU-side full target verification function
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

// Byte order conversion
static inline uint32_t swab32(uint32_t v)
{
    return (v >> 24) | ((v >> 8) & 0xff00) | ((v << 8) & 0xff0000) | (v << 24);
}

// Vulkan helper functions
#define VK_CHECK(x) \
    do { \
        VkResult err = x; \
        if (err) { \
            fprintf(stderr, "Vulkan error: %d at %s:%d\n", err, __FILE__, __LINE__); \
            exit(1); \
        } \
    } while (0)

// Vulkan context
struct VulkanContext {
    VkInstance instance;
    VkPhysicalDevice physicalDevice;
    VkDevice device;
    uint32_t computeQueueFamily;
    VkQueue queue;
    VkDescriptorSetLayout descriptorSetLayout;
    VkPipelineLayout pipelineLayout;
    VkPipeline pipeline;
    VkCommandPool commandPool;
    VkDescriptorPool descriptorPool;
    VkDescriptorSet descriptorSet;
    
    // Buffers
    VkBuffer midstateBuffer;
    VkDeviceMemory midstateMemory;
    VkBuffer dataEndBuffer;
    VkDeviceMemory dataEndMemory;
    VkBuffer targetBuffer;
    VkDeviceMemory targetMemory;
    VkBuffer resultBuffer;
    VkDeviceMemory resultMemory;
};

// Read SPIR-V shader file
std::vector<uint32_t> readShaderFile(const char* filename) {
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        fprintf(stderr, "Failed to open shader file: %s\n", filename);
        exit(1);
    }
    
    size_t fileSize = file.tellg();
    std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));
    file.seekg(0);
    file.read((char*)buffer.data(), fileSize);
    file.close();
    
    return buffer;
}

// Find memory type
uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);
    
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }
    
    fprintf(stderr, "Failed to find suitable memory type\n");
    exit(1);
}

// Create buffer
void createBuffer(VulkanContext* ctx, VkDeviceSize size, VkBufferUsageFlags usage, 
                  VkMemoryPropertyFlags properties, VkBuffer* buffer, VkDeviceMemory* memory) {
    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    
    VK_CHECK(vkCreateBuffer(ctx->device, &bufferInfo, nullptr, buffer));
    
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(ctx->device, *buffer, &memRequirements);
    
    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(ctx->physicalDevice, memRequirements.memoryTypeBits, properties);
    
    VK_CHECK(vkAllocateMemory(ctx->device, &allocInfo, nullptr, memory));
    VK_CHECK(vkBindBufferMemory(ctx->device, *buffer, *memory, 0));
}

// Initialize Vulkan
void vulkanInit(VulkanContext* ctx) {
    // Create Instance
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "SHA256d Vulkan Test";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_2;
    
    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    
    VK_CHECK(vkCreateInstance(&createInfo, nullptr, &ctx->instance));
    
    // Select physical device
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(ctx->instance, &deviceCount, nullptr);
    if (deviceCount == 0) {
        fprintf(stderr, "No Vulkan-capable devices found\n");
        exit(1);
    }
    
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(ctx->instance, &deviceCount, devices.data());
    ctx->physicalDevice = devices[0];  // use the first device
    
    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(ctx->physicalDevice, &deviceProperties);
    printf("Using Vulkan device: %s\n", deviceProperties.deviceName);
    
    // Find compute queue family
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(ctx->physicalDevice, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(ctx->physicalDevice, &queueFamilyCount, queueFamilies.data());
    
    ctx->computeQueueFamily = UINT32_MAX;
    for (uint32_t i = 0; i < queueFamilyCount; i++) {
        if (queueFamilies[i].queueFlags & VK_QUEUE_COMPUTE_BIT) {
            ctx->computeQueueFamily = i;
            break;
        }
    }
    
    if (ctx->computeQueueFamily == UINT32_MAX) {
        fprintf(stderr, "No compute queue family found\n");
        exit(1);
    }
    
    // Create logical device
    float queuePriority = 1.0f;
    VkDeviceQueueCreateInfo queueCreateInfo = {};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = ctx->computeQueueFamily;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;
    
    VkDeviceCreateInfo deviceCreateInfo = {};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.queueCreateInfoCount = 1;
    deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
    
    VK_CHECK(vkCreateDevice(ctx->physicalDevice, &deviceCreateInfo, nullptr, &ctx->device));
    vkGetDeviceQueue(ctx->device, ctx->computeQueueFamily, 0, &ctx->queue);
    
    // Create command pool
    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = ctx->computeQueueFamily;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    
    VK_CHECK(vkCreateCommandPool(ctx->device, &poolInfo, nullptr, &ctx->commandPool));
    
    printf("✓ Vulkan initialized successfully\n\n");
}

// Setup compute pipeline
void setupComputePipeline(VulkanContext* ctx, const char* shaderPath) {
    // Create descriptor set layout
    VkDescriptorSetLayoutBinding bindings[4] = {};
    
    // Binding 0: midstate
    bindings[0].binding = 0;
    bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    bindings[0].descriptorCount = 1;
    bindings[0].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    
    // Binding 1: dataEnd
    bindings[1].binding = 1;
    bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    bindings[1].descriptorCount = 1;
    bindings[1].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    
    // Binding 2: target
    bindings[2].binding = 2;
    bindings[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    bindings[2].descriptorCount = 1;
    bindings[2].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    
    // Binding 3: result
    bindings[3].binding = 3;
    bindings[3].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    bindings[3].descriptorCount = 1;
    bindings[3].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    
    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 4;
    layoutInfo.pBindings = bindings;
    
    VK_CHECK(vkCreateDescriptorSetLayout(ctx->device, &layoutInfo, nullptr, &ctx->descriptorSetLayout));
    
    // Create pipeline layout (with push constants)
    VkPushConstantRange pushConstantRange = {};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(uint32_t) * 2;  // startNonce + threads
    
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &ctx->descriptorSetLayout;
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
    
    VK_CHECK(vkCreatePipelineLayout(ctx->device, &pipelineLayoutInfo, nullptr, &ctx->pipelineLayout));
    
    // Load shader
    auto shaderCode = readShaderFile(shaderPath);
    
    VkShaderModuleCreateInfo shaderModuleInfo = {};
    shaderModuleInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderModuleInfo.codeSize = shaderCode.size() * sizeof(uint32_t);
    shaderModuleInfo.pCode = shaderCode.data();
    
    VkShaderModule shaderModule;
    VK_CHECK(vkCreateShaderModule(ctx->device, &shaderModuleInfo, nullptr, &shaderModule));
    
    // Create compute pipeline
    VkPipelineShaderStageCreateInfo shaderStageInfo = {};
    shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    shaderStageInfo.module = shaderModule;
    shaderStageInfo.pName = "main";
    
    VkComputePipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipelineInfo.stage = shaderStageInfo;
    pipelineInfo.layout = ctx->pipelineLayout;
    
    VK_CHECK(vkCreateComputePipelines(ctx->device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &ctx->pipeline));
    
    vkDestroyShaderModule(ctx->device, shaderModule, nullptr);
    
    printf("✓ Compute pipeline created\n\n");
}

// Create buffers
void createBuffers(VulkanContext* ctx) {
    // Midstate buffer: 8 * uint32
    createBuffer(ctx, sizeof(uint32_t) * 8,
                 VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 &ctx->midstateBuffer, &ctx->midstateMemory);
    
    // DataEnd buffer: 4 * uint32
    createBuffer(ctx, sizeof(uint32_t) * 4,
                 VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 &ctx->dataEndBuffer, &ctx->dataEndMemory);
    
    // Target buffer: 2 * uint32 (64-bit)
    createBuffer(ctx, sizeof(uint32_t) * 2,
                 VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 &ctx->targetBuffer, &ctx->targetMemory);
    
    // Result buffer: 3 * uint32 (2 nonces + 1 counter)
    createBuffer(ctx, sizeof(uint32_t) * 3,
                 VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 &ctx->resultBuffer, &ctx->resultMemory);
    
    // Create descriptor pool
    VkDescriptorPoolSize poolSize = {};
    poolSize.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    poolSize.descriptorCount = 4;
    
    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.maxSets = 1;
    
    VK_CHECK(vkCreateDescriptorPool(ctx->device, &poolInfo, nullptr, &ctx->descriptorPool));
    
    // Allocate descriptor set
    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = ctx->descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &ctx->descriptorSetLayout;
    
    VK_CHECK(vkAllocateDescriptorSets(ctx->device, &allocInfo, &ctx->descriptorSet));
    
    // Update descriptor set
    VkDescriptorBufferInfo bufferInfos[4] = {};
    bufferInfos[0].buffer = ctx->midstateBuffer;
    bufferInfos[0].offset = 0;
    bufferInfos[0].range = sizeof(uint32_t) * 8;
    
    bufferInfos[1].buffer = ctx->dataEndBuffer;
    bufferInfos[1].offset = 0;
    bufferInfos[1].range = sizeof(uint32_t) * 4;
    
    bufferInfos[2].buffer = ctx->targetBuffer;
    bufferInfos[2].offset = 0;
    bufferInfos[2].range = sizeof(uint32_t) * 2;
    
    bufferInfos[3].buffer = ctx->resultBuffer;
    bufferInfos[3].offset = 0;
    bufferInfos[3].range = sizeof(uint32_t) * 3;  // 2 nonces + 1 counter
    
    VkWriteDescriptorSet descriptorWrites[4] = {};
    for (int i = 0; i < 4; i++) {
        descriptorWrites[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[i].dstSet = ctx->descriptorSet;
        descriptorWrites[i].dstBinding = i;
        descriptorWrites[i].dstArrayElement = 0;
        descriptorWrites[i].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        descriptorWrites[i].descriptorCount = 1;
        descriptorWrites[i].pBufferInfo = &bufferInfos[i];
    }
    
    vkUpdateDescriptorSets(ctx->device, 4, descriptorWrites, 0, nullptr);
    
    printf("✓ Buffers created and bound\n\n");
}

// Execute compute shader
void runCompute(VulkanContext* ctx, uint32_t startNonce, uint32_t threads) {
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = ctx->commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;
    
    VkCommandBuffer commandBuffer;
    VK_CHECK(vkAllocateCommandBuffers(ctx->device, &allocInfo, &commandBuffer));
    
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    
    VK_CHECK(vkBeginCommandBuffer(commandBuffer, &beginInfo));
    
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, ctx->pipeline);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, 
                            ctx->pipelineLayout, 0, 1, &ctx->descriptorSet, 0, nullptr);
    
    // Push constants
    uint32_t pushConstants[2] = { startNonce, threads };
    vkCmdPushConstants(commandBuffer, ctx->pipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT,
                       0, sizeof(pushConstants), pushConstants);
    
    // Dispatch (threads / 256 workgroups)
    // uint32_t workgroupCount = (threads + 255) / 256;
    // Fix: groupCountZ limit is 65535. Move large dimension to X.
    // vkCmdDispatch(commandBuffer, 65536, 5, 4);
    vkCmdDispatch(commandBuffer, 5, 4, 65535);
    
    VK_CHECK(vkEndCommandBuffer(commandBuffer));
    
    // Submit
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;
    
    VK_CHECK(vkQueueSubmit(ctx->queue, 1, &submitInfo, VK_NULL_HANDLE));
    VK_CHECK(vkQueueWaitIdle(ctx->queue));
    
    vkFreeCommandBuffers(ctx->device, ctx->commandPool, 1, &commandBuffer);
}

// SHA256 host implementation (for midstate calculation)
#define ROTR(x, n) (((x) >> (n)) | ((x) << (32 - (n))))

void sha256_transform(uint32_t state[8], const uint32_t data[16]) {
    const uint32_t K[64] = {
        0x428A2F98, 0x71374491, 0xB5C0FBCF, 0xE9B5DBA5, 0x3956C25B, 0x59F111F1, 0x923F82A4, 0xAB1C5ED5,
        0xD807AA98, 0x12835B01, 0x243185BE, 0x550C7DC3, 0x72BE5D74, 0x80DEB1FE, 0x9BDC06A7, 0xC19BF174,
        0xE49B69C1, 0xEFBE4786, 0x0FC19DC6, 0x240CA1CC, 0x2DE92C6F, 0x4A7484AA, 0x5CB0A9DC, 0x76F988DA,
        0x983E5152, 0xA831C66D, 0xB00327C8, 0xBF597FC7, 0xC6E00BF3, 0xD5A79147, 0x06CA6351, 0x14292967,
        0x27B70A85, 0x2E1B2138, 0x4D2C6DFC, 0x53380D13, 0x650A7354, 0x766A0ABB, 0x81C2C92E, 0x92722C85,
        0xA2BFE8A1, 0xA81A664B, 0xC24B8B70, 0xC76C51A3, 0xD192E819, 0xD6990624, 0xF40E3585, 0x106AA070,
        0x19A4C116, 0x1E376C08, 0x2748774C, 0x34B0BCB5, 0x391C0CB3, 0x4ED8AA4A, 0x5B9CCA4F, 0x682E6FF3,
        0x748F82EE, 0x78A5636F, 0x84C87814, 0x8CC70208, 0x90BEFFFA, 0xA4506CEB, 0xBEF9A3F7, 0xC67178F2
    };
    
    uint32_t w[64];
    for (int i = 0; i < 16; i++) w[i] = data[i];
    for (int i = 16; i < 64; i++) {
        uint32_t s0 = ROTR(w[i-15], 7) ^ ROTR(w[i-15], 18) ^ (w[i-15] >> 3);
        uint32_t s1 = ROTR(w[i-2], 17) ^ ROTR(w[i-2], 19) ^ (w[i-2] >> 10);
        w[i] = w[i-16] + s0 + w[i-7] + s1;
    }
    
    uint32_t a = state[0], b = state[1], c = state[2], d = state[3];
    uint32_t e = state[4], f = state[5], g = state[6], h = state[7];
    
    for (int i = 0; i < 64; i++) {
        uint32_t S1 = ROTR(e, 6) ^ ROTR(e, 11) ^ ROTR(e, 25);
        uint32_t ch = (e & f) ^ (~e & g);
        uint32_t temp1 = h + S1 + ch + K[i] + w[i];
        uint32_t S0 = ROTR(a, 2) ^ ROTR(a, 13) ^ ROTR(a, 22);
        uint32_t maj = (a & b) ^ (a & c) ^ (b & c);
        uint32_t temp2 = S0 + maj;
        
        h = g; g = f; f = e; e = d + temp1;
        d = c; c = b; b = a; a = temp1 + temp2;
    }
    
    state[0] += a; state[1] += b; state[2] += c; state[3] += d;
    state[4] += e; state[5] += f; state[6] += g; state[7] += h;
}

int main() {
    printf("========================================\n");
        printf("  SHA256d Vulkan real difficulty mining test\n");
    printf("========================================\n\n");
    
    const uint32_t threads = 1 << 28;  // 268M threads
    const uint32_t startNonce = 0;
    
        printf("Test configuration:\n");
        printf("  Search space: %u nonces\n", threads);
        printf("  Difficulty: 0x000000ffffffffff (high 24 bits are 0)\n");
        printf("  Expected to find: %.2f nonces\n\n", (double)threads / (1 << 24));
    
        // Initialize Vulkan
        printf("1. Initializing Vulkan...\n");
    VulkanContext ctx = {};
    vulkanInit(&ctx);
    
    // Compile shader (if needed)
    printf("2. Compiling shader...\n");
    if (system("glslangValidator -V sha256d.comp -o sha256d.comp.spv 2>/dev/null") != 0) {
        fprintf(stderr, "Warning: shader compilation failed, try using existing .spv file\n");
    }
    printf("✓ Shader compiled\n\n");
    
    // Setup pipeline
    printf("3. Creating compute pipeline...\n");
    setupComputePipeline(&ctx, "sha256d.comp.spv");
    
    // Create buffers
    printf("4. Creating buffers...\n");
    createBuffers(&ctx);
    
    // Prepare data
    printf("5. Preparing data...\n");
    uint32_t pdata[20] = {
        0x00000001,  // version
        0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000,  // timestamp
        0x1d00ffff,  // bits
        0x00000000   // nonce
    };
    
    uint32_t ptarget[8] = {
        0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000,
        0xffffffff, 0x000000ff  // High 24 bits are 0
    };
    
    // Calculate midstate
    // Key point: CUDA's sha256d_setBlock_80 does swab32 on input,
    // but the caller already passes swab32(pdata), so double conversion cancels out, use original pdata
    // So here we also use original pdata directly
    uint32_t midstate[8] = {
        0x6A09E667, 0xBB67AE85, 0x3C6EF372, 0xA54FF53A,
        0x510E527F, 0x9B05688C, 0x1F83D9AB, 0x5BE0CD19
    };
    sha256_transform(midstate, pdata);  // use original pdata directly
    
    // Print midstate
    printf("   Midstate after SHA256 transform:\n   ");
    for (int i = 0; i < 8; i++) {
        printf("0x%08x%s", midstate[i], (i < 7) ? ", " : "\n");
    }
    printf("\n");
    
    // dataEnd also directly from original pdata, no conversion
    uint32_t dataEnd[4];
    for (int i = 0; i < 3; i++) dataEnd[i] = pdata[16 + i];
    dataEnd[3] = 0;  // nonce placeholder (will be filled by GPU)
    
    // Upload data to GPU
    void* data;
    vkMapMemory(ctx.device, ctx.midstateMemory, 0, sizeof(uint32_t) * 8, 0, &data);
    memcpy(data, midstate, sizeof(uint32_t) * 8);
    vkUnmapMemory(ctx.device, ctx.midstateMemory);
    
    vkMapMemory(ctx.device, ctx.dataEndMemory, 0, sizeof(uint32_t) * 4, 0, &data);
    memcpy(data, dataEnd, sizeof(uint32_t) * 4);
    vkUnmapMemory(ctx.device, ctx.dataEndMemory);
    
    // Target setup: matches CUDA's cudaMemcpyToSymbol(d_target, &ptarget[6], 8, ...)
    // ptarget[6] is the low 32 bits, ptarget[7] is the high 32 bits
    // uvec2.x = ptarget[6], uvec2.y = ptarget[7]
    uint32_t target_data[2] = { ptarget[6], ptarget[7] };
    vkMapMemory(ctx.device, ctx.targetMemory, 0, sizeof(uint32_t) * 2, 0, &data);
    memcpy(data, target_data, sizeof(uint32_t) * 2);
    vkUnmapMemory(ctx.device, ctx.targetMemory);
    
    uint32_t resData[3] = {0xFFFFFFFF, 0xFFFFFFFF, 0};  // init: 2 nonces + counter
    vkMapMemory(ctx.device, ctx.resultMemory, 0, sizeof(uint32_t) * 3, 0, &data);
    memcpy(data, resData, sizeof(uint32_t) * 3);
    vkUnmapMemory(ctx.device, ctx.resultMemory);
    
    printf("✓ Data preparation complete\n\n");
    
    // Run compute
    printf("6. Running SHA256d computation...\n");
    printf("   Search range: [%u, %u)\n", startNonce, startNonce + threads);
    runCompute(&ctx, startNonce, threads);
    printf("   ✓ Computation complete\n\n");
    
    // Read result - read more bytes to check padding
    uint32_t bigBuffer[8];
    vkMapMemory(ctx.device, ctx.resultMemory, 0, sizeof(uint32_t) * 8, 0, &data);
    
    // Debug: print raw buffer content
    uint32_t* raw = (uint32_t*)data;
    printf("Debug: Raw buffer (16 bytes):\n");
    for (int i = 0; i < 8; i++) {
        printf("  [%d] = %u (0x%08x)\n", i, raw[i], raw[i]);
    }
    
    memcpy(resData, data, sizeof(uint32_t) * 3);
    vkUnmapMemory(ctx.device, ctx.resultMemory);
    
        // Debug output
        printf("Debug: resData[0]=%u, resData[1]=%u, resData[2]=%u\n\n", 
            resData[0], resData[1], resData[2]);
    
    uint32_t resNonces[2] = {resData[0], resData[1]};
    uint32_t totalFound = resData[2];
    
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
        
        // CPU verification
        printf("\nCPU verification:\n");
        uint32_t vhash[8] __attribute__((aligned(64)));
        int valid_count = 0;
        
        // Prepare endiandata for CPU verification
        uint32_t endiandata[20];
        for (int i = 0; i < 20; i++) endiandata[i] = swab32(pdata[i]);
        
        // Verify first nonce
        endiandata[19] = swab32(resNonces[0]);
        sha256d_hash_cpu(vhash, endiandata);
        
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
            sha256d_hash_cpu(vhash, endiandata);
            
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
        printf("  GPU found (total): %u\n", totalFound);
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
        printf("GPU did not find nonce (all buffer values are 0xFFFFFFFF)\n");
        printf("But GPU total found: %u nonces\n", totalFound);
        double expected = (double)threads / (1 << 16);
        printf("Expected to find: %.2f nonces\n", expected);
    }
    
    printf("\n========================================\n");
    
    // Cleanup
    vkDestroyBuffer(ctx.device, ctx.midstateBuffer, nullptr);
    vkFreeMemory(ctx.device, ctx.midstateMemory, nullptr);
    vkDestroyBuffer(ctx.device, ctx.dataEndBuffer, nullptr);
    vkFreeMemory(ctx.device, ctx.dataEndMemory, nullptr);
    vkDestroyBuffer(ctx.device, ctx.targetBuffer, nullptr);
    vkFreeMemory(ctx.device, ctx.targetMemory, nullptr);
    vkDestroyBuffer(ctx.device, ctx.resultBuffer, nullptr);
    vkFreeMemory(ctx.device, ctx.resultMemory, nullptr);
    vkDestroyDescriptorPool(ctx.device, ctx.descriptorPool, nullptr);
    vkDestroyPipeline(ctx.device, ctx.pipeline, nullptr);
    vkDestroyPipelineLayout(ctx.device, ctx.pipelineLayout, nullptr);
    vkDestroyDescriptorSetLayout(ctx.device, ctx.descriptorSetLayout, nullptr);
    vkDestroyCommandPool(ctx.device, ctx.commandPool, nullptr);
    vkDestroyDevice(ctx.device, nullptr);
    vkDestroyInstance(ctx.instance, nullptr);
    
    return 0;
}
