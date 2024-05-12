#include "bluefruit.h"
#include "BLEHidJoybus.h"

// For using USB HID descriptor template
#include "class/hid/hid_device.h"

enum { REPORT_ID_GAMEPAD = 1 };

uint8_t const hid_gamepad_report_descriptor[] =
{
  TUD_HID_REPORT_DESC_GAMEPAD( HID_REPORT_ID(REPORT_ID_GAMEPAD) )
};

BLEHidJoybus::BLEHidJoybus(void)
  : BLEHidGeneric(1, 0, 0)
{

}

err_t BLEHidJoybus::begin(void)
{
  uint16_t input_len [] = { sizeof(uint32_t) };

  setReportLen(input_len, NULL, NULL);
  enableKeyboard(false);
  enableMouse(false);
  setReportMap(hid_gamepad_report_descriptor, sizeof(hid_gamepad_report_descriptor));

  VERIFY_STATUS( BLEHidGeneric::begin() );

  // Attempt to change the connection interval to 11.25-15 ms when starting HID
  Bluefruit.Periph.setConnInterval(6, 6);

  return ERROR_NONE;
}

bool BLEHidJoybus::report(uint16_t conn_hdl, uint32_t* report)
{
  return inputReport(conn_hdl, REPORT_ID_GAMEPAD, report, sizeof(report));
}