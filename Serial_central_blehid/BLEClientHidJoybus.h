#include "bluefruit_common.h"
#include "BLEClientCharacteristic.h"
#include "BLEClientService.h"

#include "services/BLEHidGeneric.h"

class BLEClientHidJoybus : public BLEClientService
{
  public:

    typedef void (*joybus_callback_t ) (uint32_t* report);

    BLEClientHidJoybus(void);

    virtual bool  begin(void);
    virtual bool  discover(uint16_t conn_handle);

    void setJoybusReportCallback(joybus_callback_t fp);

  protected:

    joybus_callback_t _joybus_cb;
    BLEClientCharacteristic _joybus_report;

};
