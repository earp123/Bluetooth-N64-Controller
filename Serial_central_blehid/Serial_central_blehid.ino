#include <bluefruit.h>
#include "BLEClientHidJoybus.h"

BLEClientHidJoybus hid;

uint8_t output_buffer[4]= {0};


void setup()
{
  Serial.begin(1000000);

  // Initialize Bluefruit with maximum connections as Peripheral = 0, Central = 1
  // SRAM usage required by SoftDevice will increase dramatically with number of connections
  Bluefruit.begin(0, 1);
  
  Bluefruit.setName("Bluefruit52 Central");

  // Init BLE Central Hid Serivce
  hid.begin();

  hid.setJoybusReportCallback(joybus_report_callback);
 

  // Increase Blink rate to different from PrPh advertising mode
  Bluefruit.setConnLedInterval(250);

  // Callbacks for Central
  Bluefruit.Central.setConnectCallback(connect_callback);
  Bluefruit.Central.setDisconnectCallback(disconnect_callback);

  // Set connection secured callback, invoked when connection is encrypted
  Bluefruit.Security.setSecuredCallback(connection_secured_callback);

  /* Start Central Scanning
   * - Enable auto scan if disconnected
   * - Interval = 100 ms, window = 80 ms
   * - Don't use active scan
   * - Filter only accept HID service in advertising
   * - Start(timeout) with timeout = 0 will scan forever (until connected)
   */
  Bluefruit.Scanner.setRxCallback(scan_callback);
  Bluefruit.Scanner.restartOnDisconnect(true);
  Bluefruit.Scanner.setInterval(160, 80); // in unit of 0.625 ms
  Bluefruit.Scanner.filterService(hid);   // only report HID service
  Bluefruit.Scanner.useActiveScan(true);
  Bluefruit.Scanner.start(0);             // 0 = Don't stop scanning after n seconds
}

/**
 * Callback invoked when scanner pick up an advertising data
 * @param report Structural advertising data
 */
void scan_callback(ble_gap_evt_adv_report_t* report)
{
  // Since we configure the scanner with filterUuid()
  // Scan callback only invoked for device with hid service advertised  
  // Connect to the device with hid service in advertising packet
  Bluefruit.Central.connect(report);
}

/**
 * Callback invoked when an connection is established
 * @param conn_handle
 */
void connect_callback(uint16_t conn_handle)
{
  BLEConnection* conn = Bluefruit.Connection(conn_handle);

  if ( hid.discover(conn_handle) )
  {
    // HID device mostly require pairing/bonding
    conn->requestPairing();
  }else
  {    
    // disconnect since we couldn't find blehid service
    conn->disconnect();
  }

  //TODO change some status LEDs or something since we don't have Serial to debug with
}

void connection_secured_callback(uint16_t conn_handle)
{
  BLEConnection* conn = Bluefruit.Connection(conn_handle);

  if ( !conn->secured() )
  {
    // It is possible that connection is still not secured by this time.
    // This happens (central only) when we try to encrypt connection using stored bond keys
    // but peer reject it (probably it remove its stored key).
    // Therefore we will request an pairing again --> callback again when encrypted
    conn->requestPairing();
  }
  else
  {
    
  }
}

/**
 * Callback invoked when a connection is dropped
 * @param conn_handle
 * @param reason is a BLE_HCI_STATUS_CODE which can be found in ble_hci.h
 */
void disconnect_callback(uint16_t conn_handle, uint8_t reason)
{
  (void) conn_handle;
  (void) reason;
  
}

void loop()
{
  
}


void joybus_report_callback(uint32_t* report)
{
  for(int i = 0; i < 4; i++) output_buffer[i]= (uint8_t) report[i];

  Serial.write(output_buffer, 4);
}



