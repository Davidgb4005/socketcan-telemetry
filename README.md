# SocketCAN Telemetry

A small C++17 Embedded Linux service that reads vehicle data from a SocketCAN
interface, decodes fixed-point signals, and emits newline-delimited JSON for
logging or forwarding to another process.

## What it demonstrates

- Linux raw CAN sockets and network-interface lookup
- Standard and extended CAN identifier handling
- Bounds-checked little-endian signal decoding
- A hardware-free replay mode for development and CI
- Cross-compilation-friendly CMake and systemd packaging
- Defensive parsing with clear failure messages

## Demo protocol

The repository uses two intentionally small example frames. They are not tied
to a proprietary vehicle DBC.

| CAN ID | Bytes | Signal |
| --- | --- | --- |
| `0x180` | 0–1 | Unsigned battery voltage in millivolts |
| `0x180` | 2–3 | Signed battery current in milliamps |
| `0x181` | 0–1 | Signed temperature in tenths of a degree Celsius |
| `0x181` | 2 | State of charge in percent |

## Build and test

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
```

Replay the included `candump`-style fixture without CAN hardware:

```sh
./build/socketcan-telemetry --replay tests/fixtures/vehicle.log
```

Example output:

```json
{"can_id":"0x180","type":"power","voltage_v":12.345,"current_a":-5.536}
{"can_id":"0x181","type":"battery_state","temperature_c":25.1,"state_of_charge_pct":87}
```

## Live SocketCAN

Create a virtual CAN interface for local testing:

```sh
sudo modprobe vcan
sudo ip link add dev vcan0 type vcan
sudo ip link set up vcan0
./build/socketcan-telemetry --interface vcan0
```

In another terminal, send a frame with `can-utils`:

```sh
cansend vcan0 180#393060EA00000000
```

For a physical adapter, bring up its bitrate and replace `vcan0` with `can0`.

## Deployment

`packaging/socketcan-telemetry.service` runs the decoder against `can0`, restarts
it on transient failures, and applies systemd hardening. The binary writes JSON
to standard output, so logs are available through `journalctl` and can be
forwarded by the platform's normal logging stack.

## Extending the decoder

Production protocols can replace the example switch in `TelemetryDecoder` or
generate equivalent decode functions from a DBC file. Parsing and transport are
kept separate so recorded traffic can exercise the same decoder used on target.

## License

MIT
