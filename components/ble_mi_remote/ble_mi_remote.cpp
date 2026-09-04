
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
		REPORT_COUNT(1),		0x18,			// 24
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

			// own-commits bisection stage 3/5
			this->loadTargetMac();

			NimBLEDevice::init (deviceName);
			// Real bug, found 2026-09-03: this used to redeclare pServer as
			// a local ("NimBLEServer *pServer = ..."), which shadows the
			// class member of the same name for the rest of setup() - the
			// member this->pServer was therefore NEVER actually assigned
			// anywhere, staying null/garbage for the object's whole
			// lifetime. Every OTHER method that reads pServer (stop(),
			// powerAdvertData1/2/Stop, fireDirectedBurst, etc.) reads the
			// real member, not a local - some of those calls happened to
			// tolerate a null this (probably touching mostly-static NimBLE
			// internals), which is why this went unnoticed for so long;
			// pServer->getAdvertising() does not, and crashed hardware
			// (LoadProhibited) the first time powerAdvertStart() actually
			// got exercised. Assign the member directly - no local.
			pServer = NimBLEDevice::createServer();
			pServer->setCallbacks(this);

			hid = new NimBLEHIDDevice(pServer);
			// nimble-cpp-bisect: NimBLEHIDDevice renamed several methods in
			// tag 2.0.0 - [BREAKING] - Refactor NimBLEHIDDevice (6d10f2f).
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
			// nimble-cpp-bisect: hid->startServices() deprecated as of 2.5.0
			// ("Services are now started by the server when start() is
			// called") - pServer->start() is the direct replacement.
			pServer->start();

			onStarted(pServer);

			advertising = pServer->getAdvertising();
			advertising->setAppearance(HID_KEYBOARD);
			advertising->addServiceUUID(hid->getHidService()->getUUID());
			// nimble-cpp-bisect: setScanResponse() renamed to
			// enableScanResponse() by tag 2.0.0, same bool-argument shape.
			//
			// Real regression, found 2026-09-03: this reference was rebuilt
			// from the ancient pre-2026-08-25 baseline, which predates a
			// real, already-shipped fix (84ecd47) - enableScanResponse(true)
			// + setName() below, without which the device advertises with
			// no name anywhere at all (confirmed via sniffer: Flags +
			// Appearance + Service UUID only, no Complete Local Name).
			// enableScanResponse() only flips a flag; NimBLEAdvertising::
			// start() only actually transmits scan-response content if
			// m_scanData already has a payload, so the flag alone isn't
			// enough - setName() is what populates it. A nameless device
			// still transmits fine at the radio level (this session's own
			// sniffer capture confirmed that) but real-world TV/phone
			// "Add device" pickers commonly filter out unnamed devices
			// entirely, which is exactly the symptom that surfaced this.
			advertising->enableScanResponse(true);
			advertising->setName(deviceName);

			// Gap fix 2026-09-03: save the real advertising payload now,
			// while it's still whatever setup() above just built, so
			// powerAdvertStop() can restore it later - see the doc comment
			// on _normal_advert_data in the header.
			this->_normal_advert_data = advertising->getAdvertisementData();

			advertising->start();

			hid->setBatteryLevel(batteryLevel);

			ESP_LOGD(TAG, "Advertising started!");

			pServer = NimBLEDevice::getServer();

			// Gap fix, belongs to stage 4 (082953f, same commit that introduced
			// HD-burst itself): NimBLE's own advertiseOnDisconnect auto-restart
			// only knows plain undirected advertising and would race right after
			// our own onDisconnect() dispatcher, undoing a directed HD-burst it
			// just started. Always off - onDisconnect() is the sole place
			// deciding whether/how to re-advertise (already gated on
			// _should_readvertise there since stage 3).
			pServer->advertiseOnDisconnect(false);

			release();
		}

		// Gap fix, functional (not stage 5): esp-idf's stop()/start() moved off
		// the old advertiseOnDisconnect toggle (no longer meaningful now that
		// it's permanently false, set once in setup()) onto _should_readvertise
		// + the reconnect dispatcher directly - start() now actually goes
		// through startReconnectAdvert() (HD-burst if bonded) instead of a
		// bare advertising->start().
		void BleMiRemote::stop() {
			this->_should_readvertise = false;
			// A pending reconnect-burst fallback (startReconnectAdvert()) would
			// otherwise fire later and re-start advertising, undoing this stop().
			this->cancel_timeout("ble_mi_remote_reconnect_burst");

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
			this->_should_readvertise = true;
			this->startReconnectAdvert();
		}

		void BleMiRemote::on_shutdown() {
			this->stop();
		}

		void BleMiRemote::on_safe_shutdown() {
			this->stop();
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
					k = progmem_read_byte(_asciimap + k);
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
			} else {
				this->queuePendingCommand(k, false);
			}
		}

		// Mirrors the wake trick used by github.com/DenizOner/MiPower: instead of
		// this device advertising, act as a BLE central and directly attempt a
		// connection to the (already-bonded) target's MAC address, the same
		// effect as `bluetoothctl pair <mac>`. One-shot, blocking, short timeout.
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
			} else {
				this->queuePendingCommand(k, true);
			}
		}

		// own-commits bisection: powerAdvertStart/Data1/Data2/Stop, ported verbatim.
		void BleMiRemote::powerAdvertStart() {
			ESP_LOGI(TAG, "powerAdvertStart: entered");
			_power_advert_cycle = 0;
			this->powerAdvertData1();
		}

		void BleMiRemote::powerAdvertData1() {
			// Gap fix 2026-09-03: the normal payload (set in setup()) is
			// already at the 31-byte legacy cap with nothing free, so
			// setManufacturerData() on it was failing on every call,
			// including an empty one - measured on hardware, not a guess
			// (see _normal_advert_data doc comment in the header). Build a
			// fresh, minimal payload instead and swap the whole thing in.
			// Gap fix, second round: stop advertising BEFORE touching the
			// data this time, not after - setAdvertisementData() goes
			// straight to the raw ble_gap_adv_set_data() HCI call with no
			// stopped-first check of its own, and calling it while actively
			// advertising crashed hardware (LoadProhibited) in the first
			// attempt at this fix, which kept the original mutate-then-stop
			// order.
			NimBLEAdvertising *adv = pServer->getAdvertising();
			bool stopOk = adv->stop();
			NimBLEAdvertisementData burstData;
			bool setOk = burstData.setManufacturerData(std::vector<uint8_t>{0x46, 0x00, 0xe7, 0x12, 0x97, 0x30, 0x35, 0xf2, 0x78, 0xff, 0xff, 0xff, 0x30, 0x43, 0x52, 0x4b, 0x54, 0x4d});
			bool applyOk = adv->setAdvertisementData(burstData);
			bool startOk = adv->start();
			ESP_LOGI(TAG, "powerAdvertData1: stop=%s setManufacturerData=%s applyOk=%s start=%s", stopOk ? "OK" : "FAILED", setOk ? "OK" : "FAILED", applyOk ? "OK" : "FAILED", startOk ? "OK" : "FAILED");
			this->set_timeout("ble_mi_remote_power_advert", _power_advert_delay, [this]() { this->powerAdvertData2(); });
		}

		void BleMiRemote::powerAdvertData2() {
			NimBLEAdvertising *adv = pServer->getAdvertising();
			bool stopOk = adv->stop();
			NimBLEAdvertisementData burstData;
			bool setOk = burstData.setManufacturerData(std::vector<uint8_t>{0x46, 0x00});
			bool applyOk = adv->setAdvertisementData(burstData);
			bool startOk = adv->start();
			ESP_LOGI(TAG, "powerAdvertData2: stop=%s setManufacturerData=%s applyOk=%s start=%s cycle=%d", stopOk ? "OK" : "FAILED", setOk ? "OK" : "FAILED", applyOk ? "OK" : "FAILED", startOk ? "OK" : "FAILED", _power_advert_cycle);
			if (_power_advert_cycle > 3) {
				this->powerAdvertStop();
			} else {
				_power_advert_cycle++;
				this->set_timeout("ble_mi_remote_power_advert", _power_advert_delay, [this]() { this->powerAdvertData1(); });
			}
		}

		void BleMiRemote::powerAdvertStop() {
			// Restore the real advertising payload (HID appearance/service
			// UUID) that burstData temporarily replaced, instead of trying
			// to clear manufacturer data on it in place.
			NimBLEAdvertising *adv = pServer->getAdvertising();
			bool stopOk = adv->stop();
			bool setOk = adv->setAdvertisementData(this->_normal_advert_data);
			bool startOk = adv->start();
			ESP_LOGI(TAG, "powerAdvertStop: stop=%s restoreNormalAdvertData=%s start=%s", stopOk ? "OK" : "FAILED", setOk ? "OK" : "FAILED", startOk ? "OK" : "FAILED");
		}

		// Feature 2026-09-04 - see doc comment on the declaration in the
		// header. Only remembers the latest press; a repeated press while
		// still reconnecting overwrites rather than queues multiple.
		void BleMiRemote::queuePendingCommand(uint8_t key, bool is_special) {
			this->_has_pending_command = true;
			this->_pending_is_special = is_special;
			this->_pending_key = key;
			ESP_LOGI(TAG, "queuePendingCommand: key=%d is_special=%s, (re)starting reconnect", key, is_special ? "true" : "false");
			this->startReconnectAdvert();
		}

		// Manual recovery action - see the doc comment on the declaration in
		// ble_mi_remote.h. Cancel any still-scheduled HD-burst retry first:
		// if one were still pending (mid its own 30s window), it would fire
		// later and call fireDirectedBurst(), silently undoing this and
		// reverting advertising back to directed-only.
		//
		// Also deleteAllBonds() before opening up - matches the pre-bisection
		// esp-idf design (2026-08-25 bond-preservation redesign) that this
		// action was reconstructed from: a human pressing this button is
		// explicit intent to force a totally fresh pairing, so don't rely on
		// a new pairing merely superseding a stale local bond entry on its
		// own - clear it up front instead.
		void BleMiRemote::plainAdvertStart() {
			this->cancel_timeout("ble_mi_remote_reconnect_burst");
			bool deleteOk = NimBLEDevice::deleteAllBonds();
			ESP_LOGI(TAG, "plainAdvertStart: manual override, deleteAllBonds()=%s, forcing plain/discoverable advertising", deleteOk ? "OK" : "FAILED");
			this->startPlainAdvertising();
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

		void BleMiRemote::onConnect(NimBLEServer *pServer, NimBLEConnInfo& connInfo) {
			this->_connected = true;

			// own-commits bisection stage 2/5: peripheral-initiated pairing.
			// Ported from esp-idf branch's onConnect() (4e3dd6e) - request
			// security ourselves on an unbonded connect rather than waiting
			// for the peer, since NimBLEHIDDevice's NOTIFY property isn't
			// ENC-gated so a central is never forced to bond on its own.
			if (!connInfo.isBonded()) {
				int rc = 0;
				bool startOk = NimBLEDevice::startSecurity(connInfo.getConnHandle(), &rc);
				ESP_LOGI(TAG, "onConnect: not bonded, requesting security: startSecurity()=%s rc=%d", startOk ? "OK" : "FAILED", rc);
			}

			// own-commits bisection stage 3/5
			this->learnTargetMac(connInfo.getAddress());

			// Feature 2026-09-04: fire a command that was queued while
			// disconnected (see queuePendingCommand()) now that we're
			// actually reconnected. Gated on isBonded() - only fire for a
			// RESUMED connection to an already-known peer, not a fresh
			// pairing that still needs its own security handshake first
			// (startSecurity() above, async) - NOTIFY isn't ENC-gated, so
			// sending this too early could get silently dropped by the
			// peer the same way the original onConnect()/HID-NOTIFY bug
			// this whole project started from did. The trailing release()
			// below cleans it up either way (also gated on is_connected(),
			// so harmless if nothing fired here).
			// NOT yet verified on real hardware whether isBonded() here is
			// actually early enough to guarantee delivery - needs a real
			// disconnect/reconnect/press test, not just reasoning.
			if (this->_has_pending_command && connInfo.isBonded()) {
				this->_has_pending_command = false;
				ESP_LOGI(TAG, "onConnect: firing queued command key=%d is_special=%s", this->_pending_key, this->_pending_is_special ? "true" : "false");
				if (this->_pending_is_special) {
					this->pressSpecial(this->_pending_key);
				} else {
					this->press(this->_pending_key);
				}
			}

			release();
		}

		// own-commits bisection stage 3/5: ported from esp-idf branch
		// (loadTargetMac/learnTargetMac/startPlainAdvertising/startReconnectAdvert),
		// minus the HD-burst call in startReconnectAdvert (stage 4) - falls back
		// to plain advertising even when bonded, for now.
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

		void BleMiRemote::startPlainAdvertising() {
			NimBLEAdvertising *adv = pServer->getAdvertising();
			adv->setConnectableMode(BLE_GAP_CONN_MODE_UND);
			// own-commits bisection stage 4/5: reset the sticky HD flag now
			// that fireDirectedBurst() below can set it.
			adv->setHighDutyCycleDirected(false);
			bool stopOk = adv->stop();
			bool startOk = adv->start();
			ESP_LOGI(TAG, "startPlainAdvertising: stop=%s start=%s", stopOk ? "OK" : "FAILED", startOk ? "OK" : "FAILED");
		}

		// own-commits bisection stage 4/5: now uses real HD-burst when bonded,
		// ported from esp-idf branch's startReconnectAdvert()/fireDirectedBurst().
		void BleMiRemote::startReconnectAdvert() {
			if (!this->_has_target_mac) {
				ESP_LOGI(TAG, "startReconnectAdvert: no target_mac_address learned yet, falling back to plain advertising");
				this->startPlainAdvertising();
				return;
			}

			NimBLEAddress dirAddr(this->_target_mac, BLE_ADDR_PUBLIC);
			if (!NimBLEDevice::isBonded(dirAddr)) {
				ESP_LOGI(TAG, "startReconnectAdvert: no saved bond for %s, falling back to plain advertising", dirAddr.toString().c_str());
				this->startPlainAdvertising();
				return;
			}

			this->_reconnect_retry_until_ms = millis() + 30000;
			ESP_LOGI(TAG, "startReconnectAdvert: bonded to %s -> starting HD-burst retry (30s window)", dirAddr.toString().c_str());
			this->fireDirectedBurst();
		}

		void BleMiRemote::fireDirectedBurst() {
			NimBLEAdvertising *adv = pServer->getAdvertising();
			NimBLEAddress dirAddr(this->_target_mac, BLE_ADDR_PUBLIC);
			adv->setConnectableMode(BLE_GAP_CONN_MODE_DIR);
			adv->setHighDutyCycleDirected(true);
			bool stopOk = adv->stop();
			// Real bug, found 2026-09-04 on real hardware against the real
			// Mi TV (a bisect-test round had never caught this - it needs
			// an actual disconnect/reconnect cycle to surface, which the
			// queued-command feature's own test finally exercised): NimBLE
			// host logs "E NimBLE: ble_gap_adv_validate rc=3" (BLE_HS_EINVAL)
			// periodically here - per the actual NimBLE source, the most
			// likely cause is ble_gap_adv_validate seeing the PREVIOUS
			// advertising op not yet cleared ("duplicate advertising") -
			// stop()'s host-side call returning OK doesn't guarantee the
			// controller has already settled that state before the very
			// next start() call fires. A real reconnect test with the TV
			// confirmed genuinely reachable failed end to end (two full 30s
			// windows, zero successful connects) while this was recurring.
			// Small settle delay as a first pragmatic mitigation - exact
			// value not verified, just a starting guess.
			this->delay_ms(20);
			bool startOk = adv->start(1280, &dirAddr);
			ESP_LOGI(TAG, "fireDirectedBurst: HD directed burst to %s, stop=%s start=%s, retry_remaining_ms=%d", dirAddr.toString().c_str(), stopOk ? "OK" : "FAILED", startOk ? "OK" : "FAILED", (int) (this->_reconnect_retry_until_ms - millis()));

			this->set_timeout("ble_mi_remote_reconnect_burst", 1300, [this]() {
				if (this->_connected) {
					ESP_LOGI(TAG, "fireDirectedBurst: burst window elapsed, already connected - nothing to do");
					return;
				}

				if ((int32_t) (millis() - this->_reconnect_retry_until_ms) < 0) {
					this->fireDirectedBurst();
					return;
				}

				NimBLEAdvertising *adv2 = pServer->getAdvertising();
				bool stopOk2 = adv2->stop();
				ESP_LOGI(TAG, "fireDirectedBurst: retry window exhausted, stopping advertising (stop=%s) - manual plain_advert needed to resume", stopOk2 ? "OK" : "FAILED");
			});
		}

		void BleMiRemote::onDisconnect(NimBLEServer *pServer, NimBLEConnInfo& connInfo, int reason) {
			this->_connected = false;

			ESP_LOGI(TAG, "Disconnected: %s, reason=0x%02x", connInfo.getAddress().toString().c_str(), reason);

			// own-commits bisection stage 4/5: cancel any still-pending
			// HD-burst retry from a sequence that hadn't finished its own 30s
			// window yet.
			this->cancel_timeout("ble_mi_remote_reconnect_burst");

			if (!this->_reconnect || !this->_should_readvertise) {
				return;
			}

			if (reason == BLE_HS_ERR_HCI_BASE + BLE_ERR_REM_USER_CONN_TERM) {
				bool deleteOk = NimBLEDevice::deleteBond(connInfo.getAddress());
				ESP_LOGI(TAG, "Disconnected: peer deliberately terminated the link (reason=0x%02x), deleteBond(%s)=%s - plain advertising only", reason, connInfo.getAddress().toString().c_str(), deleteOk ? "OK" : "FAILED");
				this->startPlainAdvertising();
				return;
			}

			this->startReconnectAdvert();
		}

		void BleMiRemote::onWrite(NimBLECharacteristic *me, NimBLEConnInfo& connInfo) {
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
