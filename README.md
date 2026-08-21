# SensESP BLE Gateway

> [!IMPORTANT]
> **This project is archived and no longer maintained.**
>
> It is superseded by **[espos-ble-gateway](https://github.com/dirkwa/espos-ble-gateway)**,
> a rewrite on [espOS](https://github.com/dirkwa/espOS) (pure ESP-IDF 6) that
> speaks the same signalk-server BLE provider protocol, so no server-side
> change is needed.
>
> What the successor adds:
>
> * **GATT write-without-response** (`with_response`), which JK-BMS, Daly-BMS
>   and similar peripherals require — the field was always in the server
>   protocol, and this firmware silently ignored it.
> * WiFi provisioning, a web config UI, signed OTA with rollback and a log
>   ring, all from espOS rather than hand-rolled.
> * Host tests for the wire format, and a CI matrix over five targets.
> * Fixes for several bugs that live on in this code: GATT operations ignoring
>   the connection handle (two devices sharing a vendor UUID crossed wires), a
>   double `register_for_notify` with a NULL BDA, and under-reported
>   advertisement drops.
>
> The one thing it does **not** carry over is the **NimBLE / ESP32-C5** path.
> That backend is scan-only here too (it never had a GATT client), so if you
> need C5 support this repository is the only place it exists.

BLE gateway library for [SensESP](https://github.com/SignalK/SensESP) that bridges Bluetooth Low Energy devices to [signalk-server](https://github.com/SignalK/signalk-server)'s BLE provider API.

## Features

- **BLE scanning** with advertisements forwarded to signalk-server via HTTP POST
- **GATT client** support (connect, discover, subscribe, read, write) on Bluedroid targets
- **Control WebSocket** for gateway metadata (hello, status) and GATT commands
- **4-level scan watchdog** for esp_hosted targets (restart, RPC reset, GPIO hard-reset, reboot)
- **Multiple BT stack support**:
  - `EspHostedBluedroidBLE` — ESP32-P4 + C6 companion via esp_hosted SDIO
  - `NativeBLE` — native Bluedroid on ESP32, ESP32-C3, ESP32-S3, etc.
  - `NimBLEProvisioner` — NimBLE for memory-constrained chips (ESP32-C5 with WiFi)

## Hardware Tested

| Board | BT Stack | Network | Status |
|-------|----------|---------|--------|
| Waveshare ESP32-P4-WIFI6-POE-ETH | Bluedroid + esp_hosted | Ethernet | Full (WS + GATT) |
| Waveshare ESP32-C5-WIFI6-KIT | NimBLE | WiFi | POST-only (WS disabled for RAM) |

## Quick Start

Add to your `platformio.ini`:

```ini
lib_deps =
    SignalK/SensESP@>=3.0.0
    https://github.com/dirkwa/sensesp-ble-gateway.git
```

### P4 Example (Bluedroid + Ethernet)

```cpp
#include "sensesp_ble_gateway/ble_signalk_gateway.h"
#include "sensesp_ble_gateway/esp_hosted_bluedroid_ble.h"

auto ble = std::make_shared<EspHostedBluedroidBLE>();
auto gateway = std::make_shared<BLESignalKGateway>(ble, app->get_ws_client());
gateway->start();
```

### C5 Example (NimBLE + WiFi)

```cpp
#include "sensesp_ble_gateway/ble_signalk_gateway.h"
#include "sensesp_ble_gateway/nimble_ble.h"

auto ble = std::make_shared<NimBLEProvisioner>();
BLESignalKGatewayConfig gw_cfg;
gw_cfg.enable_control_ws = false;  // Save RAM
auto gateway = std::make_shared<BLESignalKGateway>(ble, app->get_ws_client(), gw_cfg);
gateway->start();
```

See the [examples/](examples/) directory for complete working firmware.

## Requirements

- SensESP >= 3.3.0 (needs `SKWSClient::get_auth_token()`, hostname persistence fix, and stale polling href fix — all merged into main, pending next release; until then use `https://github.com/SignalK/SensESP.git#main` in `lib_deps`)
- signalk-server with BLE provider API (branch `ble-provider-api`)
- PlatformIO with `framework = espidf, arduino` (pioarduino)

## Note on the ESP32-P4 C6 Antenna

The Waveshare ESP32-P4-WIFI6-POE-ETH uses the ESP32-C6-MINI-**1U** module which has **no built-in PCB antenna**. You must connect an external 2.4 GHz antenna to the IPEX connector for BLE to work.

## License

sensesp-ble-gateway 1.0.0 and later is **source available, not open source**.
See [LICENSE.md](LICENSE.md).

**You may**, free of charge: run it on your own boat or fleet, private or
commercial; use it for internal company operations; modify it for your own use;
use it in education and research; and provide professional services around it.

**You may not**: redistribute it, or publish a modified version of it to the
PlatformIO registry, the Arduino library index or anywhere else. Verbatim
copies of official releases may be mirrored and cached.

Versions 0.1.0 and earlier remain available under the Apache-2.0 license, see
[LICENSE-Apache-2.0-through-v0.x.txt](LICENSE-Apache-2.0-through-v0.x.txt).

## Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md).
