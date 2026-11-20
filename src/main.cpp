#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>

#include "can_telemetry/decoder.hpp"
#include "can_telemetry/frame.hpp"
#include "can_telemetry/socketcan.hpp"

namespace {

void PrintUsage(const char* program) {
    std::cout << "Usage: " << program
              << " (--interface CAN_IFACE | --replay CANDUMP_FILE)\n";
}

int Replay(const std::string& path, const can_telemetry::TelemetryDecoder& decoder) {
    std::ifstream input(path);
    if (!input) {
        std::cerr << "Unable to open replay file: " << path << '\n';
        return EXIT_FAILURE;
    }

    std::string line;
    std::size_t line_number = 0U;
    while (std::getline(input, line)) {
        ++line_number;
        if (line.empty() || line.front() == '#') {
            continue;
        }

        can_telemetry::CanFrame frame;
        std::string error;
        if (!can_telemetry::ParseCompactFrame(line, frame, error)) {
            std::cerr << path << ':' << line_number << ": " << error << '\n';
            return EXIT_FAILURE;
        }
        if (const auto json = decoder.DecodeJson(frame)) {
            std::cout << *json << '\n';
        }
    }
    return EXIT_SUCCESS;
}

}  // namespace

int main(int argc, char** argv) {
    if (argc != 3) {
        PrintUsage(argv[0]);
        return EXIT_FAILURE;
    }

    const std::string mode = argv[1];
    const std::string value = argv[2];
    const can_telemetry::TelemetryDecoder decoder;

    if (mode == "--replay") {
        return Replay(value, decoder);
    }
    if (mode == "--interface") {
        return can_telemetry::RunSocketCan(value, [&decoder](const auto& frame) {
            if (const auto json = decoder.DecodeJson(frame)) {
                std::cout << *json << std::endl;
            }
        }) == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
    }

    PrintUsage(argv[0]);
    return EXIT_FAILURE;
}
