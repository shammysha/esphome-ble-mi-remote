#pragma once

#ifdef USE_ESP32

#include "esphome/core/component.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include <NimBLEServer.h>
#include "BleMiRemote.h"
#include <string>

namespace esphome {
	namespace ble_mi_remote {
		class BleMiRemote : public PollingComponent {
			public:
				BleMiRemote(std::string name, std::string manufacturer_id, uint8_t battery_level, bool reconnect)
				: PollingComponent(1000), BleMiRemote(name, manufacturer_id, battery_level) {
					reconnect_ = reconnect;
				}

				void setup() override;
				void update() override;

				float get_setup_priority() const override { return setup_priority::AFTER_BLUETOOTH; }

				void set_delay(uint32_t delay_ms = 8) { BleMiRemote.setDelay(delay_ms); };
				void set_release_delay(uint32_t delay_ms = 8) { release_delay_ = delay_ms; };
				void set_battery_level(uint8_t level = 100) { BleMiRemote.setBatteryLevel(level); };

				void set_state_sensor(binary_sensor::BinarySensor *state_sensor) { state_sensor_ = state_sensor; }

				void press(uint8_t key, bool with_timer = true);
				void pressSpecial(uint8_t key, bool with_timer = true);
				void release(uint8_t key);
				void releaseSpecial(uint8_t key);
				void releaseAll() { ;

				void start();
				void stop();

			protected:
				binary_sensor::BinarySensor *state_sensor_;

			private:
				bool is_connected();
				void update_timer();

				BLEServer *pServer;
				BleMiRemote BleMiRemote;

				bool reconnect_{true};
				uint32_t default_delay_{100};
				uint32_t release_delay_{8};
		};
	}  // namespace ble_mi_remote
}  // namespace esphome

#endif
