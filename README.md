# esphome-ble-mi-remote
Bluetooth remote controller for Xiaomi MI TV

![image](https://github.com/shammysha/esphome-ble-mi-remote/assets/65885873/acd9d0e0-b6f6-47c7-aa71-7ea0387eae70)

Usage:

```
external_components:
  - source: github://shammysha/esphome-ble-mi-remote

ble_mi_remote:
  id: mi_remote
  name: "MI Remote"
  manufacturer_id: "Shammysha"
  battery_level: 100 
  reconnect: true # auto reconnect after reboot
```

Code based on source from repositories...
* https://github.com/dmamontov/esphome-blekeyboard
* https://github.com/T-vK/ESP32-BLE-Keyboard
