#include "can_telemetry/decoder.hpp"

#include <cstdint>
#include <iomanip>
#include <sstream>

namespace can_telemetry {
namespace {

std::uint16_t ReadU16Le(const CanFrame& frame, std::size_t offset) {
    return static_cast<std::uint16_t>(frame.data[offset]) |
           static_cast<std::uint16_t>(frame.data[offset + 1U] << 8U);
}

std::int16_t ReadI16Le(const CanFrame& frame, std::size_t offset) {
    return static_cast<std::int16_t>(ReadU16Le(frame, offset));
}

}  // namespace

std::optional<std::string> TelemetryDecoder::DecodeJson(const CanFrame& frame) const {
    std::ostringstream output;
    output << std::fixed;

    if (frame.id == 0x180U && frame.size >= 4U) {
        const double voltage = static_cast<double>(ReadU16Le(frame, 0U)) / 1000.0;
        const double current = static_cast<double>(ReadI16Le(frame, 2U)) / 1000.0;
        output << std::setprecision(3)
               << "{\"can_id\":\"0x180\",\"type\":\"power\",\"voltage_v\":"
               << voltage << ",\"current_a\":" << current << '}';
        return output.str();
    }

    if (frame.id == 0x181U && frame.size >= 3U) {
        const double temperature = static_cast<double>(ReadI16Le(frame, 0U)) / 10.0;
        output << std::setprecision(1)
               << "{\"can_id\":\"0x181\",\"type\":\"battery_state\","
               << "\"temperature_c\":" << temperature
               << ",\"state_of_charge_pct\":" << static_cast<unsigned int>(frame.data[2U])
               << '}';
        return output.str();
    }

    return std::nullopt;
}

}  // namespace can_telemetry
