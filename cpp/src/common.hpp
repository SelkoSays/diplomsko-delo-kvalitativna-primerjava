#pragma once

#include <cstdint>
#include <cstddef>
#include <string>
#include <string_view>
#include <memory>
#include <vector>
#include <functional>
#include <optional>
#include <variant>
#include <array>
#include <chrono>
#include <iostream>

// Type aliases matching C version
using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;
using i8 = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;
using usize = size_t;
using isize = ptrdiff_t;

// Time utilities
constexpr i64 US_PER_SEC = 1'000'000LL;

inline i64 time_us() {
    using namespace std::chrono;
    return duration_cast<microseconds>(
        steady_clock::now().time_since_epoch()
    ).count();
}
