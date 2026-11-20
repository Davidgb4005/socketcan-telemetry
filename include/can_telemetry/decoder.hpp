#pragma once

#include <optional>
#include <string>

#include "can_telemetry/frame.hpp"

namespace can_telemetry {

class TelemetryDecoder {
public:
    std::optional<std::string> DecodeJson(const CanFrame& frame) const;
};

}  // namespace can_telemetry
