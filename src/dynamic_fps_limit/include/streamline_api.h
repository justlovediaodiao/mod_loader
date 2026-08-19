#pragma once

#include <stddef.h>
#include <stdint.h>

// Minimal Streamline ABI used by this mod. Only fields accessed by the hooks
// are declared for input structures owned by the game.
namespace streamline {

constexpr uint32_t FEATURE_DLSS_G = 1000;
constexpr uint32_t FEATURE_REFLEX = 3;
constexpr uint32_t STRUCT_VERSION_1 = 1;

enum class Result : int32_t {
    ok = 0,
};

struct StructType {
    uint32_t data1;
    uint16_t data2;
    uint16_t data3;
    uint8_t data4[8];
};

struct BaseStructure {
    BaseStructure* next{};
    StructType struct_type{};
    size_t struct_version{};
};

struct ViewportHandle : BaseStructure {
    explicit ViewportHandle(uint32_t viewport) {
        struct_type = {0x171b6435, 0x9b3c, 0x4fc8,
                       {0x99, 0x94, 0xfb, 0xe5, 0x25, 0x69, 0xaa, 0xa4}};
        struct_version = STRUCT_VERSION_1;
        value = viewport;
    }

private:
    uint32_t value{};
};

enum class DLSSGMode : uint32_t {
    off,
    on,
    automatic,
    dynamic,
};

// Only the stable prefix read by the hook is declared here.
struct DLSSGOptions : BaseStructure {
    DLSSGMode mode{DLSSGMode::off};
};

enum class ReflexMode : int32_t {
    off,
    low_latency,
    low_latency_with_boost,
};

struct ReflexOptions : BaseStructure {
    ReflexOptions() {
        struct_type = {0xf03af81a, 0x6d0b, 0x4902,
                       {0xa6, 0x51, 0xc4, 0x96, 0x5e, 0x21, 0x54, 0x34}};
        struct_version = STRUCT_VERSION_1;
    }

    ReflexMode mode{ReflexMode::off};
    uint32_t frame_limit_us{};
    bool use_markers_to_optimize{};
    uint16_t virtual_key{};
    uint32_t thread_id{};
};

using GetFeatureFunction =
    Result (*)(uint32_t feature, const char* function_name, void*& function);
using DLSSGSetOptions = Result (*)(const ViewportHandle& viewport,
                                   const DLSSGOptions& options);
using ReflexSetOptions = Result (*)(const ReflexOptions& options);

static_assert(sizeof(StructType) == 16);
static_assert(sizeof(Result) == 4);
static_assert(sizeof(BaseStructure) == 32);
static_assert(sizeof(ViewportHandle) == 40);
static_assert(sizeof(DLSSGOptions) == 40);
static_assert(sizeof(ReflexOptions) == 48);

} // namespace streamline
