#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <string_view>

namespace can_telemetry {

struct CanFrame {
    std::uint32_t id = 0;
    std::array<std::uint8_t, 8> data{};
    std::uint8_t size = 0;
};

bool ParseCompactFrame(std::string_view line, CanFrame& frame, std::string& error);

}  // namespace can_telemetry
