/**
 * @file main.cpp
 * @brief SensESP BLE gateway on ESP32-C5 (NimBLE + WiFi).
 *
 * Uses NimBLE instead of Bluedroid to fit WiFi + BLE + HTTP/WS
 * within the C5's limited internal SRAM. Scan duty cycle and
 * ad buffer are tuned to keep heap above ~10KB.
 */

#include "sensesp_ble_gateway/ble_signalk_gateway.h"
#include "sensesp_ble_gateway/nimble_ble.h"
#include "sensesp_app_builder.h"

using namespace sensesp;

static std::shared_ptr<NimBLEProvisioner> g_ble;
static std::shared_ptr<BLESignalKGateway> g_gateway;

void setup() {
  SetupLogging(ESP_LOG_INFO);

  // Credentials come from build flags, never from the source tree. Set them
  // in a local, git-ignored file rather than here:
  //
  //   ; platformio_local.ini  (add `extra_configs = platformio_local.ini`)
  //   build_flags = -D WIFI_SSID='"your-ssid"' -D WIFI_PASSWORD='"your-pass"'
  //
  // Leaving them unset drops the call entirely, so SensESP falls back to its
  // own WiFi provisioning portal instead of shipping a hardcoded network.
  SensESPAppBuilder builder;
  builder.set_hostname(GATEWAY_HOSTNAME)->enable_ota("c5-ble-gw-ota");
#if defined(WIFI_SSID) && defined(WIFI_PASSWORD)
  builder.set_wifi_client(WIFI_SSID, WIFI_PASSWORD);
#endif
  auto app = builder.get_app();

  // Lower scan duty cycle to reduce memory pressure on the C5
  // (WiFi + BLE + HTTP/WS is tight on internal RAM).
  NimBLEProvisionerConfig ble_cfg;
  ble_cfg.scan_interval_ms = 320;
  ble_cfg.scan_window_ms = 30;  // ~9% duty
  g_ble = std::make_shared<NimBLEProvisioner>(ble_cfg);

  // Smaller ad buffer and faster POST interval to keep heap stable.
  BLESignalKGatewayConfig gw_cfg;
  gw_cfg.max_pending_ads = 50;
  gw_cfg.post_interval_ms = 3000;
  gw_cfg.enable_control_ws = false;  // Save RAM on the C5
  g_gateway =
      std::make_shared<BLESignalKGateway>(g_ble, app->get_ws_client(), gw_cfg);
  g_gateway->start();

  event_loop()->onRepeat(5000, []() {
    ESP_LOGI(
        "GW",
        "alive — uptime=%lus heap=%u ble_hits=%u ble_scan=%d gw_rx=%u "
        "gw_posted=%u gw_dropped=%u post_ok=%u post_fail=%u ws_up=%d",
        (unsigned long)(millis() / 1000), (unsigned)ESP.getFreeHeap(),
        (unsigned)(g_ble ? g_ble->scan_hit_count() : 0),
        (int)(g_ble ? g_ble->is_scanning() : false),
        (unsigned)(g_gateway ? g_gateway->advertisements_received() : 0),
        (unsigned)(g_gateway ? g_gateway->advertisements_posted() : 0),
        (unsigned)(g_gateway ? g_gateway->advertisements_dropped() : 0),
        (unsigned)(g_gateway ? g_gateway->http_post_success() : 0),
        (unsigned)(g_gateway ? g_gateway->http_post_fail() : 0),
        (int)(g_gateway ? g_gateway->control_ws_connected() : false));
  });
}

void loop() { event_loop()->tick(); }
