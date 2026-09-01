
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

      this->loadTargetMac();

      NimBLEDevice::init(deviceName);
      this->pServer = NimBLEDevice::createServer();

      pServer->setCallbacks(this);
      // Always false: NimBLE's own auto-restart only knows plain undirected
      // advertising and would race right after onDisconnect() below, undoing
      // the directed reconnect burst startReconnectAdvert() just started.
      // onDisconnect() is now the sole place deciding whether/how to
      // re-advertise (gated on _reconnect there instead).
      pServer->advertiseOnDisconnect(false);

      hid = new NimBLEHIDDevice(pServer);
      inputSpecialKeys = hid->getInputReport(CONSUMER_ID);
      inputKeyboard = hid->getInputReport(KEYBOARD_ID);
      outputKeyboard = hid->getOutputReport(KEYBOARD_ID);
      outputKeyboard->setCallbacks(this);
      // Diagnostic-only (2026-08-26): previously only outputKeyboard had
      // callbacks attached, so onRead/onSubscribe/onStatus never fired for
      // the actual input-report characteristics sendReport() writes to -
      // exactly the ones a real HID host would read/subscribe to during
      // its own setup handshake.
      inputSpecialKeys->setCallbacks(this);
      inputKeyboard->setCallbacks(this);

      vendorReport_06 = hid->getInputReport(0x06);
      vendorReport_07 = hid->getInputReport(0x07);
      vendorReport_08 = hid->getInputReport(0x08);

      hid->setManufacturer(deviceManufacturer);
      hid->setPnp(sid, vid, pid, version);
      // HID Information flags (2026-09-01): read directly off the genuine
      // Xiaomi remote via pub.home's own BLE adapter (bluetoothctl paired +
      // busctl GattCharacteristic1.ReadValue) - bcdHID=0x0000, CountryCode=0,
      // Flags=0x01 (RemoteWake only, NOT NormallyConnectable). We'd
      // previously tried 0x00 (both false) and 0x03 (both true) - neither
      // matched the real device and neither changed the ~17-20s real-Mi-TV
      // disconnect timing at all. This exact value is untested.
      hid->setHidInfo(0x00, 0x01);

      // Device Information Service fields (2026-09-01): the genuine remote
      // (paired directly via pub.home's Bluetooth adapter) has these set to
      // completely unmodified Realtek "RTKBee" BLE SDK reference-firmware
      // defaults, not anything Xiaomi-branded - Manufacturer Name is the
      // only field previously set here (via the existing, user-configurable
      // deviceManufacturer), everything below was entirely absent. Added
      // verbatim from the real device in case the TV's own compatibility/
      // validation layer keys off any of it during the otherwise-invisible
      // ~17s window before it disconnects.
      {
        NimBLEService* dis = hid->getDeviceInfoService();
        dis->createCharacteristic((uint16_t) 0x2a24, NIMBLE_PROPERTY::READ)->setValue("Model Nbr 0.9");    // Model Number String
        dis->createCharacteristic((uint16_t) 0x2a25, NIMBLE_PROPERTY::READ)->setValue("RTKBeeSerialNum");  // Serial Number String
        dis->createCharacteristic((uint16_t) 0x2a26, NIMBLE_PROPERTY::READ)->setValue("19415");            // Firmware Revision String
        dis->createCharacteristic((uint16_t) 0x2a27, NIMBLE_PROPERTY::READ)->setValue("2.0.1.4");          // Hardware Revision String
        dis->createCharacteristic((uint16_t) 0x2a28, NIMBLE_PROPERTY::READ)->setValue("2.3");              // Software Revision String
        dis->createCharacteristic((uint16_t) 0x2a2a, NIMBLE_PROPERTY::READ)->setValue("RTKBeeIEEEDatalist"); // IEEE 11073-20601 Regulatory Cert. Data List
        static const uint8_t sysIdVal[8] = {0x00, 0x01, 0x02, 0x00, 0x00, 0x03, 0x04, 0x05};
        dis->createCharacteristic((uint16_t) 0x2a23, NIMBLE_PROPERTY::READ)->setValue(sysIdVal, sizeof(sysIdVal)); // System ID
      }

      NimBLEDevice::setSecurityAuth(true, true, true);

      hid->setReportMap((uint8_t*) _hidReportDescriptor, sizeof(_hidReportDescriptor));

      // Descriptor-level diagnostics (2026-09-01) - see the block comment on
      // onRead(NimBLEDescriptor*, ...) in the header. Report Reference
      // (0x2908) is the descriptor a real HOGP host reads on each Report
      // characteristic during service discovery to learn its report ID/type -
      // exactly the kind of activity that could fill the ~17s the real Mi TV
      // spends connected+subscribed before giving up, invisible to us until
      // now since only characteristic-level callbacks were ever wired.
      inputSpecialKeys->getDescriptorByUUID((uint16_t) 0x2908)->setCallbacks(this);
      inputKeyboard->getDescriptorByUUID((uint16_t) 0x2908)->setCallbacks(this);
      outputKeyboard->getDescriptorByUUID((uint16_t) 0x2908)->setCallbacks(this);
      vendorReport_06->getDescriptorByUUID((uint16_t) 0x2908)->setCallbacks(this);
      vendorReport_07->getDescriptorByUUID((uint16_t) 0x2908)->setCallbacks(this);
      vendorReport_08->getDescriptorByUUID((uint16_t) 0x2908)->setCallbacks(this);
      // Also wire the standard HID/DIS characteristics a real host reads
      // during its own initial setup handshake (HID Information, Report
      // Map, Protocol Mode, PnP ID, Battery Level) - previously none of
      // these had callbacks either, so a read on any of them was equally
      // invisible to us.
      hid->getHidInfo()->setCallbacks(this);
      hid->getReportMap()->setCallbacks(this);
      hid->getProtocolMode()->setCallbacks(this);
      hid->getPnp()->setCallbacks(this);
      hid->getBatteryLevel()->setCallbacks(this);

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
      // enableScanResponse() alone only flips a flag - NimBLEAdvertising::start()
      // only actually transmits scan response *content* if m_scanData already
      // has a non-empty payload (see NimBLEAdvertising.cpp: `if (m_scanResp &&
      // m_scanData.getPayload().size() > 0)`). Without this call the scan
      // response goes out completely empty (confirmed via live sniffer capture:
      // "Scan Response Data: <MISSING>", length 6 = address only, zero payload) -
      // a still-bonded central doesn't care (it already has the name cached from
      // the original pairing), but a scanner seeing this device for the first
      // time (nRF Connect, a fresh "add device" scan) sees a nameless device and
      // may not offer to pair with it at all. setName() with m_scanResp already
      // true routes the name into the scan response specifically, not the main
      // advertisement.
      advertising->setName(deviceName);

      this->startReconnectAdvert();

      hid->setBatteryLevel(batteryLevel);

      release();
    }

    void BleMiRemote::stop() {
      ESP_LOGI(TAG, "stop: entered, reconnect=%s", this->_reconnect ? "true" : "false");

      this->_should_readvertise = false;
      // A pending reconnect-burst fallback (startReconnectAdvert()) would
      // otherwise fire later and re-start advertising, undoing this stop().
      this->cancel_timeout("ble_mi_remote_reconnect_burst");

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
      this->startReconnectAdvert();
    }

    void BleMiRemote::update() { state_sensor_->publish_state(this->_connected); }

    // dump_config() output is cached and replayed to any client that
    // (re)subscribes to logs, unlike plain ESP_LOGI - the only reliable way
    // to see this component's boot-time state remotely, since setup() runs
    // and finishes well before a remote API log listener can ever attach.
    void BleMiRemote::dump_config() {
      ESP_LOGCONFIG(TAG, "BLE Mi Remote:");
      ESP_LOGCONFIG(TAG, "  Reconnect: %s", this->_reconnect ? "true" : "false");
      ESP_LOGCONFIG(TAG, "  Connect count: %u, Disconnect count: %u (this boot)", (unsigned) this->_connect_count, (unsigned) this->_disconnect_count);
      if (this->_has_target_mac) {
        ESP_LOGCONFIG(TAG, "  Target MAC: %s (%s)", NimBLEAddress(this->_target_mac, BLE_ADDR_PUBLIC).toString().c_str(), this->_target_mac_from_config ? "from config" : "learned");
      } else {
        ESP_LOGCONFIG(TAG, "  Target MAC: none yet");
      }
    }

    bool BleMiRemote::isConnected() {
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

    void BleMiRemote::updateTimer() {
      this->cancel_timeout(TAG);
      this->set_timeout(TAG, _release_delay, [this]() { this->release(); });
    }


    void BleMiRemote::sendReport(KeyReport *keys) {
      ESP_LOGD(TAG, "sendReport FIRING...");
      if (this->isConnected()) {
        ESP_LOGD(TAG, "sendReport FIRED!!!");

        this->inputKeyboard->setValue((uint8_t*) keys, sizeof(KeyReport));
        if (!this->inputKeyboard->notify()) {
          ESP_LOGE(TAG, "sendReport FAILED!!!");
        }
        this->delayMs(_delay_ms);
      }
    }

    void BleMiRemote::sendReport(SpecialKeyReport *keys) {
      ESP_LOGD(TAG, "sendReport FIRING...");
      if (this->isConnected()) {
        ESP_LOGD(TAG, "sendReport FIRED!!!");

        this->inputSpecialKeys->setValue((uint8_t*) keys, sizeof(SpecialKeyReport));
        if (!this->inputSpecialKeys->notify()) {
          ESP_LOGE(TAG, "sendReport FAILED!!!");
        }
        this->delayMs(_delay_ms);
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
      if (this->isConnected()) {
        if (with_timer) {
          this->updateTimer();
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
      if (this->isConnected()) {
        if (with_timer) {
          this->updateTimer();
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
      if (this->isConnected()) {
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
    void BleMiRemote::loadTargetMac() {
      this->_target_mac_pref = global_preferences->make_preference<uint64_t>(fnv1_hash("ble_mi_remote_target_mac"));

      if (this->_target_mac_from_config) {
        ESP_LOGI(TAG, "loadTargetMac: using target_mac_address from config: %s", NimBLEAddress(this->_target_mac, BLE_ADDR_PUBLIC).toString().c_str());
        return;
      }

      uint64_t stored = 0;
      if (this->_target_mac_pref.load(&stored) && stored != 0) {
        this->_target_mac = stored;
        this->_has_target_mac = true;
        ESP_LOGI(TAG, "loadTargetMac: loaded learned target %s from flash", NimBLEAddress(stored, BLE_ADDR_PUBLIC).toString().c_str());
      } else {
        ESP_LOGI(TAG, "loadTargetMac: no target_mac_address configured and nothing learned yet");
      }
    }

    void BleMiRemote::learnTargetMac(NimBLEAddress addr) {
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
      ESP_LOGI(TAG, "learnTargetMac: learned peer %s, saved to flash=%s", addr.toString().c_str(), ok ? "OK" : "FAILED");
    }

    // Raw plain/undirected advertising start - touches no bond state. This
    // is what every *automatic* fallback path (no target MAC yet, no saved
    // bond, a plain mid-session disconnect) calls: the box being off is a
    // completely normal, expected outcome (it's not a 24/7 device) and must
    // never by itself cost us a saved LTK/IRK - only plainAdvertStart()'s
    // deliberate manual escape hatch and a *confirmed* saved-key auth
    // rejection (onAuthenticationComplete()) are allowed to touch bonds.
    void BleMiRemote::startPlainAdvertising() {
      NimBLEAdvertising *adv = pServer->getAdvertising();
      adv->setConnectableMode(BLE_GAP_CONN_MODE_UND);
      adv->setHighDutyCycleDirected(false);
      bool stopOk = adv->stop();
      bool startOk = adv->start();
      ESP_LOGI(TAG, "startPlainAdvertising: stop=%s start=%s", stopOk ? "OK" : "FAILED", startOk ? "OK" : "FAILED");
    }

    // Manual test/escape-hatch action (ble_mi_remote.plain_advert): explicit
    // user intent to force a completely fresh pairing, regardless of the
    // HD-burst/retry logic in startReconnectAdvert().
    void BleMiRemote::plainAdvertStart() {
      // A fresh manual pairing attempt should not be able to collide with
      // whatever bond (LTK/IRK) is left over from a previous one - clear it
      // first so the box is forced into a genuinely new pairing rather than
      // risking a mismatched-key auth failure against stale local state.
      // Deliberate, human-initiated action only - see startPlainAdvertising()
      // for why every automatic path avoids this.
      bool deleteOk = NimBLEDevice::deleteAllBonds();
      ESP_LOGI(TAG, "plainAdvertStart: deleteAllBonds()=%s", deleteOk ? "OK" : "FAILED");

      this->startPlainAdvertising();
    }

    // Mirrors what the real Xiaomi remote (and a compatible third-party
    // remote, independently confirmed the same way) actually does on
    // power-up, confirmed by live nRF52840 sniffer captures: a burst of
    // ADV_DIRECT_IND at high duty cycle (~3.75ms/channel) targeted at the
    // bonded central's address (MAC learned via learnTargetMac()),
    // instead of our previous plain undirected advertising. High duty cycle
    // is a separate flag from conn_mode/duration - see
    // setHighDutyCycleDirected() (our esp-nimble-cpp fork's addition, since
    // upstream never exposed ble_gap_adv_params.high_duty_cycle at all).
    // Falls back to normal undirected advertising once the burst window
    // elapses without a connection.
    void BleMiRemote::startReconnectAdvert() {
      if (!this->_has_target_mac) {
        // Nothing to send a directed burst *to* yet (first-ever boot) -
        // plain/discoverable advertising is the only option, same as every
        // other legitimate automatic-plain-advert trigger below.
        ESP_LOGI(TAG, "startReconnectAdvert: no target_mac_address learned yet, falling back to plain advertising");
        this->startPlainAdvertising();
        return;
      }

      NimBLEAddress dirAddr(this->_target_mac, BLE_ADDR_PUBLIC);
      if (!NimBLEDevice::isBonded(dirAddr)) {
        // We know *who* to target but hold no bond (LTK/IRK) for them -
        // e.g. the target was learned from a connection that never actually
        // bonded, or a confirmed auth failure already cleared it (see
        // onAuthenticationComplete()). A directed burst only makes sense as
        // a reconnect to an already-bonded peer; without a bond there's
        // nothing to reconnect to, so don't waste the 30s retry window on
        // it. No bond to preserve here either way - nothing to delete.
        ESP_LOGI(TAG, "startReconnectAdvert: no saved bond for %s, falling back to plain advertising", dirAddr.toString().c_str());
        this->startPlainAdvertising();
        return;
      }

      // A single 1.28s HD burst matches a genuine remote's packet
      // byte-for-byte (confirmed via sniffer) but still wasn't enough by
      // itself for the box to notice/respond in testing - the previous
      // one-shot-then-fall-back-to-slow-undirected-forever design gives up
      // right at the moment the box might still be waking its own scanner
      // up. Instead: keep firing fresh back-to-back HD bursts (the
      // controller hard-caps each one at 1.28s regardless) for up to 30s
      // before giving up to plain undirected advertising.
      this->_reconnect_retry_until_ms = millis() + 30000;
      this->fireDirectedBurst();
    }

    // setConnectableMode()/setHighDutyCycleDirected() are the actual
    // switches NimBLE/the controller use to pick ADV_DIRECT_IND (and its
    // duty cycle) - start()'s dirAddr/duration arguments alone are NOT
    // enough (confirmed the hard way: without setConnectableMode(DIR), a
    // live nRF52840 sniffer capture showed dirAddr being silently ignored
    // and plain undirected advertising going out regardless; without
    // setHighDutyCycleDirected(true), the burst transmitted but at a slow
    // ~65ms interval instead of the real remotes' ~3.75ms). Both are sticky
    // and must be reset for the undirected fallback.
    void BleMiRemote::fireDirectedBurst() {
      NimBLEAdvertising *adv = pServer->getAdvertising();
      NimBLEAddress dirAddr(this->_target_mac, BLE_ADDR_PUBLIC);
      adv->setConnectableMode(BLE_GAP_CONN_MODE_DIR);
      adv->setHighDutyCycleDirected(true);
      bool stopOk = adv->stop();
      bool startOk = adv->start(1280, &dirAddr);
      ESP_LOGI(TAG, "fireDirectedBurst: HD directed burst to %s, stop=%s start=%s, retry_remaining_ms=%d", dirAddr.toString().c_str(), stopOk ? "OK" : "FAILED", startOk ? "OK" : "FAILED", (int) (this->_reconnect_retry_until_ms - millis()));

      this->set_timeout("ble_mi_remote_reconnect_burst", 1300, [this]() {
        if (this->_connected) {
          // A real connection already completed during the burst window -
          // the controller auto-stops advertising once connected, so
          // there's nothing to fall back to.
          ESP_LOGI(TAG, "fireDirectedBurst: burst window elapsed, already connected - nothing to do");
          return;
        }

        if ((int32_t) (millis() - this->_reconnect_retry_until_ms) < 0) {
          // Still within the retry window - fire another burst immediately
          // instead of falling back.
          this->fireDirectedBurst();
          return;
        }

        // Retry window exhausted - stop advertising entirely rather than
        // auto-falling back to plain undirected (explicit user instruction:
        // plain/undirected advertising must only ever be started manually,
        // via plainAdvertStart()/the plain_advert action - never
        // automatically).
        NimBLEAdvertising *adv2 = pServer->getAdvertising();
        bool stopOk2 = adv2->stop();
        ESP_LOGI(TAG, "fireDirectedBurst: retry window exhausted, stopping advertising (stop=%s) - manual plain_advert needed to resume", stopOk2 ? "OK" : "FAILED");
      });
    }

    void BleMiRemote::onConnect(NimBLEServer *pServer, NimBLEConnInfo& connInfo) {
      this->_connected = true;
      this->_connect_count++;
      this->_connect_millis = millis();
      NimBLEConnInfo peer = connInfo;

      ESP_LOGI(TAG, "Connected: %s", peer.getAddress().toString().c_str());
      // Diagnostic: NimBLEHIDDevice's input report characteristic requires
      // encryption (READ_ENC) for a direct read, but its NOTIFY property is
      // granted unconditionally - so an unbonded/unencrypted central can, in
      // principle, subscribe and receive our sendReport()/notify() calls
      // (which would explain "sendReport FIRED!!! but nothing happens on
      // the TV" if the host OS silently drops HID input from an
      // unauthenticated link per HOGP policy, while we see local success).
      // Logging this to confirm/rule out on the next real connect.
      ESP_LOGI(TAG, "Connected: bonded=%s encrypted=%s authenticated=%s", peer.isBonded() ? "true" : "false", peer.isEncrypted() ? "true" : "false", peer.isAuthenticated() ? "true" : "false");

      // Confirmed via the above diagnostic (real hardware, 2026-08-25): the
      // box connects without ever bonding/encrypting on its own - it's never
      // forced to, since NOTIFY isn't ENC-gated. Request security ourselves
      // instead of waiting for a central that apparently never will -
      // standard "peripheral-initiated pairing". No-op if already bonded.
      //
      // Tested disabling this on the real Mi TV (2026-08-26) to see if the
      // rc=2/BLE_HS_EALREADY race (Mi TV negotiates security on its own,
      // unlike AM8) was behind the ~18s-later disconnect - did NOT help,
      // restored as-is. Not the cause.
      if (!peer.isBonded()) {
        int rc = 0;
        bool startOk = NimBLEDevice::startSecurity(peer.getConnHandle(), &rc);
        ESP_LOGI(TAG, "Connected: not bonded, requesting security: startSecurity()=%s rc=%d", startOk ? "OK" : "FAILED", rc);
      }

      this->learnTargetMac(peer.getAddress());

      // Explicit connection-parameter/supervision-timeout request deferred
      // to onAuthenticationComplete() instead of here - see there for why.

      release();
    }

    // Fires once the security procedure startSecurity() kicked off in
    // onConnect() actually finishes (success or failure) - event-driven,
    // unlike guessing from bonded/encrypted state on a later reconnect.
    void BleMiRemote::onAuthenticationComplete(NimBLEConnInfo& connInfo) {
      ESP_LOGI(TAG, "onAuthenticationComplete: bonded=%s encrypted=%s authenticated=%s", connInfo.isBonded() ? "true" : "false", connInfo.isEncrypted() ? "true" : "false", connInfo.isAuthenticated() ? "true" : "false");

      if (!connInfo.isBonded()) {
        // startSecurity() ran against a peer we believed we already held
        // keys for (onConnect() only calls it when !isBonded() at connect
        // time, so getting here at all with a still-failed bond means our
        // saved keys were rejected). This is the ONE automatic case allowed
        // to touch bond storage - and only this specific peer's entry, not
        // deleteAllBonds(): a merely failed/timed-out HD-burst (e.g. the box
        // is simply off right now - not 24/7) must never cost us a key, but
        // a *confirmed* rejection like this one would otherwise leave a
        // permanently-bad bond that isBonded() keeps reporting as present,
        // making startReconnectAdvert() retry the same failing HD burst
        // forever instead of ever reaching the plain-advertising fallback.
        bool deleteOk = NimBLEDevice::deleteBond(connInfo.getAddress());
        ESP_LOGW(TAG, "onAuthenticationComplete: saved-key authentication failed, deleteBond(%s)=%s, disconnecting to trigger plain-advertising fallback", connInfo.getAddress().toString().c_str(), deleteOk ? "OK" : "FAILED");
        pServer->disconnect(connInfo.getConnHandle());
        return;
      }

      // Explicit supervision timeout so a peer that silently vanishes at the
      // link layer (radio gone - box powered off ungracefully, out of
      // range, etc.) gets detected and torn down within a bounded time,
      // instead of leaving us believing we're still connected (and
      // therefore never re-entering startReconnectAdvert()) for however
      // long the central happened to negotiate at connect time (up to 32s
      // per spec, entirely outside our control). Placed here rather than in
      // onConnect() (right alongside startSecurity()) specifically to avoid
      // an earlier-suspected race between the two requests during the
      // sensitive pairing window - by the time we get here, security has
      // already succeeded, so there's nothing left to race with.
      // min/max interval 15/30ms, no peripheral latency, 4s supervision
      // timeout (400 * 10ms).
      //
      // Tested disabling this on the real Mi TV (2026-08-26): the box still
      // dropped the link ~18.7s after bonding, same as with it enabled
      // (~18.5s) and same as with startSecurity() also disabled (~18.6s) -
      // three separate tests, three near-identical intervals regardless of
      // what our code does here. Not the cause - the box has some fixed
      // timeout of its own that fires this consistently after bonding
      // completes, unrelated to connection parameters. Restored as-is.
      pServer->updateConnParams(connInfo.getConnHandle(), 12, 24, 0, 400);
      ESP_LOGI(TAG, "onAuthenticationComplete: updateConnParams() requested");
    }

    void BleMiRemote::onDisconnect(NimBLEServer *pServer, NimBLEConnInfo& connInfo, int reason) {
      this->_connected = false;
      this->_disconnect_count++;

      ESP_LOGI(TAG, "Disconnected: %s, reason=0x%02x, elapsed_since_connect_ms=%u", connInfo.getAddress().toString().c_str(), reason, (unsigned) (millis() - this->_connect_millis));

      // Also cancel any still-pending HD-burst retry from a sequence that
      // hadn't finished its own 30s window yet.
      this->cancel_timeout("ble_mi_remote_reconnect_burst");

      if (!this->_reconnect || !this->_should_readvertise) {
        return;
      }

      // BLE_ERR_REM_USER_CONN_TERM (0x13, reported here offset by
      // BLE_HS_ERR_HCI_BASE = 0x200, i.e. 0x213) means the peer's own BLE
      // stack was still fully alive and *chose* to end the link - this is
      // what a human manually disconnecting/forgetting the device in the
      // box's Bluetooth settings produces (confirmed live: user forgot the
      // device, our HD-burst reconnected within ~400ms and silently
      // re-bonded via Just Works before they could even open "Add device" -
      // explicitly unwanted, "он не должен возвращаться"). A genuine power
      // loss/reboot of the box gives its BLE stack no time for a graceful
      // goodbye and shows up as a supervision-timeout disconnect instead
      // (a different reason code) - that case is unaffected and still
      // triggers the reconnect dispatcher below as before.
      //
      // Explicit user instruction (2026-08-26): always delete this one
      // peer's bond here too, not just on a confirmed auth rejection
      // (targeted deleteBond(), not deleteAllBonds() - same reasoning as
      // onAuthenticationComplete()'s point deletion). A deliberate peer-
      // initiated termination has consistently correlated with the peer
      // also dropping its own side of the bond in this session's real
      // testing (both the manual "forget device" case and the real Mi
      // TV's own "Add device" cleanup) - keeping our half around after
      // that just means the next reconnect attempt silently fails against
      // a peer that no longer recognizes us, instead of properly falling
      // through to a fresh, discoverable plain advert.
      if (reason == BLE_HS_ERR_HCI_BASE + BLE_ERR_REM_USER_CONN_TERM) {
        bool deleteOk = NimBLEDevice::deleteBond(connInfo.getAddress());
        ESP_LOGI(TAG, "Disconnected: peer deliberately terminated the link (reason=0x%02x), deleteBond(%s)=%s - plain advertising only, no HD-burst reconnect", reason, connInfo.getAddress().toString().c_str(), deleteOk ? "OK" : "FAILED");
        this->startPlainAdvertising();
        return;
      }

      // A mid-session disconnect is just as legitimate a reconnect scenario
      // as the boot-time one - most commonly the box simply being powered
      // off (it's not a 24/7 device). Reuse the same dispatcher setup()/
      // start() use: if we still hold a valid bond for the target, retry
      // via HD-burst so the box gets a genuine reconnect burst the moment
      // it's powered back on, exactly like a real remote; a failed/timed-
      // out HD-burst here is expected and must not touch saved keys. Only
      // a target-less or bond-less state falls through to plain
      // advertising.
      this->startReconnectAdvert();
    }

    void BleMiRemote::onWrite(NimBLECharacteristic *me, NimBLEConnInfo& connInfo) {
      uint8_t *value = (uint8_t*) (me->getValue().c_str());
      (void) value;
      ESP_LOGD(TAG, "special keys: %d", *value);
    }

    // Diagnostic-only overrides (2026-08-26) - see the block comment on
    // their declarations in ble_mi_remote.h. Every one logs
    // elapsed_since_connect_ms so a real-Mi-TV capture can be lined up
    // against the ~18.5s mark where the TV has, so far, always disconnected
    // regardless of what we've tried on our own side.
    void BleMiRemote::onMTUChange(uint16_t mtu, NimBLEConnInfo& connInfo) {
      ESP_LOGI(TAG, "onMTUChange: mtu=%u, elapsed_since_connect_ms=%u", (unsigned) mtu, (unsigned) (millis() - this->_connect_millis));
    }

    void BleMiRemote::onConnParamsUpdate(NimBLEConnInfo& connInfo) {
      ESP_LOGI(TAG, "onConnParamsUpdate: interval=%u latency=%u timeout=%u, elapsed_since_connect_ms=%u", (unsigned) connInfo.getConnInterval(), (unsigned) connInfo.getConnLatency(), (unsigned) connInfo.getConnTimeout(), (unsigned) (millis() - this->_connect_millis));
    }

    void BleMiRemote::onIdentity(NimBLEConnInfo& connInfo) {
      ESP_LOGI(TAG, "onIdentity: %s, elapsed_since_connect_ms=%u", connInfo.getAddress().toString().c_str(), (unsigned) (millis() - this->_connect_millis));
    }

    void BleMiRemote::onPhyUpdate(NimBLEConnInfo& connInfo, uint8_t txPhy, uint8_t rxPhy) {
      ESP_LOGI(TAG, "onPhyUpdate: txPhy=%u rxPhy=%u, elapsed_since_connect_ms=%u", (unsigned) txPhy, (unsigned) rxPhy, (unsigned) (millis() - this->_connect_millis));
    }

    void BleMiRemote::onRead(NimBLECharacteristic *me, NimBLEConnInfo& connInfo) {
      ESP_LOGI(TAG, "onRead: uuid=%s, elapsed_since_connect_ms=%u", me->getUUID().toString().c_str(), (unsigned) (millis() - this->_connect_millis));
    }

    void BleMiRemote::onSubscribe(NimBLECharacteristic *me, NimBLEConnInfo& connInfo, uint16_t subValue) {
      ESP_LOGI(TAG, "onSubscribe: uuid=%s subValue=%u (%s), elapsed_since_connect_ms=%u", me->getUUID().toString().c_str(), (unsigned) subValue, subValue == 0 ? "unsubscribed" : (subValue & 0x0001 ? "notify" : (subValue & 0x0002 ? "indicate" : "?")), (unsigned) (millis() - this->_connect_millis));

      // Real-Mi-TV test (2026-09-01): setup()'s own release() call at boot
      // sends its two sendReport()s well before any peer has subscribed -
      // dropped silently, no subscriber yet (confirmed: onSubscribe fires
      // at ~3.3s into the connection, sendReport() at ~0s). After that
      // subscribe, nothing ever sends another report until a human presses
      // a button - and a real capture showed the TV then just sitting
      // subscribed+idle for ~17s before giving up and disconnecting
      // (0x213), unsubscribing 8ms before. Sending a neutral/"keys up"
      // report right when notifications actually get enabled - once real
      // subscribers exist to receive it - tests whether the TV is simply
      // waiting to see the link is alive before it considers the HID
      // device valid.
      if (subValue != 0) {
        this->release();
      }
    }

    void BleMiRemote::onStatus(NimBLECharacteristic *me, NimBLEConnInfo& connInfo, int code) {
      ESP_LOGI(TAG, "onStatus: uuid=%s code=%d, elapsed_since_connect_ms=%u", me->getUUID().toString().c_str(), code, (unsigned) (millis() - this->_connect_millis));
    }

    // Descriptor-level diagnostics (2026-09-01) - see the block comment on
    // the declaration in ble_mi_remote.h. Wired onto the Report Reference
    // descriptor (0x2908) of every Report characteristic plus the standard
    // HID/DIS characteristics themselves, to catch the real Mi TV's ~17s of
    // otherwise-invisible activity between subscribing and unsubscribing/
    // disconnecting - a real capture (2026-09-01) showed a full live,
    // encrypted, subscribed connection with zero onRead/onWrite/onStatus at
    // the characteristic level in that whole window.
    void BleMiRemote::onRead(NimBLEDescriptor *me, NimBLEConnInfo& connInfo) {
      ESP_LOGI(TAG, "onRead (descriptor): uuid=%s, elapsed_since_connect_ms=%u", me->getUUID().toString().c_str(), (unsigned) (millis() - this->_connect_millis));
    }

    void BleMiRemote::onWrite(NimBLEDescriptor *me, NimBLEConnInfo& connInfo) {
      ESP_LOGI(TAG, "onWrite (descriptor): uuid=%s, elapsed_since_connect_ms=%u", me->getUUID().toString().c_str(), (unsigned) (millis() - this->_connect_millis));
    }

    void BleMiRemote::on_shutdown() {
      ESP_LOGD(TAG, "on_shutdown FIRED!!!");
      this->stop();
    }

    void BleMiRemote::on_safe_shutdown() {
      ESP_LOGD(TAG, "on_safe_shutdown FIRED!!!");
      this->stop();
    }


    void BleMiRemote::delayMs(uint64_t ms) {
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
