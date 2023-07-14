#pragma once

#ifdef USE_ESP32

#include "esphome/core/component.h"
#include "esphome/components/button/button.h"
#include "ble_mi_remote.h"
#include <string>

namespace esphome {
	namespace ble_mi_remote {
		class BleMiRemoteButton : public button::Button, public Component {
			public:
				void set_parent(BleMiRemote *parent) { parent_ = parent; }

				void set_value(int8_t value) { value_ = value; }

			protected:
				void press_action() override;

				int8_t value_{-1};

				BleMiRemote *parent_;
		};
	}  // namespace ble_mi_remote
}  // namespace esphome

#endif
