#include "BLEHidJoybus.h"
#include <bluefruit.h>
#include "bluefruit_common.h"


BLEDis bledis;
BLEHidJoybus blejoybus;

uint8_t buf[4] = "aa";

void setup()
{
  Serial.begin(115200);

  Serial.println("Begin......................");
  

  Bluefruit.begin();
  Bluefruit.setTxPower(4);    // Check bluefruit.h for supported values

  // Configure and Start Device Information Service
  bledis.setManufacturer("Adafruit Industries");
  bledis.setModel("Bluefruit Feather 52");
  bledis.begin();

  blejoybus.begin();

  startAdv();
}

void startAdv(void)
{
  // Advertising packet
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addTxPower();
  Bluefruit.Advertising.addAppearance(BLE_APPEARANCE_HID_GAMEPAD);

  // Include BLE HID service
  Bluefruit.Advertising.addService(blejoybus);

  // There is enough room for the dev name in the advertising packet
  Bluefruit.Advertising.addName();


  Bluefruit.Advertising.restartOnDisconnect(true);
  Bluefruit.Advertising.setInterval(32, 244);    // in unit of 0.625 ms
  Bluefruit.Advertising.setFastTimeout(30);      // number of seconds in fast mode
  Bluefruit.Advertising.start(0);                // 0 = Don't stop advertising after n seconds
}

void loop()
{
  while(Serial.available()) 
  {
    Serial.println("Got somethin.");
    uint16_t conn_hdl = Bluefruit.connHandle();
    int count = Serial.readBytes(buf, sizeof(buf));
    blejoybus.report(conn_hdl, (uint32_t *) buf);
    
  }
  
}

