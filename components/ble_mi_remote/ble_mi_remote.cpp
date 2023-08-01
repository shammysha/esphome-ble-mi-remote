
#ifdef USE_ESP32

#define CONFIG_BT_NIMBLE_DEBUG
#define CONFIG_NIMBLE_CPP_LOG_LEVEL 4
#define CONFIG_BT_NIMBLE_LOG_LEVEL 0

#include "ble_mi_remote.h"
#include <NimBLEDevice.h>
#include <NimBLEService.h>
#include <NimBLEUtils.h>
#include <NimBLEAdvertising.h>
#include "HIDTypes.h"
#include "HIDKeyboardTypes.h"
#include <driver/adc.h>
#include "sdkconfig.h"
#include <string>
#include <list>
#include <signal.h>


#define CONSUMER_ID 0x01
#define KEYBOARD_ID 0x02

static const std::string sUUID_6287 = "00006287-3c17-d293-8e48-14fe2e4da212";
static const std::string sUUID_D1FF = "0000d1ff-3c17-d293-8e48-14fe2e4da212";
static const std::string sUUID_D0FF = "0000d0ff-3c17-d293-8e48-14fe2e4da212";

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

			NimBLEDevice::init (deviceName);
			pServer = NimBLEDevice::createServer();
			pServer->setCallbacks(this);

			hid = new NimBLEHIDDevice(pServer);
			inputSpecialKeys = hid->inputReport(CONSUMER_ID);
			inputSpecialKeys->setCallbacks(this);
			inputKeyboard = hid->inputReport(KEYBOARD_ID);
			inputKeyboard->setCallbacks(this);
			outputKeyboard = hid->outputReport(KEYBOARD_ID);
			outputKeyboard->setCallbacks(this);

			vendorReport_06 = hid->inputReport(0x06);
			vendorReport_06->setCallbacks(this);
			vendorReport_07 = hid->inputReport(0x07);
			vendorReport_07->setCallbacks(this);
			vendorReport_08 = hid->inputReport(0x08);
			vendorReport_08->setCallbacks(this);

//			vendorServicesSetup();

			hid->manufacturer()->setValue(deviceManufacturer);
			hid->pnp(sid, vid, pid, version);
			hid->hidInfo(0x00, 0x01);

			NimBLEDevice::setSecurityAuth(true, true, true);

			hid->reportMap((uint8_t*) _hidReportDescriptor, sizeof(_hidReportDescriptor));

			hid->startServices();

			onStarted(pServer);

			powerAdvData = NULL;

			advertisingStart();

			hid->setBatteryLevel(batteryLevel);

			pServer = NimBLEDevice::getServer();

			pServer->advertiseOnDisconnect(this->_reconnect);
			release();
		}

		void BleMiRemote::advertisingStart() {
			advertising = pServer->getAdvertising();
			advertising->reset();
			advertising->setAppearance(HID_KEYBOARD);
			advertising->addServiceUUID(hid->hidService()->getUUID());
//			advertising->addServiceUUID( sVendor_6287->getUUID() );
//			advertising->addServiceUUID( sVendor_d1ff->getUUID() );
//			advertising->addServiceUUID( sVendor_d0ff->getUUID() );
			advertising->setScanResponse(false);

			if (!this->is_connected()) {
				advertising->start();

				ESP_LOGD(TAG, "Advertising started!");
			}
		}

		void BleMiRemote::advertisingStop() {
			pServer->stopAdvertising();

			ESP_LOGD(TAG, "Advertising stopped!");
		}

		void BleMiRemote::powerAdvertisingStart() {
			if (powerAdvData) {
				delete powerAdvData;
				powerAdvData = NULL;
			}

			powerAdvData = new NimBLEAdvertisementData();
			powerAdvData->setFlags(1);

			char mfgData[] = { 0x00, 0x01 };
			powerAdvData->setManufacturerData(std::string(mfgData, 2));

			powerAdvData->setShortName("MI RC");
			powerAdvData->setPartialServices((NimBLEUUID) "1812");

			char codData[] = { 0x04, 0x0d, 0x04, 0x05, 0x00 };
			powerAdvData->addData((char*) codData, sizeof(codData));

			powerAdvData->addTxPower();

			char custData[] = { 0x04, 0xfe, 0xee, 0x68, 0xc4 };
			powerAdvData->addData((char*) custData, sizeof(custData));

			advertising = pServer->getAdvertising();
			advertising->reset();
			advertising->setAdvertisementData(*powerAdvData);
			advertising->setAdvertisementType(0);
			advertising->start(3, BleMiRemote::callbHandler);
			advertising->setScanResponse(false);

			ESP_LOGD(TAG, "Power advertise started");
		}

		void BleMiRemote::powerAdvertisingStop(NimBLEAdvertising *pAdv) {
			ESP_LOGD(TAG, "Power advertise stopped");

			advertisingStart();
		}

		void BleMiRemote::stop() {
			if (_reconnect) {
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
			} else {
				if (k == 5) {
					powerAdvertisingStart();
				}
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

		void BleMiRemote::onConnect(NimBLEServer *pServer) {
			this->_connected = true;
			NimBLEConnInfo peer = pServer->getPeerInfo(0);

			release();
		}

		void BleMiRemote::onDisconnect(NimBLEServer *pServer) {
			this->_connected = false;
		}

		void BleMiRemote::onConnect(NimBLEServer* pServer, ble_gap_conn_desc* desc) {
			ESP_LOGD(TAG, "Client address: %s", NimBLEAddress(desc->peer_ota_addr).toString().c_str());
			ESP_LOGD(TAG, "Client id: %s", NimBLEAddress(desc->peer_id_addr).toString().c_str());
			ESP_LOGD(TAG, "Connection handle: %s", NimBLEAddress(desc->conn_handle).toString().c_str());
			/** We can use the connection handle here to ask for different connection parameters.
			 *  Args: connection handle, min connection interval, max connection interval
			 *  latency, supervision timeout.
			 *  Units; Min/Max Intervals: 1.25 millisecond increments.
			 *  Latency: number of intervals allowed to skip.
			 *  Timeout: 10 millisecond increments, try for 5x interval time for best results.
			 */


		}

		void BleMiRemote::onRead(NimBLECharacteristic* pCharacteristic){
			ESP_LOGD(TAG,"Chr %s: onRead(), value: %s", pCharacteristic->getUUID().toString().c_str(), str2hex(pCharacteristic->getValue()).c_str());
		}

    	void BleMiRemote::onWrite(NimBLECharacteristic* pCharacteristic) {
			ESP_LOGD(TAG,"Chr %s onWrite(), value: %s", pCharacteristic->getUUID().toString().c_str(), str2hex(pCharacteristic->getValue()).c_str());
    	}

    	void BleMiRemote::onWrite(NimBLECharacteristic* pCharacteristic, ble_gap_conn_desc* desc) {
			ESP_LOGD(TAG,"Chr %s onWrite(), addr: %s, value: %s", pCharacteristic->getUUID().toString().c_str(), std::string(NimBLEAddress(desc->peer_ota_addr)).c_str(), str2hex(pCharacteristic->getValue()).c_str());
    	}

    	void BleMiRemote::onNotify(NimBLECharacteristic* pCharacteristic) {
    		ESP_LOGD(TAG, "Chr %s onNotify(), value: %s'", pCharacteristic->getUUID().toString().c_str(), str2hex(pCharacteristic->getValue()).c_str());
    	}

    	void BleMiRemote::onStatus(NimBLECharacteristic* pCharacteristic, Status status, int code) {
			String str = ("Notification/Indication status code: ");
			str += status;
			str += ", return code: ";
			str += code;
			str += ", ";
			str += NimBLEUtils::returnCodeToString(code);
			ESP_LOGD(TAG, "Status: %s", str.c_str());
		}

		void BleMiRemote::onSubscribe(NimBLECharacteristic* pCharacteristic, ble_gap_conn_desc* desc, uint16_t subValue) {
			String str = "Client ID: ";
			str += desc->conn_handle;
			str += " Address: ";
			str += std::string(NimBLEAddress(desc->peer_ota_addr)).c_str();
			if(subValue == 0) {
				str += " Unsubscribed to ";
			}else if(subValue == 1) {
				str += " Subscribed to notfications for ";
			} else if(subValue == 2) {
				str += " Subscribed to indications for ";
			} else if(subValue == 3) {
				str += " Subscribed to notifications and indications for ";
			}
			str += std::string(pCharacteristic->getUUID()).c_str();

			ESP_LOGD(TAG, "onSubscribe: %s", str.c_str());
		}

		void BleMiRemote::onWrite(NimBLEDescriptor* pDescriptor) {
			ESP_LOGD(TAG,"Descriptor witten value: %s", str2hex(pDescriptor->getValue()).c_str());
		}

		void BleMiRemote::onRead(NimBLEDescriptor* pDescriptor) {
			ESP_LOGD(TAG, "Descriptor %s read value", pDescriptor->getUUID().toString().c_str());
		}

		void BleMiRemote::callbHandler(NimBLEAdvertising *a) {
            ((BleMiRemote*)a)->powerAdvertisingStop(a);
        }

		void BleMiRemote::vendorServicesSetup() {
//			NimBLEService* sVendor_6287 = pServer->createService("00006287-3c17-d293-8e48-14fe2e4da212");
//			NimBLECharacteristic* cVendor_6287_6487 = sVendor_6287->createCharacteristic("00006487-3c17-d293-8e48-14fe2e4da212", NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::NOTIFY);
//			NimBLECharacteristic* cVendor_6287_6387 = sVendor_6287->createCharacteristic("00006387-3c17-d293-8e48-14fe2e4da212", NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::NOTIFY);
//			NimBLEDescriptor* dVendor_6287_6487_2902 = cVendor_6287_6487->createDescriptor("00002902-0000-1000-8000-00805f9b34fb");
//
//			NimBLEService* sVendor_d1ff = pServer->createService("0000d1ff-3c17-d293-8e48-14fe2e4da212");
//			NimBLECharacteristic* cVendor_d1ff_a001 = sVendor_d1ff->createCharacteristic("0000a001-0000-1000-8000-00805f9b34fb", NIMBLE_PROPERTY::WRITE_NR | NIMBLE_PROPERTY::NOTIFY);
//			NimBLEDescriptor* dVendor_d1ff_a001_2902 = cVendor_d1ff_a001->createDescriptor("00002902-0000-1000-8000-00805f9b34fb");
//
//			NimBLEService* sVendor_d0ff = pServer->createService("0000d0ff-3c17-d293-8e48-14fe2e4da212");
//			NimBLECharacteristic* cVendor_d0ff_fff2 = sVendor_d0ff->createCharacteristic("0000fff2-0000-1000-8000-00805f9b34fb", NIMBLE_PROPERTY::WRITE);
//			NimBLECharacteristic* cVendor_d0ff_fff1 = sVendor_d0ff->createCharacteristic("0000fff1-0000-1000-8000-00805f9b34fb", NIMBLE_PROPERTY::WRITE);
//			NimBLECharacteristic* cVendor_d0ff_ffd8 = sVendor_d0ff->createCharacteristic("0000ffd8-0000-1000-8000-00805f9b34fb", NIMBLE_PROPERTY::WRITE_NR);
//			NimBLECharacteristic* cVendor_d0ff_ffd5 = sVendor_d0ff->createCharacteristic("0000ffd5-0000-1000-8000-00805f9b34fb", NIMBLE_PROPERTY::READ);
//			NimBLECharacteristic* cVendor_d0ff_ffd4 = sVendor_d0ff->createCharacteristic("0000ffd4-0000-1000-8000-00805f9b34fb", NIMBLE_PROPERTY::READ);
//			NimBLECharacteristic* cVendor_d0ff_ffd3 = sVendor_d0ff->createCharacteristic("0000ffd3-0000-1000-8000-00805f9b34fb", NIMBLE_PROPERTY::READ);
//			NimBLECharacteristic* cVendor_d0ff_ffd2 = sVendor_d0ff->createCharacteristic("0000ffd2-0000-1000-8000-00805f9b34fb", NIMBLE_PROPERTY::READ);
//			NimBLECharacteristic* cVendor_d0ff_ffd1 = sVendor_d0ff->createCharacteristic("0000ffd1-0000-1000-8000-00805f9b34fb", NIMBLE_PROPERTY::WRITE_NR);
//
//			cVendor_6287_6487->setCallbacks(this);
//			cVendor_6287_6387->setCallbacks(this);
//			dVendor_6287_6487_2902->setCallbacks(this);

			sVendor_6287 = pServer->createService((NimBLEUUID) sUUID_6287);

				NimBLECharacteristic* cVendor_6287_6487 = sVendor_6287->createCharacteristic("00006487-3c17-d293-8e48-14fe2e4da212", NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::NOTIFY);
				cVendor_6287_6487->setCallbacks(this);

//					NimBLEDescriptor* dVendor_6287_6487_2902 = cVendor_6287_6487->createDescriptor("2902");
//					uint8_t dVendor_6287_6487_2902_data[] = { 0x00, 0x00 };
//					dVendor_6287_6487_2902->setValue( (uint8_t*) dVendor_6287_6487_2902_data, sizeof(dVendor_6287_6487_2902_data) );
//					dVendor_6287_6487_2902->setCallbacks(this);

				NimBLECharacteristic* cVendor_6287_6387 = sVendor_6287->createCharacteristic("00006387-3c17-d293-8e48-14fe2e4da212", NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::NOTIFY);
				cVendor_6287_6387->setCallbacks(this);

			sVendor_d1ff = pServer->createService((NimBLEUUID) sUUID_D1FF);

				NimBLECharacteristic* cVendor_d1ff_a001 = sVendor_d1ff->createCharacteristic("0000a001-0000-1000-8000-00805f9b34fb", NIMBLE_PROPERTY::WRITE_NR | NIMBLE_PROPERTY::NOTIFY);
				cVendor_d1ff_a001->setCallbacks(this);

//					NimBLEDescriptor* dVendor_d1ff_a001_2902 = cVendor_d1ff_a001->createDescriptor("2902");
//					uint8_t dVendor_d1ff_a001_2902_data[] = { 0x00, 0x00 };
//					dVendor_d1ff_a001_2902->setValue( (uint8_t*) dVendor_d1ff_a001_2902_data, sizeof(dVendor_d1ff_a001_2902_data) );
//					dVendor_d1ff_a001_2902->setCallbacks(this);

			sVendor_d0ff = pServer->createService((NimBLEUUID) sUUID_D0FF);

				NimBLECharacteristic* cVendor_d0ff_fff2 = sVendor_d0ff->createCharacteristic("0000fff2-0000-1000-8000-00805f9b34fb", NIMBLE_PROPERTY::WRITE);
				cVendor_d0ff_fff2->setCallbacks(this);

				NimBLECharacteristic* cVendor_d0ff_fff1 = sVendor_d0ff->createCharacteristic("0000fff1-0000-1000-8000-00805f9b34fb", NIMBLE_PROPERTY::WRITE);
				cVendor_d0ff_fff1->setValue(0x01);
				cVendor_d0ff_fff1->setCallbacks(this);

				NimBLECharacteristic* cVendor_d0ff_ffd8 = sVendor_d0ff->createCharacteristic("0000ffd8-0000-1000-8000-00805f9b34fb", NIMBLE_PROPERTY::WRITE_NR);
				cVendor_d0ff_ffd8->setCallbacks(this);

				NimBLECharacteristic* cVendor_d0ff_ffd5 = sVendor_d0ff->createCharacteristic("0000ffd5-0000-1000-8000-00805f9b34fb", NIMBLE_PROPERTY::READ);
				uint8_t cVendor_d0ff_ffd5_data[] = { 0x00, 0x00 };
				cVendor_d0ff_ffd5->setValue( (uint8_t*) cVendor_d0ff_ffd5_data, sizeof(cVendor_d0ff_ffd5_data) );
				cVendor_d0ff_ffd5->setCallbacks(this);

				NimBLECharacteristic* cVendor_d0ff_ffd4 = sVendor_d0ff->createCharacteristic("0000ffd4-0000-1000-8000-00805f9b34fb", NIMBLE_PROPERTY::READ);
				uint8_t cVendor_d0ff_ffd4_data[] = { 0x14, 0x20 };
				cVendor_d0ff_ffd4->setValue( (uint8_t*) cVendor_d0ff_ffd4_data, sizeof(cVendor_d0ff_ffd4_data) );
				cVendor_d0ff_ffd4->setCallbacks(this);

				NimBLECharacteristic* cVendor_d0ff_ffd3 = sVendor_d0ff->createCharacteristic("0000ffd3-0000-1000-8000-00805f9b34fb", NIMBLE_PROPERTY::READ);
				uint8_t cVendor_d0ff_ffd3_data[] = { 0xd7, 0x4b };
				cVendor_d0ff_ffd3->setValue( (uint8_t*) cVendor_d0ff_ffd3_data, sizeof(cVendor_d0ff_ffd3_data) );
				cVendor_d0ff_ffd3->setCallbacks(this);

				NimBLECharacteristic* cVendor_d0ff_ffd2 = sVendor_d0ff->createCharacteristic("0000ffd2-0000-1000-8000-00805f9b34fb", NIMBLE_PROPERTY::READ);
				uint8_t cVendor_d0ff_ffd2_data[] = { 0x18, 0x46, 0x44, 0xc1, 0x4a, 0xab };
				cVendor_d0ff_ffd3->setValue( (uint8_t*) cVendor_d0ff_ffd2_data, sizeof(cVendor_d0ff_ffd2_data) );
				cVendor_d0ff_ffd2->setCallbacks(this);

				NimBLECharacteristic* cVendor_d0ff_ffd1 = sVendor_d0ff->createCharacteristic("0000ffd1-0000-1000-8000-00805f9b34fb", NIMBLE_PROPERTY::WRITE_NR);
				cVendor_d0ff_ffd1->setCallbacks(this);
		}

		std::string BleMiRemote::str2hex(std::string val) {
			std::vector<uint8_t> myVector(val.begin(), val.end());

			return format_hex_pretty(myVector);
		}

	}  // namespace ble_mi_remote
}  // namespace esphome


#endif
