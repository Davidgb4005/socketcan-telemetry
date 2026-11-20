#include <cstdlib>
#include <iostream>
#include <string>

#include "can_telemetry/decoder.hpp"
#include "can_telemetry/frame.hpp"

#define CHECK(condition)                                                        \
    do {                                                                        \
        if (!(condition)) {                                                     \
            std::cerr << "check failed at " << __FILE__ << ':' << __LINE__     \
                      << ": " #condition << '\n';                              \
            return EXIT_FAILURE;                                                \
        }                                                                       \
    } while (false)

int main() {
    can_telemetry::CanFrame frame;
    std::string error;
    CHECK(can_telemetry::ParseCompactFrame("can0 180#393060EA00000000", frame, error));
    CHECK(frame.id == 0x180U);
    CHECK(frame.size == 8U);
    CHECK(frame.data[0] == 0x39U);

    const can_telemetry::TelemetryDecoder decoder;
    const auto power = decoder.DecodeJson(frame);
    CHECK(power.has_value());
    CHECK(power->find("\"voltage_v\":12.345") != std::string::npos);
    CHECK(power->find("\"current_a\":-5.536") != std::string::npos);

    CHECK(can_telemetry::ParseCompactFrame("(1.250000) can0 181#FB005700", frame, error));
    const auto battery = decoder.DecodeJson(frame);
    CHECK(battery.has_value());
    CHECK(battery->find("\"temperature_c\":25.1") != std::string::npos);
    CHECK(battery->find("\"state_of_charge_pct\":87") != std::string::npos);

    CHECK(can_telemetry::ParseCompactFrame("can0 777#00", frame, error));
    CHECK(!decoder.DecodeJson(frame).has_value());
    CHECK(!can_telemetry::ParseCompactFrame("can0 180#123", frame, error));
    CHECK(!can_telemetry::ParseCompactFrame("can0 ZZZ#00", frame, error));
    CHECK(!can_telemetry::ParseCompactFrame("can0 180#000102030405060708", frame, error));
    return EXIT_SUCCESS;
}
