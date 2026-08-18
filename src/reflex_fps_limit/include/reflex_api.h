#pragma once

#include <stddef.h>
#include <stdint.h>

namespace streamline {

constexpr uint32_t FEATURE_REFLEX = 3;
constexpr size_t STRUCT_VERSION_1 = 1;

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

enum ReflexMode : int32_t {
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

    ReflexMode mode{off};
    uint32_t frame_limit_us{};
    bool use_markers_to_optimize{};
    uint16_t virtual_key{};
    uint32_t thread_id{};
};

using GetFeatureFunction =
    Result (*)(uint32_t feature, const char* function_name, void*& function);
using ReflexSetOptions = Result (*)(const ReflexOptions& options);

static_assert(sizeof(StructType) == 16);
static_assert(sizeof(BaseStructure) == 32);
static_assert(sizeof(ReflexOptions) == 48);

} // namespace streamline
