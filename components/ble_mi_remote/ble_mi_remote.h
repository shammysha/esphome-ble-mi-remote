#pragma once

#ifdef USE_ESP32

#include "esphome/core/component.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/core/log.h"
#include "sdkconfig.h"
#include <NimBLEServer.h>
#include "NimBLECharacteristic.h"
#include "NimBLEHIDDevice.h"
#include <string>

const uint8_t SPECIAL_KEYS_COUNT = 24;

const uint8_t SPECIAL_MENU_PICK = 0;
const uint8_t SPECIAL_MENU_UP = 1;
const uint8_t SPECIAL_MENU_DOWN = 2;
const uint8_t SPECIAL_MENU_LEFT = 3;
const uint8_t SPECIAL_MENU_RIGHT = 4;
const uint8_t SPECIAL_POWER = 5;
const uint8_t SPECIAL_VOICE = 6;
const uint8_t SPECIAL_VOLUME_UP = 7;
const uint8_t SPECIAL_VOLUME_DOWN = 8;
const uint8_t SPECIAL_TASK_SELECT = 9;
const uint8_t SPECIAL_MOVIE_BROWSER = 10;
const uint8_t SPECIAL_BUTTON_GREEN = 11;
const uint8_t SPECIAL_BUTTON_BLUE = 12;
const uint8_t SPECIAL_MI_HOME = 13;
const uint8_t SPECIAL_EMAIL_READER = 14;
const uint8_t SPECIAL_CALCULATOR = 15;
const uint8_t SPECIAL_LOCAL_BROWSER = 16;
const uint8_t SPECIAL_VOICE2 = 17;
const uint8_t SPECIAL_ANDROID_HOME = 18;
const uint8_t SPECIAL_BACK = 19;
const uint8_t SPECIAL_FORWARD = 20;
const uint8_t SPECIAL_STOP = 21;
const uint8_t SPECIAL_REFRESH = 22;
const uint8_t SPECIAL_BOOKMARK = 23;

//  Low level key report: up to 6 keys and shift, ctrl etc at once
typedef struct {
  uint8_t modifiers;
  uint8_t reserved;
  uint8_t keys[6];
} KeyReport;

typedef struct {
	uint8_t keys[3];
} SpecialKeyReport;

namespace esphome {
	namespace ble_mi_remote {
		class BleMiRemote : public PollingComponent, public NimBLEServerCallbacks, public NimBLECharacteristicCallbacks, public NimBLEDescriptorCallbacks, public NimBLEAdvertising {
			public:
				BleMiRemote(std::string name, std::string manufacturer_id, uint8_t battery_level = 100, bool reconnect = true);

				void setup() override;
				void update() override;

				float get_setup_priority() const override { return setup_priority::AFTER_BLUETOOTH; }

				void set_delay(uint32_t delay_ms = 8) { this->_delay_ms = delay_ms; };
				void set_release_delay(uint32_t delay_ms = 8) { _release_delay = delay_ms; };
				void set_battery_level(uint8_t level = 100);

				void set_state_sensor(binary_sensor::BinarySensor *state_sensor) { state_sensor_ = state_sensor; }

				void press(uint8_t key, bool with_timer = true);
				void pressSpecial(uint8_t key, bool with_timer = true);
				void release();

				void start();
				void stop();

				void begin(void);
				void end(void);
				void sendReport(KeyReport* keys);
				void sendReport(SpecialKeyReport* keys);

				void powerAdvertisingSetup();
				void powerAdvertisingStart();
				void powerAdvertisingStop(NimBLEAdvertising *pAdv);
				void (*advCallback)(NimBLEAdvertising *pAdv);

			protected:
				binary_sensor::BinarySensor *state_sensor_;

			private:
				bool is_connected();
				void update_timer();
				void delay_ms(uint64_t ms);

				void vendorServicesSetup();

				NimBLEServer* 				pServer;
				NimBLEHIDDevice*			hid;
				NimBLECharacteristic*		inputKeyboard;
				NimBLECharacteristic*		outputKeyboard;
				NimBLECharacteristic*		inputSpecialKeys;
				NimBLECharacteristic*		vendorReport_06;
				NimBLECharacteristic*		vendorReport_07;
				NimBLECharacteristic*		vendorReport_08;
				NimBLEService*				sVendor_6287;
				NimBLEService*				sVendor_d1ff;
				NimBLEService*				sVendor_d0ff;

				NimBLEAdvertisementData* 	powerAdvData;
				NimBLEAdvertising*			advertising;
				NimBLEAdvertising*			powerAdvertising;



				bool 				_reconnect{true};
				uint32_t 			_default_delay{100};
				uint32_t 			_release_delay{8};
				KeyReport			_keyReport;
				SpecialKeyReport	_specialKeyReport;
				std::string			deviceName;
				std::string			deviceManufacturer;
				uint8_t				batteryLevel;
				bool				_connected = false;
				uint32_t			_delay_ms = 7;

				uint16_t sid		= 0x01;
				uint16_t vid		= 0x2717;
				uint16_t pid		= 0x32b9;
				uint16_t version	= 0x4a4f;

			protected:
				void onStarted(NimBLEServer *pServer) { };
				void onConnect(NimBLEServer* pServer);
				void onConnect(NimBLEServer* pServer, ble_gap_conn_desc* desc);
				void onDisconnect(NimBLEServer* pServer);

				void onWrite(NimBLECharacteristic* pCharacteristic);
			    void onRead(NimBLECharacteristic* pCharacteristic);
			    void onNotify(NimBLECharacteristic* pCharacteristic);
			    void onStatus(NimBLECharacteristic* pCharacteristic, Status status, int code);
			    void onSubscribe(NimBLECharacteristic* pCharacteristic, ble_gap_conn_desc* desc, uint16_t subValue);

				void onWrite(NimBLEDescriptor* pDescriptor);
			    void onRead(NimBLEDescriptor* pDescriptor);

		};
	}  // namespace ble_mi_remote
}  // namespace esphome


static void advCallback(NimBLEAdvertising *pAdv) {
	esphome::ble_mi_remote::BleMiRemote m;
	m->powerAdvertisingStop(*pAdv);
}

#endif
