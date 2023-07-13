#pragma once

#ifdef USE_ESP32

#include "esphome/core/component.h"
#include "esphome/core/automation.h"
#include "../mi_remote/mi_remote.h"
#include <string>

namespace esphome {
	namespace mi_remote {
		template<typename... Ts> class MiRemotePressAction : public Action<Ts...> {
			public:
				explicit MiRemotePressAction(MiRemote *mi_remote) : mi_remote_(mi_remote) {}
				TEMPLATABLE_VALUE(uint8_t, key)
				TEMPLATABLE_VALUE(uint8_t, special)

				void play(Ts... x) override {
					if (this->key_.has_value()) {
						this->mi_remote_->press(this->key_.value(x...));
					} else if (this->special_.has_value()) {
						this->mi_remote_->pressSpecial(this->special_.value(x...));
					}
				}
			protected:
				MiRemote *mi_remote_;
		};

		template<typename... Ts> class MiRemoteReleaseAction : public Action<Ts...> {
			public:
				explicit MiRemoteReleaseAction(MiRemote *mi_remote) : mi_remote_(mi_remote) {}

				void play(Ts... x) override { this->mi_remote_->release(); }

			protected:
				MiRemote *mi_remote_;
		};

		template<typename... Ts> class MiRemoteStartAction : public Action<Ts...> {
			public:
				explicit MiRemoteStartAction(MiRemote *mi_remote) : mi_remote_(mi_remote) {}

				void play(Ts... x) override { this->mi_remote_->start(); }

			protected:
				MiRemote *mi_remote_;
		};

		template<typename... Ts> class MiRemoteStopAction : public Action<Ts...> {
			public:
				explicit MiRemoteStopAction(MiRemote *mi_remote) : mi_remote_(mi_remote) {}

				void play(Ts... x) override { this->mi_remote_->stop(); }

			protected:
				MiRemote *mi_remote_;
		};
	}  // namespace mi_remote
}  // namespace esphome

#endif
