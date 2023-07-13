#pragma once

#ifdef USE_ESP32

#include "esphome/core/component.h"
#include "esphome/components/button/button.h"
#include "mi_remote.h"
#include <string>

namespace esphome {
	namespace mi_remote {
		class MiRemoteButton : public button::Button, public Component {
			public:
				void set_parent(MiRemote *parent) { parent_ = parent; }

				void set_value(int8_t value) { value_ = value;	}

			protected:
				void press_action() override;

				int8_t value_{-1};

				MiRemote *parent_;
		};
	}  // namespace mi_remote
}  // namespace esphome

#endif
