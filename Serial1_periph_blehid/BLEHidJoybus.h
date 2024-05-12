#ifndef BLEHID_JOYBUS_H_
#define BLEHID_JOYBUS_H_

#include "bluefruit_common.h"

#include "BLECharacteristic.h"
#include "services/BLEHidGeneric.h"
#include "BLEService.h"

class BLEHidJoybus : public BLEHidGeneric
{
  public:

    BLEHidJoybus(void);

    virtual err_t begin(void);

    bool report(uint16_t conn_hdl, uint32_t* report);

  protected:

};

#endif