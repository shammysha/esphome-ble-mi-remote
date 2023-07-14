#pragma once

#ifdef USE_ESP32

#include "esphome/core/component.h"
#include "esphome/core/automation.h"
#include "ble_mi_remote.h"
#include <string>

namespace esphome {
	namespace ble_mi_remote {
		template<typename... Ts> class BleMiRemotePressAction : public Action<Ts...> {
			public:
				explicit BleMiRemotePressAction(BleMiRemote *ble_mi_remote) : ble_mi_remote_(ble_mi_remote) {}
				TEMPLATABLE_VALUE(uint8_t, key)
				TEMPLATABLE_VALUE(uint8_t, special)

				void play(Ts... x) override {
					if (this->key_.has_value()) {
						this->ble_mi_remote_->press(this->key_.value(x...));
					} else if (this->special_.has_value()) {
						this->ble_mi_remote_->pressSpecial(this->special_.value(x...));
					}
				}
			protected:
				BleMiRemote *ble_mi_remote_;
		};

		template<typename... Ts> class BleMiRemoteReleaseAction : public Action<Ts...> {
			public:
				explicit BleMiRemoteReleaseAction(BleMiRemote *ble_mi_remote) : ble_mi_remote_(ble_mi_remote) {}

				void play(Ts... x) override { this->ble_mi_remote_->release(); }

			protected:
				BleMiRemote *ble_mi_remote_;
		};

		template<typename... Ts> class BleMiRemoteStartAction : public Action<Ts...> {
			public:
				explicit BleMiRemoteStartAction(BleMiRemote *ble_mi_remote) : ble_mi_remote_(ble_mi_remote) {}

				void play(Ts... x) override { this->ble_mi_remote_->start(); }

			protected:
				BleMiRemote *ble_mi_remote_;
		};

		template<typename... Ts> class BleMiRemoteStopAction : public Action<Ts...> {
			public:
				explicit BleMiRemoteStopAction(BleMiRemote *ble_mi_remote) : ble_mi_remote_(ble_mi_remote) {}

				void play(Ts... x) override { this->ble_mi_remote_->stop(); }

			protected:
				BleMiRemote *ble_mi_remote_;
		};
	}  // namespace ble_mi_remote
}  // namespace esphome

#endif
