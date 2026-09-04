#pragma once

#ifdef USE_ESP32

#include "esphome/core/component.h"
#include "esphome/core/preferences.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "sdkconfig.h"
#include <NimBLEServer.h>
#include "NimBLECharacteristic.h"
#include "NimBLEHIDDevice.h"
// Explicit, not relying on a transitive include (order varies per
// translation unit) - needed for NimBLEAdvertisementData below.
#include "NimBLEAdvertising.h"
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
		class BleMiRemote : public PollingComponent, public NimBLEServerCallbacks, public NimBLECharacteristicCallbacks {
			public:
				BleMiRemote(std::string name, std::string manufacturer_id, uint8_t battery_level = 100, bool reconnect = true);

				void setup() override;
				void update() override;

				float get_setup_priority() const override { return setup_priority::AFTER_BLUETOOTH; }

				void set_delay(uint32_t delay_ms = 8) { this->_delay_ms = delay_ms; };
				void set_release_delay(uint32_t delay_ms = 8) { _release_delay = delay_ms; };
				void set_battery_level(uint8_t level = 100);

				void set_state_sensor(binary_sensor::BinarySensor *state_sensor) { state_sensor_ = state_sensor; }
				// Feature 2026-09-04: expose our own BLE MAC (diagnostic) -
				// needed it directly ourselves this session to correlate
				// sniffer captures, no reason not to surface it in HA too.
				void set_mac_address_sensor(text_sensor::TextSensor *mac_address_sensor) { mac_address_sensor_ = mac_address_sensor; }

				void press(uint8_t key, bool with_timer = true);
				void pressSpecial(uint8_t key, bool with_timer = true);
				void release();

				void start();
				void stop();

				void begin(void);
				void end(void);
				void sendReport(KeyReport* keys);
				void sendReport(SpecialKeyReport* keys);

				// Gap fix, functional (not stage 5): real Component lifecycle
				// hooks, fire automatically on any reboot (incl. OTA) - not
				// button-gated, not diagnostics. Both just call stop() on
				// esp-idf, for a clean disconnect/advertising-stop before the
				// actual reboot instead of relying on the hardware reset alone.
				void on_shutdown() override;
				void on_safe_shutdown() override;

				// own-commits bisection: connectWakeStart, ported for full
				// structural parity with esp-idf (per user request, even
				// though it's only ever invoked via its own button action -
				// dead code otherwise, can't affect a normal pairing test).
				void connectWakeStart();
				// own-commits bisection: powerAdvertStart/Stop, ditto - reached
				// via pressSpecial(SPECIAL_POWER) while disconnected, which the
				// auto-generated "Power" special-key button already calls.
				void powerAdvertStart();
				void powerAdvertStop();

				// Manual recovery action (2026-09-03): startReconnectAdvert()/
				// fireDirectedBurst() deliberately never fall back from
				// HD-directed-only retry to plain/discoverable on their own -
				// that protects a still-live bond from being evicted by a
				// stray pairing just because the real peer is briefly
				// unreachable (off/rebooting). But it also means a bond that
				// went stale WITHOUT a clean reason=0x213 disconnect (e.g. the
				// peer forgot/unpaired us while we were powered off) has no
				// automatic way back to discoverability. Only a human has the
				// missing context (genuinely gone vs. just asleep) to make
				// that call safely, so this exposes it as an explicit action
				// instead of guessing from elapsed time. Reconstructed from
				// (and matches) the pre-bisection esp-idf design's own manual
				// plain_advert action, deleteAllBonds() included.
				void plainAdvertStart();

			protected:
				binary_sensor::BinarySensor *state_sensor_;
				text_sensor::TextSensor *mac_address_sensor_;

			private:
				bool is_connected();
				void update_timer();
				void delay_ms(uint64_t ms);
				// own-commits bisection: powerAdvertData1/2 (internal, timer-chained)
				void powerAdvertData1();
				void powerAdvertData2();

				// own-commits bisection stage 3/5: bond-preservation/reconnect
				// dispatch, ported from esp-idf branch minus HD-burst (stage 4).
				void loadTargetMac();
				void learnTargetMac(NimBLEAddress addr);
				void startPlainAdvertising();
				void startReconnectAdvert();
				// own-commits bisection stage 4/5
				void fireDirectedBurst();

				// Feature 2026-09-04: any button press while disconnected
				// used to silently do nothing (except SPECIAL_POWER, which
				// stays separate - that's a wake-an-OFF-TV attempt, not a
				// reconnect, and HD-burst can't reach a TV whose BLE radio
				// is off anyway). Matches the industry-standard BLE HID
				// remote pattern found via research: a key press is the
				// "external event" that (re-)starts the reconnect-
				// advertising cycle, not a timer or network check. Queues
				// the command and re-arms startReconnectAdvert(); onConnect()
				// fires it once actually reconnected. Only the latest
				// press wins if pressed repeatedly while still reconnecting.
				void queuePendingCommand(uint8_t key, bool is_special);

				NimBLEServer 			*pServer;
				NimBLEHIDDevice*		hid;
				NimBLECharacteristic*	inputKeyboard;
				NimBLECharacteristic*	outputKeyboard;
				NimBLECharacteristic*	inputSpecialKeys;
				NimBLECharacteristic*	vendorReport_06;
				NimBLECharacteristic*	vendorReport_07;
				NimBLECharacteristic*	vendorReport_08;
				NimBLEAdvertising*		advertising;

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

				// own-commits bisection stage 3/5
				bool				_should_readvertise{true};
				uint64_t			_target_mac = 0;
				bool				_has_target_mac = false;
				bool				_target_mac_from_config = false;
				ESPPreferenceObject	_target_mac_pref;
				// own-commits bisection stage 4/5
				uint32_t			_reconnect_retry_until_ms = 0;
				// own-commits bisection: powerAdvert*
				uint32_t			_power_advert_delay = 1000;
				uint8_t				_power_advert_cycle = 0;
				// Gap fix 2026-09-03: the normal advertising payload (flags +
				// appearance + HID service UUID, set up once in setup()) sits
				// at the legacy 31-byte cap with zero bytes to spare - real,
				// measured on hardware (advData=31, scanData=31, budget 31),
				// not a guess. setManufacturerData() on the live advertising
				// object was therefore FAILING on every single call, even an
				// empty one, so the whole powerAdvertStart() burst has never
				// actually transmitted anything. Fix: swap in a fresh,
				// minimal NimBLEAdvertisementData (just the manufacturer
				// data) for the duration of the burst, saved here once in
				// setup() so powerAdvertStop() can restore the real one.
				NimBLEAdvertisementData _normal_advert_data;

				// Feature 2026-09-04: queuePendingCommand() state - see its
				// doc comment above.
				bool				_has_pending_command = false;
				bool				_pending_is_special = false;
				uint8_t				_pending_key = 0;


				uint16_t sid		= 0x01;
				uint16_t vid		= 0x2717;
				uint16_t pid		= 0x32b9;
				uint16_t version	= 0x4a4f;

			protected:
				virtual void onStarted(NimBLEServer *pServer) { };
				// nimble-cpp-bisect: tag 2.0.0 added NimBLEConnInfo& to every
				// callback (and a reason code to onDisconnect) - [Breaking]
				// Update callbacks to use NimBLEConnInfo (ba79a1b).
				virtual void onConnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo) override;
				virtual void onDisconnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo, int reason) override;
				virtual void onWrite(NimBLECharacteristic* me, NimBLEConnInfo& connInfo) override;
		};
	}  // namespace ble_mi_remote
}  // namespace esphome

#endif
