# esphome-mi-remote
Bluetooth remote controller for Xiaomi MI TV

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
