
#ifdef USE_ESP32

#include "mi_remote.h"
#include "esphome/core/log.h"
#include <NimBLEServer.h>
#include <NimBLEDevice.h>
#include <NimBLEService.h>
#include <NimBLECharacteristic.h>
#include <NimBLEAdvertising.h>
#include <string>
#include <list>

namespace esphome {
	namespace mi_remote {
		static const char *const TAG = "mi_remote";

		void MiRemote::setup() {
			ESP_LOGI(TAG, "Setting up...");

			BLEMiRemote.begin();

			pServer = BLEDevice::getServer();

			pServer->advertiseOnDisconnect(this->reconnect_);

			BLEMiRemote.releaseAll();
		}

		void MiRemote::stop() {
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

		void MiRemote::start() {
			if (this->reconnect_) {
				pServer->advertiseOnDisconnect(true);
			}

			pServer->startAdvertising();
		}

		void MiRemote::update() { state_sensor_->publish_state(BLEMiRemote.isConnected()); }

		bool MiRemote::is_connected() {
			if (!BLEMiRemote.isConnected()) {
				ESP_LOGI(TAG, "Disconnected");

				return false;
			}

			return true;
		}

		void MiRemote::update_timer() {
			this->cancel_timeout((const std::string) TAG);
			this->set_timeout((const std::string) TAG, release_delay_, [this]() { this->release(); });
		}

		void MiRemote::press(uint8_t key, bool with_timer) {
			if (this->is_connected()) {
				if (with_timer) {
					this->update_timer();
				}

				BLEMiRemote.press(key);
			}
		}

		void MiRemote::pressSpecial(uint8_t key, bool with_timer) {
			if (this->is_connected()) {
				if (with_timer) {
					this->update_timer();
				}
				BLEMiRemote.pressSpecial(key);
			}
		}

		void MiRemote::release() {
			if (this->is_connected()) {
				this->cancel_timeout((const std::string) TAG);
				BLEMiRemote.releaseAll();
			}
		}

	}  // namespace mi_remote
}  // namespace esphome

#endif
