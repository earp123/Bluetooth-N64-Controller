#include "bluefruit.h"
#include "BLEClientHidJoybus.h"

BLEClientHidJoybus::BLEClientHidJoybus(void)
 : BLEClientService(UUID16_SVC_HUMAN_INTERFACE_DEVICE),
   _joybus_report(UUID16_CHR_REPORT)
{
  _joybus_cb = NULL;

}

bool BLEClientHidJoybus::begin(void)
{
  BLEClientService::begin();

  _joybus_report.begin(this);

  return true;

}

void BLEClientHidJoybus::setJoybusReportCallback(joybus_callback_t fp)
{
  _joybus_cb = fp;
}

bool BLEClientHidJoybus::discover(uint16_t conn_handle)
{
  
  // Call Base class discover
  VERIFY( BLEClientService::discover(conn_handle) );
  _conn_hdl = BLE_CONN_HANDLE_INVALID; // make as invalid until we found all chars

  // Discover all characteristics
  Bluefruit.Discovery.discoverCharacteristic(conn_handle, _joybus_report);//TODO not declared?

  _conn_hdl = conn_handle;
  return true;
}