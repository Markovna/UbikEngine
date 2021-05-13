#include <cstdint>

namespace gfx::static_config {

constexpr static const uint16_t kAttributesCapacity = 16;
constexpr static const uint16_t kCamerasCapacity = 16;
constexpr static const uint16_t kTextureSlotsCapacity = 64;
constexpr static const uint16_t kFrameBufferMaxAttachments = 8;
constexpr static const uint16_t kIndexBuffersCapacity = 1024;
constexpr static const uint16_t kVertexBuffersCapacity = 1024;
constexpr static const uint16_t kFrameBuffersCapacity = 256;
constexpr static const uint16_t kUniformsCapacity = 2048;
constexpr static const uint16_t kShadersCapacity = 1024;
constexpr static const uint16_t kTexturesCapacity = 1024;
constexpr static const uint32_t kMaxDrawCallsCount = 2048;
constexpr static const uint16_t kMaxUniformsPerDrawCall = 128;
constexpr static const uint16_t kMaxFrameCommandsCount = 2048;

};