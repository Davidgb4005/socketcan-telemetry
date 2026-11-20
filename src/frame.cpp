#include "can_telemetry/frame.hpp"

#include <charconv>
#include <cctype>

namespace can_telemetry {
namespace {

std::string_view Trim(std::string_view value) {
    while (!value.empty() && std::isspace(static_cast<unsigned char>(value.front()))) {
        value.remove_prefix(1);
    }
    while (!value.empty() && std::isspace(static_cast<unsigned char>(value.back()))) {
        value.remove_suffix(1);
    }
    return value;
}

bool ParseHex(std::string_view value, std::uint32_t& result) {
    const auto parsed = std::from_chars(value.data(), value.data() + value.size(), result, 16);
    return parsed.ec == std::errc{} && parsed.ptr == value.data() + value.size();
}

}  // namespace

bool ParseCompactFrame(std::string_view line, CanFrame& frame, std::string& error) {
    line = Trim(line);
    const auto hash = line.find('#');
    if (hash == std::string_view::npos) {
        error = "missing # separator";
        return false;
    }

    auto id_start = line.rfind(' ', hash);
    id_start = id_start == std::string_view::npos ? 0U : id_start + 1U;
    const std::string_view id_text = Trim(line.substr(id_start, hash - id_start));
    const std::string_view payload = Trim(line.substr(hash + 1U));
    if (id_text.empty() || id_text.size() > 8U) {
        error = "invalid CAN identifier";
        return false;
    }
    if (payload.size() > 16U || payload.size() % 2U != 0U) {
        error = "payload must contain 0 to 8 complete bytes";
        return false;
    }

    std::uint32_t identifier = 0;
    if (!ParseHex(id_text, identifier) || identifier > 0x1FFFFFFFU) {
        error = "CAN identifier is not hexadecimal or exceeds 29 bits";
        return false;
    }

    CanFrame parsed_frame;
    parsed_frame.id = identifier;
    parsed_frame.size = static_cast<std::uint8_t>(payload.size() / 2U);
    for (std::size_t index = 0; index < parsed_frame.size; ++index) {
        std::uint32_t byte = 0;
        if (!ParseHex(payload.substr(index * 2U, 2U), byte)) {
            error = "payload contains a non-hexadecimal byte";
            return false;
        }
        parsed_frame.data[index] = static_cast<std::uint8_t>(byte);
    }

    frame = parsed_frame;
    error.clear();
    return true;
}

}  // namespace can_telemetry
