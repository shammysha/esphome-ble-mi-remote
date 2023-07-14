
#ifdef USE_ESP32

#include "ble_mi_remote.h"
#include "esphome/core/log.h"
#include <NimBLEServer.h>
#include <NimBLEDevice.h>
#include <NimBLEService.h>
#include <NimBLECharacteristic.h>
#include <NimBLEAdvertising.h>
#include <string>
#include <list>

namespace esphome {
	namespace ble_mi_remote {
		static const char *const TAG = "ble_mi_remote";

		void BleMiRemote::setup() {
			ESP_LOGI(TAG, "Setting up...");

			BleMiRemote.begin();

			pServer = BLEDevice::getServer();

			pServer->advertiseOnDisconnect(this->reconnect_);

			BleMiRemote.releaseAll();
		}

		void BleMiRemote::stop() {
			if (this->reconnect_) {
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
			if (this->reconnect_) {
				pServer->advertiseOnDisconnect(true);
			}

			pServer->startAdvertising();
		}

		void BleMiRemote::update() { state_sensor_->publish_state(BleMiRemote.isConnected()); }

		bool BleMiRemote::is_connected() {
			if (!BleMiRemote.isConnected()) {
				ESP_LOGI(TAG, "Disconnected");

				return false;
			}

			return true;
		}

		void BleMiRemote::update_timer() {
			this->cancel_timeout((const std::string) TAG);
			this->set_timeout((const std::string) TAG, release_delay_, [this]() { this->release(); });
		}

		void BleMiRemote::press(uint8_t key, bool with_timer) {
			if (this->is_connected()) {
				if (with_timer) {
					this->update_timer();
				}

				BleMiRemote.press(key);
			}
		}

		void BleMiRemote::pressSpecial(uint8_t key, bool with_timer) {
			if (this->is_connected()) {
				if (with_timer) {
					this->update_timer();
				}
				BleMiRemote.pressSpecial(key);
			}
		}

		void BleMiRemote::release() {
			if (this->is_connected()) {
				this->cancel_timeout((const std::string) TAG);
				BleMiRemote.releaseAll();
			}
		}

	}  // namespace ble_mi_remote
}  // namespace esphome

#endif
