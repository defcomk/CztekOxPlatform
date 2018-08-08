#ifndef __EEPROM_BL24SA64_H__
#define __EEPROM_BL24SA64_H__
#include "OtpSensor.h"
#include "EEPROM_Normal.h"

class EEPROM_BL24SA64 : public EEPROM_Normal
{
public:
    EEPROM_BL24SA64(uint i2cAddr);
    ~EEPROM_BL24SA64(void);

    int OtpRead(uint startAddr, uint endAddr, uchar *buf, ushort page = 0);
    int OtpWrite(uint startAddr, uint endAddr, const uchar *buf, ushort page = 0);
};

#endif
