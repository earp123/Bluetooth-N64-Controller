# PROTOTYPE Application Bluetooth-N64-Controller
Open Source files for a pair of Teensy 4s and a pair of Nordic SoCs that implement a custom [Joybus](https://n64brew.dev/wiki/Joybus_Protocol) driver and accompanying Bluetooth interface between a Nintendo 64 and its controller. This is a prototype repository that contains arduino code for the Teensys and adafruit bluetooth kits that use the nordic BLE UART service.

Results were fair. Input response capture is nice and speedy with no noticeable latency during gameplay. Still some spontaneous false button presses, likely due to the sensitvity of the hardware at such speeds. 

 

