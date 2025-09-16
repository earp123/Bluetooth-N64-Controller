# Joybus Driver and Wireless application
This projects seeks to add wireless capability to origianl Nintendo 64 consoles by way of emulating the Joybus protocol.
https://n64brew.dev/wiki/Joybus_Protocol

Included in this repository is a proof of conecpt application that uses the NXP chip on board the Teensy 4 to measure and bit bang the high frequency pulse widths used to encode the controller response. The prototype also used a pair of Adafruit BLE boards to transmit and reeceieve the joybus data.

From this, I sought a smaller BOM and a tighter hardware interface and thus arrived at the Pico W. The PIO modules supported by the RP2040 make for a perfect solution to bit bang Joybus data with an on board wireless chip. 
