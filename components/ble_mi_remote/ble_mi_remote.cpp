
#ifdef USE_ESP32

#include "ble_mi_remote.h"
#include <NimBLEServer.h>
#include <NimBLEDevice.h>
#include <NimBLEService.h>
#include <NimBLEUtils.h>
#include <NimBLEHIDDevice.h>
#include <NimBLECharacteristic.h>
#include <NimBLEAdvertising.h>
#include <NimBLEClient.h>
#include <NimBLEAddress.h>
#include "HIDTypes.h"
#include "HIDKeyboardTypes.h"
// #include <driver/adc.h>
#include "sdkconfig.h"
#include <string>
#include <list>
#include "esphome/core/hal.h"
#include "esphome/core/log.h"
#include "esphome/core/helpers.h"
#include "esphome/core/preferences.h"

#define CONSUMER_ID 0x01
#define KEYBOARD_ID 0x02

static const uint8_t _hidReportDescriptor[] = {
    USAGE_PAGE(1),      0x0C,     // Consumer
    USAGE(1),       0x01,     // Consumer Control

    COLLECTION(1),      0x01,     // Application

    REPORT_ID(1),     CONSUMER_ID,  //
    LOGICAL_MINIMUM(1),   0x00,     //
    LOGICAL_MAXIMUM(1),   0x01,     //
    REPORT_SIZE(1),     0x01,     //
    REPORT_COUNT(1),    0x18,     // 24
    USAGE(1),       0x41,     // Menu Pick
    USAGE(1),       0x42,     // Menu Up
    USAGE(1),       0x43,     // Menu Down
    USAGE(1),       0x44,     // Menu Left
    USAGE(1),       0x45,     // Menu Right
    USAGE(1),       0x30,     // Power
    USAGE(1),       0xCF,     // Voice Command
    USAGE(1),       0xE9,     // Volume Increment
    USAGE(1),       0xEA,     // Volume Decrement
    USAGE(2),       0xA2, 0x01,   // Instance 41473 (Select Task/Application)
    USAGE(2),       0xB8, 0x01,   // Instance 47105 (Movie Browser)
    USAGE(1),       0x6A,     // Green Menu Button
    USAGE(1),       0x6B,     // Blue Menu Button
    USAGE(1),       0x6C,     // Yellow Menu Button
    USAGE(2),       0x8A, 0x01,   // Instance 35329 (Email reader??)
    USAGE(2),       0x92, 0x01,   // Instance 37377 (Calculator)
    USAGE(2),       0x94, 0x01,   // Instance 37889 (Local Machine Browser)
    USAGE(1),       0x40,        // Menu
    USAGE(2),       0x23, 0x02,   // Instance 8962 (Home)
    USAGE(2),       0x24, 0x02,   // Instance 9218 (Back)
    USAGE(2),       0x25, 0x02,   // Instance 9474 (Forward)
    USAGE(2),       0x26, 0x02,   // Instance 9730 (Stop)
    USAGE(2),       0x27, 0x02,   // Instance 9986 (Refresh)
    USAGE(2),       0x2A, 0x02,   // Instance 10754 (Bookmarks)
    HIDINPUT(1),      0x02,     // In bytes

    USAGE_PAGE(2),      0x00, 0xFF,   // Vendor ??? переставить местами байты?
    USAGE(1),       0x00,     // Vendor

    COLLECTION(1),      0x01,     // Application

    REPORT_ID(1),     0x06,     //
    REPORT_SIZE(1),     0x08,     //
    REPORT_COUNT(1),    0x78,     // 120
    LOGICAL_MINIMUM(1),   0x00,     //
    LOGICAL_MAXIMUM(1),   0xFF,     // -1
    USAGE_MINIMUM(1),   0x00,     //
    USAGE_MAXIMUM(1),   0xFF,     //
    HIDINPUT(1),      0x00,     //

    REPORT_ID(1),     0x07,     //
    REPORT_SIZE(1),     0x08,     //
    REPORT_COUNT(1),    0x78,     // 120
    LOGICAL_MINIMUM(1),   0x00,     //
    LOGICAL_MAXIMUM(1),   0xFF,     // -1
    USAGE_MINIMUM(1),   0x00,     //
    USAGE_MAXIMUM(1),   0xFF,     //
    HIDINPUT(1),      0x00,     //

    REPORT_ID(1),     0x08,     //
    REPORT_SIZE(1),     0x08,     //
    REPORT_COUNT(1),    0x78,     // 120
    LOGICAL_MINIMUM(1),   0x00,     //
    LOGICAL_MAXIMUM(1),   0xFF,     // 255
    USAGE_MINIMUM(1),   0x00,     //
    USAGE_MAXIMUM(1),   0xFF,     //
    HIDINPUT(1),      0x00,     //

    END_COLLECTION(0),

    USAGE_PAGE(1),      0x01,     // Generic Desktop Controls
    USAGE(1),       0x06,     // Keyboard

    COLLECTION(1),      0x01,     // Application

    REPORT_ID(1),     KEYBOARD_ID,  //
    USAGE_PAGE(1),      0x07,     // Keyboard/Keypad
    USAGE_MINIMUM(1),   0xE0,     // Ctrl, Shift, Alt, GUI
    USAGE_MAXIMUM(1),   0xE7,     // Ctrl, Shift, Alt, GUI
    LOGICAL_MINIMUM(1),   0x00,     //
    LOGICAL_MAXIMUM(1),   0x01,     //
    REPORT_SIZE(1),     0x01,     //
    REPORT_COUNT(1),    0x08,     //
    HIDINPUT(1),      0x02,     //
    REPORT_COUNT(1),    0x01,     //
    REPORT_SIZE(1),     0x08,     //
    HIDINPUT(1),      0x01,     //

    REPORT_COUNT(1),    0x05,     //
    REPORT_SIZE(1),     0x01,     //
    USAGE_PAGE(1),      0x08,     // LED
    USAGE_MINIMUM(1),   0x01,     //
    USAGE_MAXIMUM(1),   0x05,     //
    HIDOUTPUT(1),     0x02,     //
    REPORT_COUNT(1),    0x01,     //
    REPORT_SIZE(1),     0x03,     //
    HIDOUTPUT(1),     0x01,     //

    REPORT_COUNT(1),    0x06,     //
    REPORT_SIZE(1),     0x08,     //
    LOGICAL_MINIMUM(1),   0x28,     // 40
    LOGICAL_MAXIMUM(1),   0xFE,     // 254
    USAGE_PAGE(1),      0x07,     // Keyboard/Keypad
    USAGE_MINIMUM(1),   0x28,     // 40
    USAGE_MAXIMUM(1),   0xFE,     // 254
    HIDINPUT(1),      0x00,     //

    END_COLLECTION(0),
    END_COLLECTION(0)
};

namespace esphome {
  namespace ble_mi_remote {
    static const char *const TAG = "ble_mi_remote";

    BleMiRemote::BleMiRemote(std::string name, std::string manufacturer_id, uint8_t battery_level, bool reconnect)
      :
        PollingComponent(1000),
        hid(0),
        deviceName(std::string(name).substr(0, 15)),
          deviceManufacturer(std::string(manufacturer_id).substr(0,15)),
          batteryLevel(battery_level)
    {
      _reconnect = reconnect;
    }

    void BleMiRemote::setup() {
      ESP_LOGI(TAG, "Setting this up...");

      this->load_target_mac_();

      NimBLEDevice::init(deviceName);
      this->pServer = NimBLEDevice::createServer();

      pServer->setCallbacks(this);
      // Always false: NimBLE's own auto-restart only knows plain undirected
      // advertising and would race right after onDisconnect() below, undoing
      // the directed reconnect burst start_reconnect_advert_() just started.
      // onDisconnect() is now the sole place deciding whether/how to
      // re-advertise (gated on _reconnect there instead).
      pServer->advertiseOnDisconnect(false);

      hid = new NimBLEHIDDevice(pServer);
      inputSpecialKeys = hid->getInputReport(CONSUMER_ID);
      inputKeyboard = hid->getInputReport(KEYBOARD_ID);
      outputKeyboard = hid->getOutputReport(KEYBOARD_ID);
      outputKeyboard->setCallbacks(this);

      vendorReport_06 = hid->getInputReport(0x06);
      vendorReport_07 = hid->getInputReport(0x07);
      vendorReport_08 = hid->getInputReport(0x08);

      hid->setManufacturer(deviceManufacturer);
      hid->setPnp(sid, vid, pid, version);
      hid->setHidInfo(0x00, 0x00);

      NimBLEDevice::setSecurityAuth(true, true, true);

      hid->setReportMap((uint8_t*) _hidReportDescriptor, sizeof(_hidReportDescriptor));
      bool serverStartOk = pServer->start();
      ESP_LOGI(TAG, "setup: pServer->start()=%s", serverStartOk ? "OK" : "FAILED");

      onStarted(pServer);

      advertising = pServer->getAdvertising();
      advertising->setAppearance(HID_KEYBOARD);
      advertising->addServiceUUID(hid->getHidService()->getUUID());
      // Scan response carries the device name; without it, some centrals
      // (confirmed: this TV box) won't show the device at all in a fresh
      // "add device" scan - only bonded/directed reconnects worked.
      advertising->enableScanResponse(true);

      this->start_reconnect_advert_();

      hid->setBatteryLevel(batteryLevel);

      release();
    }

    void BleMiRemote::stop() {
      ESP_LOGI(TAG, "stop: entered, reconnect=%s", this->_reconnect ? "true" : "false");

      this->_should_readvertise = false;

      std::vector<uint16_t> ids = pServer->getPeerDevices();

      if (ids.size() > 0) {
        ESP_LOGI(TAG, "stop: disconnecting %d peer(s)", (int) ids.size());
        for (uint16_t &id : ids) {
          pServer->disconnect(id);
        }
      } else {
        bool ok = pServer->stopAdvertising();
        ESP_LOGI(TAG, "stop: stopAdvertising()=%s", ok ? "OK" : "FAILED");
      }
    }

    void BleMiRemote::start() {
      ESP_LOGI(TAG, "start: entered, reconnect=%s", this->_reconnect ? "true" : "false");

      this->_should_readvertise = true;
      this->start_reconnect_advert_();
    }

    void BleMiRemote::update() { state_sensor_->publish_state(this->_connected); }

    bool BleMiRemote::is_connected() {
      if (!this->_connected) {
        ESP_LOGI(TAG, "Disconnected");

        return false;
      }

      return true;
    }

    void BleMiRemote::set_battery_level(uint8_t level) {
      this->batteryLevel = level;
      if (hid != 0)
        this->hid->setBatteryLevel(this->batteryLevel);
    }

    void BleMiRemote::update_timer() {
      this->cancel_timeout(TAG);
      this->set_timeout(TAG, _release_delay, [this]() { this->release(); });
    }


    void BleMiRemote::sendReport(KeyReport *keys) {
      ESP_LOGD(TAG, "sendReport FIRING...");
      if (this->is_connected()) {
        ESP_LOGD(TAG, "sendReport FIRED!!!");

        this->inputKeyboard->setValue((uint8_t*) keys, sizeof(KeyReport));
        if (!this->inputKeyboard->notify()) {
          ESP_LOGE(TAG, "sendReport FAILED!!!");
        }
        this->delay_ms(_delay_ms);
      }
    }

    void BleMiRemote::sendReport(SpecialKeyReport *keys) {
      ESP_LOGD(TAG, "sendReport FIRING...");
      if (this->is_connected()) {
        ESP_LOGD(TAG, "sendReport FIRED!!!");

        this->inputSpecialKeys->setValue((uint8_t*) keys, sizeof(SpecialKeyReport));
        if (!this->inputSpecialKeys->notify()) {
          ESP_LOGE(TAG, "sendReport FAILED!!!");
        }
        this->delay_ms(_delay_ms);
      }
    }

    extern
    const uint8_t _asciimap[128] PROGMEM;

    #define SHIFT 0x80
    const uint8_t _asciimap[128] = {
        0x00,             // NUL
        0x00,             // SOH
        0x00,             // STX
        0x00,             // ETX
        0x00,             // EOT
        0x00,             // ENQ
        0x00,             // ACK
        0x00,             // BEL
        0x2a,     // BS Backspace
        0x2b,     // TAB  Tab
        0x28,     // LF Enter
        0x00,             // VT
        0x00,             // FF
        0x00,             // CR
        0x00,             // SO
        0x00,             // SI
        0x00,             // DEL
        0x00,             // DC1
        0x00,             // DC2
        0x00,             // DC3
        0x00,             // DC4
        0x00,             // NAK
        0x00,             // SYN
        0x00,             // ETB
        0x00,             // CAN
        0x00,             // EM
        0x00,             // SUB
        0x00,             // ESC
        0x00,             // FS
        0x00,             // GS
        0x00,             // RS
        0x00,             // US

        0x2c,      //  ' '
        0x1e | SHIFT,    // !
        0x34 | SHIFT,    // "
        0x20 | SHIFT,    // #
        0x21 | SHIFT,    // $
        0x22 | SHIFT,    // %
        0x24 | SHIFT,    // &
        0x34,          // '
        0x26 | SHIFT,    // (
        0x27 | SHIFT,    // )
        0x25 | SHIFT,    // *
        0x2e | SHIFT,    // +
        0x36,          // ,
        0x2d,          // -
        0x37,          // .
        0x38,          // /
        0x27,          // 0
        0x1e,          // 1
        0x1f,          // 2
        0x20,          // 3
        0x21,          // 4
        0x22,          // 5
        0x23,          // 6
        0x24,          // 7
        0x25,          // 8
        0x26,          // 9
        0x33 | SHIFT,      // :
        0x33,          // ;
        0x36 | SHIFT,      // <
        0x2e,          // =
        0x37 | SHIFT,      // >
        0x38 | SHIFT,      // ?
        0x1f | SHIFT,      // @
        0x04 | SHIFT,      // A
        0x05 | SHIFT,      // B
        0x06 | SHIFT,      // C
        0x07 | SHIFT,      // D
        0x08 | SHIFT,      // E
        0x09 | SHIFT,      // F
        0x0a | SHIFT,      // G
        0x0b | SHIFT,      // H
        0x0c | SHIFT,      // I
        0x0d | SHIFT,      // J
        0x0e | SHIFT,      // K
        0x0f | SHIFT,      // L
        0x10 | SHIFT,      // M
        0x11 | SHIFT,      // N
        0x12 | SHIFT,      // O
        0x13 | SHIFT,      // P
        0x14 | SHIFT,      // Q
        0x15 | SHIFT,      // R
        0x16 | SHIFT,      // S
        0x17 | SHIFT,      // T
        0x18 | SHIFT,      // U
        0x19 | SHIFT,      // V
        0x1a | SHIFT,      // W
        0x1b | SHIFT,      // X
        0x1c | SHIFT,      // Y
        0x1d | SHIFT,      // Z
        0x2f,          // [
        0x31,          // bslash
        0x30,          // ]
        0x23 | SHIFT,    // ^
        0x2d | SHIFT,    // _
        0x35,          // `
        0x04,          // a
        0x05,          // b
        0x06,          // c
        0x07,          // d
        0x08,          // e
        0x09,          // f
        0x0a,          // g
        0x0b,          // h
        0x0c,          // i
        0x0d,          // j
        0x0e,          // k
        0x0f,          // l
        0x10,          // m
        0x11,          // n
        0x12,          // o
        0x13,          // p
        0x14,          // q
        0x15,          // r
        0x16,          // s
        0x17,          // t
        0x18,          // u
        0x19,          // v
        0x1a,          // w
        0x1b,          // x
        0x1c,          // y
        0x1d,          // z
        0x2f | SHIFT,    // {
        0x31 | SHIFT,    // |
        0x30 | SHIFT,    // }
        0x35 | SHIFT,    // ~
        0       // DEL
    };

    uint8_t USBPutChar(uint8_t c);

    void BleMiRemote::press(uint8_t k, bool with_timer) {
      ESP_LOGI(TAG, "press: k=%d connected=%s", k, this->_connected ? "true" : "false");
      if (this->is_connected()) {
        if (with_timer) {
          this->update_timer();
        }

        uint8_t i;
        if (k >= 136) {     // it's a non-printing key (not a modifier)
          k = k - 136;
        } else if (k >= 128) {  // it's a modifier key
          _keyReport.modifiers |= (1 << (k - 128));
          k = 0;
        } else {        // it's a printing key
          k = progmem_read_byte(_asciimap + k);
          if (!k) {

            return;
          }
          if (k & 0x80) {           // it's a capital letter or other character reached with shift
            _keyReport.modifiers |= 0x02; // the left shift modifier
            k &= 0x7F;
          }
        }

        // Add k to the key report only if it's not already present
        // and if there is an empty slot.
        if (_keyReport.keys[0] != k && _keyReport.keys[1] != k && _keyReport.keys[2] != k && _keyReport.keys[3] != k && _keyReport.keys[4] != k && _keyReport.keys[5] != k) {

          for (i = 0; i < 6; i++) {
            if (_keyReport.keys[i] == 0x00) {
              _keyReport.keys[i] = k;
              break;
            }
          }
          if (i == 6) {
            return;
          }
        }
        sendReport (&_keyReport);
      }
    }

    void BleMiRemote::powerAdvertStart() {
      ESP_LOGI(TAG, "powerAdvertStart: entered");
      _power_advert_cycle = 0;
      this->powerAdvertData1();
    }

    // NimBLEAdvertising::setManufacturerData() only marks the advertisement
    // data dirty (m_advDataSet = false); it's only actually pushed to the
    // controller the next time start() runs past its "already active" guard.
    // Since advertising never legitimately stops while this is called (it's
    // always running from setup()), a bare setManufacturerData() never
    // reached the air at all - confirmed by an over-the-air capture showing
    // no manufacturer data ever transmitted. stop()+start() forces the
    // guard to actually re-apply the new data.
    void BleMiRemote::powerAdvertData1() {
      NimBLEAdvertising *adv = pServer->getAdvertising();
      bool setOk = adv->setManufacturerData(std::vector<uint8_t>{0x46, 0x00, 0xe7, 0x12, 0x97, 0x30, 0x35, 0xf2, 0x78, 0xff, 0xff, 0xff, 0x30, 0x43, 0x52, 0x4b, 0x54, 0x4d});
      bool stopOk = adv->stop();
      bool startOk = adv->start();
      ESP_LOGI(TAG, "powerAdvertData1: setManufacturerData=%s stop=%s start=%s", setOk ? "OK" : "FAILED", stopOk ? "OK" : "FAILED", startOk ? "OK" : "FAILED");
      this->set_timeout("ble_mi_remote_power_advert", _power_advert_delay, [this]() { this->powerAdvertData2(); });
    }

    void BleMiRemote::powerAdvertData2() {
      NimBLEAdvertising *adv = pServer->getAdvertising();
      bool setOk = adv->setManufacturerData(std::vector<uint8_t>{0x46, 0x00});
      bool stopOk = adv->stop();
      bool startOk = adv->start();
      ESP_LOGI(TAG, "powerAdvertData2: setManufacturerData=%s stop=%s start=%s cycle=%d", setOk ? "OK" : "FAILED", stopOk ? "OK" : "FAILED", startOk ? "OK" : "FAILED", _power_advert_cycle);
      if (_power_advert_cycle > 3) {
        this->powerAdvertStop();
      } else {
        _power_advert_cycle++;
        this->set_timeout("ble_mi_remote_power_advert", _power_advert_delay, [this]() { this->powerAdvertData1(); });
      }
    }

    void BleMiRemote::powerAdvertStop() {
      NimBLEAdvertising *adv = pServer->getAdvertising();
      bool setOk = adv->setManufacturerData(std::vector<uint8_t>{});
      bool stopOk = adv->stop();
      bool startOk = adv->start();
      ESP_LOGI(TAG, "powerAdvertStop: setManufacturerData=%s stop=%s start=%s", setOk ? "OK" : "FAILED", stopOk ? "OK" : "FAILED", startOk ? "OK" : "FAILED");
    }

    // Mirrors the wake trick used by github.com/DenizOner/MiPower: instead of
    // this device advertising, act as a BLE central and directly attempt a
    // connection to the (already-bonded) target's MAC address, the same
    // effect as `bluetoothctl pair <mac>`. One-shot, blocking, short timeout;
    // not looped like powerAdvertStart() pending hardware testing.
    void BleMiRemote::connectWakeStart() {
      if (!_has_target_mac) {
        ESP_LOGW(TAG, "connectWakeStart: no target_mac_address configured, skipping");
        return;
      }

      NimBLEAddress targetAddress(_target_mac, BLE_ADDR_PUBLIC);
      NimBLEClient* pClient = NimBLEDevice::createClient(targetAddress);
      pClient->setSelfDelete(true, true);
      pClient->setConnectTimeout(3000);

      ESP_LOGD(TAG, "connectWakeStart: connecting to %s", targetAddress.toString().c_str());

      if (!pClient->connect(targetAddress, false, false, false)) {
        ESP_LOGW(TAG, "connectWakeStart: connect attempt failed or timed out");
        return;
      }

      ESP_LOGD(TAG, "connectWakeStart: connected, disconnecting");
      pClient->disconnect();
    }

    void BleMiRemote::pressSpecial(uint8_t k, bool with_timer) {
      ESP_LOGI(TAG, "pressSpecial: k=%d connected=%s", k, this->_connected ? "true" : "false");
      if (this->is_connected()) {
        if (with_timer) {
          this->update_timer();
        }
          uint8_t bit = k % 8;
          uint8_t byte = int(k / 8);

          _specialKeyReport.keys[byte] |= (1 << bit);

          ESP_LOGD(TAG, "Send: %d, %d, %d", _specialKeyReport.keys[0], _specialKeyReport.keys[1], _specialKeyReport.keys[2]);

          sendReport (&_specialKeyReport);
      } else if (k == SPECIAL_POWER) {
        this->powerAdvertStart();
      }
    }

    void BleMiRemote::release() {
      if (this->is_connected()) {
        this->cancel_timeout(TAG);

        _keyReport.keys[0] = 0;
        _keyReport.keys[1] = 0;
        _keyReport.keys[2] = 0;
        _keyReport.keys[3] = 0;
        _keyReport.keys[4] = 0;
        _keyReport.keys[5] = 0;
        _keyReport.modifiers = 0;
        _specialKeyReport.keys[0] = 0;
        _specialKeyReport.keys[1] = 0;
        _specialKeyReport.keys[2] = 0;
        sendReport (&_keyReport);
        sendReport (&_specialKeyReport);
      }
    }

    // Auto-learns the target_mac_address instead of requiring it hand-typed
    // in YAML: every time a peer actually connects to us, remember its
    // address (persisted to NVS flash so it survives reboots). An explicit
    // target_mac_address: in YAML always takes priority and is never
    // overwritten by this.
    void BleMiRemote::load_target_mac_() {
      this->_target_mac_pref = global_preferences->make_preference<uint64_t>(fnv1_hash("ble_mi_remote_target_mac"));

      if (this->_target_mac_from_config) {
        ESP_LOGI(TAG, "load_target_mac_: using target_mac_address from config: %s", NimBLEAddress(this->_target_mac, BLE_ADDR_PUBLIC).toString().c_str());
        return;
      }

      uint64_t stored = 0;
      if (this->_target_mac_pref.load(&stored) && stored != 0) {
        this->_target_mac = stored;
        this->_has_target_mac = true;
        ESP_LOGI(TAG, "load_target_mac_: loaded learned target %s from flash", NimBLEAddress(stored, BLE_ADDR_PUBLIC).toString().c_str());
      } else {
        ESP_LOGI(TAG, "load_target_mac_: no target_mac_address configured and nothing learned yet");
      }
    }

    void BleMiRemote::learn_target_mac_(NimBLEAddress addr) {
      if (this->_target_mac_from_config) {
        return;
      }

      uint64_t mac = (uint64_t) addr;
      if (mac == 0 || mac == this->_target_mac) {
        return;
      }

      this->_target_mac = mac;
      this->_has_target_mac = true;

      bool ok = this->_target_mac_pref.save(&mac);
      ESP_LOGI(TAG, "learn_target_mac_: learned peer %s, saved to flash=%s", addr.toString().c_str(), ok ? "OK" : "FAILED");
    }

    // Mirrors what the real Xiaomi remote actually does on power-up, confirmed
    // by a live nRF52840 sniffer capture: a burst of ADV_DIRECT_IND targeted
    // at the bonded central's address (MAC learned via learn_target_mac_()),
    // ~3.75ms/channel, instead of our previous plain undirected advertising.
    // NimBLE/the controller pick high-duty-cycle directed advertising
    // automatically when duration<=1280ms and a dirAddr is given - passing
    // duration=0 here would instead map to BLE_HS_FOREVER and *not* get the
    // fast cadence, so the explicit 1280 (the BLE spec's own HD directed-adv
    // cap) matters. Falls back to normal undirected advertising once the
    // burst window elapses without a connection.
    void BleMiRemote::start_reconnect_advert_() {
      NimBLEAdvertising *adv = pServer->getAdvertising();

      if (!this->_has_target_mac) {
        bool startOk = adv->start();
        ESP_LOGI(TAG, "start_reconnect_advert_: no target_mac_address learned yet, plain start()=%s", startOk ? "OK" : "FAILED");
        return;
      }

      NimBLEAddress dirAddr(this->_target_mac, BLE_ADDR_PUBLIC);
      bool stopOk = adv->stop();
      bool startOk = adv->start(1280, &dirAddr);
      ESP_LOGI(TAG, "start_reconnect_advert_: directed burst to %s, stop=%s start=%s", dirAddr.toString().c_str(), stopOk ? "OK" : "FAILED", startOk ? "OK" : "FAILED");

      this->set_timeout("ble_mi_remote_reconnect_burst", 1300, [this]() {
        NimBLEAdvertising *adv2 = pServer->getAdvertising();
        bool ok2 = adv2->start();
        ESP_LOGI(TAG, "start_reconnect_advert_: burst window elapsed, fallback start()=%s", ok2 ? "OK" : "FAILED");
      });
    }

    void BleMiRemote::onConnect(NimBLEServer *pServer, NimBLEConnInfo& connInfo) {
      this->_connected = true;
      NimBLEConnInfo peer = connInfo;

      ESP_LOGI(TAG, "Connected: %s", peer.getAddress().toString().c_str());

      this->learn_target_mac_(peer.getAddress());

      // Explicit supervision timeout so a peer that silently vanishes at the
      // app layer (radio still ACKing link-layer traffic, host-side BT
      // service gone) gets detected and torn down within a bounded time,
      // instead of leaving us believing we're connected - and therefore not
      // advertising - indefinitely. min/max interval 15/30ms, no peripheral
      // latency, 4s supervision timeout (400 * 10ms).
      pServer->updateConnParams(peer.getConnHandle(), 12, 24, 0, 400);

      release();
    }

    void BleMiRemote::onDisconnect(NimBLEServer *pServer, NimBLEConnInfo& connInfo, int reason) {
      this->_connected = false;

      ESP_LOGI(TAG, "Disconnected: %s, reason=0x%02x", connInfo.getAddress().toString().c_str(), reason);

      if (this->_reconnect && this->_should_readvertise) {
        this->start_reconnect_advert_();
      }
    }

    void BleMiRemote::onWrite(NimBLECharacteristic *me, NimBLEConnInfo& connInfo) {
      uint8_t *value = (uint8_t*) (me->getValue().c_str());
      (void) value;
      ESP_LOGD(TAG, "special keys: %d", *value);
    }

    void BleMiRemote::on_shutdown() {
      ESP_LOGD(TAG, "on_shutdown FIRED!!!");
      this->stop();
    }

    void BleMiRemote::on_safe_shutdown() {
      ESP_LOGD(TAG, "on_safe_shutdown FIRED!!!");
      this->stop();
    }


    void BleMiRemote::delay_ms(uint64_t ms) {
      uint64_t m = esp_timer_get_time();
      if (ms) {
        uint64_t e = (m + (ms * 1000));
        if (m > e) { //overflow
          while (esp_timer_get_time() > e) {
          }
        }
        while (esp_timer_get_time() < e) {
        }
      }
    }
  }  // namespace ble_mi_remote
}  // namespace esphome

#endif
