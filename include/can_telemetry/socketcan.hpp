#pragma once

#include <functional>
#include <string>

#include "can_telemetry/frame.hpp"

namespace can_telemetry {

using FrameHandler = std::function<void(const CanFrame&)>;

int RunSocketCan(const std::string& interface_name, const FrameHandler& handler);

}  // namespace can_telemetry
