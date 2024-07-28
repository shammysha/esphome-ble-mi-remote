
#ifdef USE_ESP32

#include "ble_mi_remote.h"
#include <NimBLEServer.h>
#include <NimBLEDevice.h>
#include <NimBLEService.h>
#include <NimBLEUtils.h>
#include <NimBLEHIDDevice.h>
#include <NimBLECharacteristic.h>
#include <NimBLEAdvertising.h>
#include "HIDTypes.h"
#include "HIDKeyboardTypes.h"
#include <driver/adc.h>
#include "sdkconfig.h"
#include <string>
#include <list>
#include "esphome/core/log.h"

#define CONSUMER_ID 0x01
#define KEYBOARD_ID 0x02

static const uint8_t _hidReportDescriptor[] = {
		USAGE_PAGE(1),			0x0C,			// Consumer
		USAGE(1),				0x01,			// Consumer Control

		COLLECTION(1),			0x01,			// Application

		REPORT_ID(1),			CONSUMER_ID,	//
		LOGICAL_MINIMUM(1),		0x00,			//
		LOGICAL_MAXIMUM(1),		0x01,			//
		REPORT_SIZE(1),			0x01,			//
		REPORT_COUNT(1),		0x19,			// 25
		USAGE(1),				0x41,			// Menu Pick
		USAGE(1),				0x42,			// Menu Up
		USAGE(1),				0x43,			// Menu Down
		USAGE(1),				0x44,			// Menu Left
		USAGE(1),				0x45,			// Menu Right
		USAGE(1),				0x30,			// Power
		USAGE(1),				0xCF,			// Voice Command
		USAGE(1),				0xE9,			// Volume Increment
		USAGE(1),				0xEA,			// Volume Decrement
		USAGE(2),				0xA2, 0x01,		// Instance 41473 (Select Task/Application)
		USAGE(2),				0xB8, 0x01,		// Instance 47105 (Movie Browser)
		USAGE(1),				0x6A,			// Green Menu Button
		USAGE(1),				0x6B,			// Blue Menu Button
		USAGE(1),				0x6C,			// Yellow Menu Button
		USAGE(2),				0x8A, 0x01,		// Instance 35329 (Email reader??)
		USAGE(2),				0x92, 0x01,		// Instance 37377 (Calculator)
		USAGE(2),				0x94, 0x01,		// Instance 37889 (Local Machine Browser)
		USAGE(1),				0xCF,			// Voice Command
		USAGE(2),				0x23, 0x02,		// Instance 8962 (Home)
		USAGE(2),				0x24, 0x02,		// Instance 9218 (Back)
		USAGE(2),				0x25, 0x02,		// Instance 9474 (Forward)
		USAGE(2),				0x26, 0x02,		// Instance 9730 (Stop)
		USAGE(2),				0x27, 0x02,		// Instance 9986 (Refresh)
		USAGE(2),				0x2A, 0x02,		// Instance 10754 (Bookmarks)
    USAGE(1),       0x40,     // Menu		
		HIDINPUT(1),			0x02,			// In bytes

		USAGE_PAGE(2),			0x00, 0xFF,		// Vendor ??? переставить местами байты?
		USAGE(1),				0x00,			// Vendor

		COLLECTION(1),			0x01,			// Application

		REPORT_ID(1),			0x06,			//
		REPORT_SIZE(1),			0x08,			//
		REPORT_COUNT(1),		0x78,			// 120
		LOGICAL_MINIMUM(1),		0x00,			//
		LOGICAL_MAXIMUM(1),		0xFF,			// -1
		USAGE_MINIMUM(1),		0x00,			//
		USAGE_MAXIMUM(1),		0xFF,			//
		HIDINPUT(1),			0x00,			//

		REPORT_ID(1),			0x07,			//
		REPORT_SIZE(1),			0x08,			//
		REPORT_COUNT(1),		0x78,			// 120
		LOGICAL_MINIMUM(1),		0x00,			//
		LOGICAL_MAXIMUM(1),		0xFF,			// -1
		USAGE_MINIMUM(1),		0x00,			//
		USAGE_MAXIMUM(1),		0xFF,			//
		HIDINPUT(1),			0x00,			//

		REPORT_ID(1),			0x08,			//
		REPORT_SIZE(1),			0x08,			//
		REPORT_COUNT(1),		0x78,			// 120
		LOGICAL_MINIMUM(1),		0x00,			//
		LOGICAL_MAXIMUM(1),		0xFF,			// 255
		USAGE_MINIMUM(1),		0x00,			//
		USAGE_MAXIMUM(1),		0xFF,			//
		HIDINPUT(1),			0x00,			//

		END_COLLECTION(0),

		USAGE_PAGE(1),			0x01,			// Generic Desktop Controls
		USAGE(1),				0x06,			// Keyboard

		COLLECTION(1),			0x01,			// Application

		REPORT_ID(1),			KEYBOARD_ID,	//
		USAGE_PAGE(1),			0x07,			// Keyboard/Keypad
		USAGE_MINIMUM(1),		0xE0,			// Ctrl, Shift, Alt, GUI
		USAGE_MAXIMUM(1),		0xE7,			// Ctrl, Shift, Alt, GUI
		LOGICAL_MINIMUM(1),		0x00,			//
		LOGICAL_MAXIMUM(1),		0x01,			//
		REPORT_SIZE(1),			0x01,			//
		REPORT_COUNT(1),		0x08,			//
		HIDINPUT(1),			0x02,			//
		REPORT_COUNT(1),		0x01,			//
		REPORT_SIZE(1),			0x08,			//
		HIDINPUT(1),			0x01,			//

		REPORT_COUNT(1),		0x05,			//
		REPORT_SIZE(1),			0x01,			//
		USAGE_PAGE(1),			0x08,			// LED
		USAGE_MINIMUM(1),		0x01,			//
		USAGE_MAXIMUM(1),		0x05,			//
		HIDOUTPUT(1),			0x02,			//
		REPORT_COUNT(1),		0x01,			//
		REPORT_SIZE(1),			0x03,			//
		HIDOUTPUT(1),			0x01,			//

		REPORT_COUNT(1),		0x06,			//
		REPORT_SIZE(1),			0x08,			//
		LOGICAL_MINIMUM(1),		0x28,			// 40
		LOGICAL_MAXIMUM(1),		0xFE,			// 254
		USAGE_PAGE(1),			0x07,			// Keyboard/Keypad
		USAGE_MINIMUM(1),		0x28,			// 40
		USAGE_MAXIMUM(1),		0xFE,			// 254
		HIDINPUT(1),			0x00,			//

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
			ESP_LOGI(TAG, "Setting up...");

			NimBLEDevice::init(deviceName);
			NimBLEServer *pServer = NimBLEDevice::createServer();
			pServer->setCallbacks(this);

			hid = new NimBLEHIDDevice(pServer);
			inputSpecialKeys = hid->inputReport(CONSUMER_ID);
			inputKeyboard = hid->inputReport(KEYBOARD_ID);
			outputKeyboard = hid->outputReport(KEYBOARD_ID);
			outputKeyboard->setCallbacks(this);

			vendorReport_06 = hid->inputReport(0x06);
			vendorReport_07 = hid->inputReport(0x07);
			vendorReport_08 = hid->inputReport(0x08);

			hid->manufacturer()->setValue(deviceManufacturer);
			hid->pnp(sid, vid, pid, version);
			hid->hidInfo(0x00, 0x00);

			NimBLEDevice::setSecurityAuth(true, true, true);

			hid->reportMap((uint8_t*) _hidReportDescriptor, sizeof(_hidReportDescriptor));
			hid->startServices();

			onStarted(pServer);

			advertising = pServer->getAdvertising();
			advertising->setAppearance(HID_KEYBOARD);
			advertising->addServiceUUID(hid->hidService()->getUUID());
			advertising->setScanResponse(false);

			advertising->start();

			hid->setBatteryLevel(batteryLevel);

			ESP_LOGD(TAG, "Advertising started!");

			pServer = NimBLEDevice::getServer();

			pServer->advertiseOnDisconnect(this->_reconnect);

			release();
		}

		void BleMiRemote::stop() {
			if (this->_reconnect) {
				pServer->advertiseOnDisconnect(false);
			}

			std::vector<uint16_t> ids = pServer->getPeerDevices();

			if (ids.size() > 0) {
				for (uint16_t &id : ids) {
					pServer->disconnect(id);
				}
			} else {
				pServer->stopAdvertising();
			}
		}

		void BleMiRemote::start() {
			if (this->_reconnect) {
				pServer->advertiseOnDisconnect(true);
			}

			pServer->startAdvertising();
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
			this->cancel_timeout((const std::string) TAG);
			this->set_timeout((const std::string) TAG, _release_delay, [this]() { this->release(); });
		}


		void BleMiRemote::sendReport(KeyReport *keys) {
			if (this->is_connected()) {
				this->inputKeyboard->setValue((uint8_t*) keys, sizeof(KeyReport));
				this->inputKeyboard->notify();
				this->delay_ms(_delay_ms);
			}
		}

		void BleMiRemote::sendReport(SpecialKeyReport *keys) {
			if (this->is_connected()) {
				this->inputSpecialKeys->setValue((uint8_t*) keys, sizeof(SpecialKeyReport));
				this->inputSpecialKeys->notify();
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
				0x2a,			// BS	Backspace
				0x2b,			// TAB	Tab
				0x28,			// LF	Enter
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

				0x2c,		   //  ' '
				0x1e | SHIFT,	   // !
				0x34 | SHIFT,	   // "
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
				0				// DEL
		};

		uint8_t USBPutChar(uint8_t c);

		void BleMiRemote::press(uint8_t k, bool with_timer) {
			if (this->is_connected()) {
				if (with_timer) {
					this->update_timer();
				}

				uint8_t i;
				if (k >= 136) {			// it's a non-printing key (not a modifier)
					k = k - 136;
				} else if (k >= 128) {	// it's a modifier key
					_keyReport.modifiers |= (1 << (k - 128));
					k = 0;
				} else {				// it's a printing key
					k = pgm_read_byte(_asciimap + k);
					if (!k) {

						return;
					}
					if (k & 0x80) {						// it's a capital letter or other character reached with shift
						_keyReport.modifiers |= 0x02;	// the left shift modifier
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

		void BleMiRemote::pressSpecial(uint8_t k, bool with_timer) {
			if (this->is_connected()) {
				if (with_timer) {
					this->update_timer();
				}
			    uint8_t bit = k % 8;
			    uint8_t byte = int(k / 8);

			    _specialKeyReport.keys[byte] |= (1 << bit);

			    ESP_LOGD(TAG, "Send: %d, %d, %d", _specialKeyReport.keys[0], _specialKeyReport.keys[1], _specialKeyReport.keys[2]);

			    sendReport (&_specialKeyReport);
			}
		}

		void BleMiRemote::release() {
			if (this->is_connected()) {
				this->cancel_timeout((const std::string) TAG);

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

		void BleMiRemote::onConnect(NimBLEServer *pServer) {
			this->_connected = true;
			NimBLEConnInfo peer = pServer->getPeerInfo(0);

			release();
		}

		void BleMiRemote::onDisconnect(NimBLEServer *pServer) {
			this->_connected = false;
		}

		void BleMiRemote::onWrite(NimBLECharacteristic *me) {
			uint8_t *value = (uint8_t*) (me->getValue().c_str());
			(void) value;
			ESP_LOGD(TAG, "special keys: %d", *value);
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
