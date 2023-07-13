
#ifdef USE_ESP32

#include "button.h"
#include "esphome/core/log.h"
#include <string>

namespace esphome {
	namespace mi_remote {
		static const char *const TAG = "mi_remote";

		void MiRemoteButton::press_action() {
			if (value > -1) {
				parent_->pressSpecial((uint8_t) value_);
			} else {
				parent_->release();
			}
		}
	}  // namespace mi_remote
}  // namespace esphome

#endif
