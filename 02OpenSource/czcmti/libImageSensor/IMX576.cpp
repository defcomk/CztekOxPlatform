
#include "IMX576.h"
#include <QThread>
#include <QVector>
#include "czcmtidefs.h"
#include "macrodefs.h"

// 参考文档《IMX576-AAKH5_Software_Reference_Manual_0.0.1.pdf》
// 参考OTP读写操作的文档《IMX576_AAKH5_OTP_Manual_0.0.1.pdf》
/** Temperature Reg Addr Define **/
#define TEMPATURE_CTRL_ADDR         0x0138
#define TEMP_CTRL_DIS               (0x00 << 0)
#define TEMP_CTRL_EN                (0x01 << 0)
#define TEMPATURE_DATA_ADDR         0x013A

/** Exposure Reg Addr Define **/
#define EXPOSURE_REG_H_ADDR         0x0202
#define EXPOSURE_REG_L_ADDR         0x0203

/** Analog Gain Reg Addr Define **/
#define SENSOR_GAIN_REG_H_ADDR      0x0204
#define SENSOR_GAIN_REG_L_ADDR      0x0205

/** Channel GAIN Reg Addr Define **/
#define GAIN_DEFAULT                0x0100
#define GAIN_GREEN1_HIGH_ADDR       0x020E
#define GAIN_GREEN1_LOW_ADDR        0x020F
#define GAIN_BLUE_HIGH_ADDR         0x0212
#define GAIN_BLUE_LOW_ADDR          0x0213
#define GAIN_RED_HIGH_ADDR          0x0210
#define GAIN_RED_LOW_ADDR           0x0211
#define GAIN_GREEN2_HIGH_ADDR       0x0214
#define GAIN_GREEN2_LOW_ADDR        0x0215

#define DPGA_USE_GLOBAL_GAIN        0x3FF9
#define DIGITAL_GAIN_BY_COLOR       (0x00 << 0)
#define DIGITAL_GAIN_ALL_COLOR      (0x01 << 0)

/** OTP Reg Addr Define **/
#define REG_PAGE_SEL_ADDR           0x0A02
#define REG_DATA_START_ADDR         0x0A04
#define REG_DATA_END_ADDR           0x0A43
#define PAGE_LEN                    (REG_DATA_END_ADDR - REG_DATA_START_ADDR + 1)

/** Fuse ID Reg Addr Define **/
#define FUSE_ID_PAGE                0x3F
#define FUSE_ID_ADDR_START          0x0A20
#define FUSE_ID_ADDR_END            0x0A2A
#define FUSE_ID_LEN                 (FUSE_ID_ADDR_END - FUSE_ID_ADDR_START + 1)


IMX576::IMX576(const T_SensorSetting &sensorSetting) :
    ImageSensor(sensorSetting)
{
}


int IMX576::GetTemperature(int &temperature)
{
    ushort tempCtlData = 0;
    int ec = I2cRead(TEMPATURE_CTRL_ADDR, tempCtlData, I2C_MODE_ADDR16_DATA8);
    if (ec < 0) {
        QString strLog = "Read IMX576 TEMPATURE CTRL Register failed";
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        qCritical() << strLog;
        return ec;
    }
    if ((tempCtlData & 0x01) != 0x01) {
        ec = I2cWrite(TEMPATURE_CTRL_ADDR, 0x01, I2C_MODE_ADDR16_DATA8);
        if (ec < 0) {
            QString strLog = "Wtrie IMX576 temperature control Register failed";
            m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
            qCritical() << strLog;
            return ec;
        }
        QThread::msleep(200); // delay 2 frames
    }

    ushort ubTemp = 0x00;
    ec = I2cRead(TEMPATURE_DATA_ADDR, ubTemp, I2C_MODE_ADDR16_DATA8);
    if (ec < 0) {
        QString strLog = "Read IMX576 temperature data Register failed";
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        qCritical() << strLog;
        return ec;
    }

    /**
      * 温度范围：-20°~80°，负温度采用补码表示，正温度采用原码表示。
      *     1、0x81~0xEC：都是-20°
      *     2、0xED~0xFF：-19°~-1°
      *     3、0x00：0°
      *     4、0x01~0x4F：1°~79°
      *     5、0x50~0x7F：都是80°
      * ----20180411
    **/
    if (ubTemp >= 0xED/* && ubTemp <= 0xFF*/)
    {
        temperature = (int)(ubTemp - 256);
    }
    else if (ubTemp >= 0x81 && ubTemp <= 0xEC)
    {
        temperature = -20;
    }
    else if (ubTemp >= 0x50 && ubTemp <= 0x7F)
    {
        temperature = 80;
    }
    else if (/*ubTemp >= 0x00 && */ubTemp < 0x50)
    {
        temperature = ubTemp;
    }
    else    //处理0x80的值，手册没有说明，按手册的写法，应该是【-0°】的补码
    {
        temperature = 0;
    }

    return ERR_NoError;
}

int IMX576::GetSensorFuseId(std::string &fuseId, bool bUseMasterI2C)
{
	(void)bUseMasterI2C;
    uchar tmpBuff[FUSE_ID_LEN] = {0};
    int ec = readOtpData(FUSE_ID_ADDR_START, FUSE_ID_ADDR_END, tmpBuff, FUSE_ID_PAGE);
    if (ec < 0) {
        QString strLog = QString("Read IMX576 FuseId failed[%1]").arg(ec);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        qCritical() << strLog;
        return ec;
    }
    fuseId = "";
    for (uint i = 0; i < ARRAY_SIZE(tmpBuff); i++) {
        QString strFuseId = "";
        strFuseId = QString::asprintf("%02X", tmpBuff[i]);
        fuseId += strFuseId.toStdString();
    }
    return ERR_NoError;
}

int IMX576::ApplyAWBGain(int rg, int bg, int Typical_rg, int Typical_bg)
{
    ushort r_ratio, b_ratio;
    r_ratio = 512 * (Typical_rg) /(rg);
    b_ratio = 512 * (Typical_bg) /(bg);

    ushort R_GAIN;
    ushort B_GAIN;
    ushort Gr_GAIN;
    ushort Gb_GAIN;
    ushort G_GAIN;

    if(r_ratio >= 512 )
    {
        if(b_ratio>=512)
        {
            R_GAIN = (ushort)(GAIN_DEFAULT * r_ratio / 512);
            G_GAIN = GAIN_DEFAULT;
            B_GAIN = (ushort)(GAIN_DEFAULT * b_ratio / 512);
        }
        else
        {
            R_GAIN =  (ushort)(GAIN_DEFAULT * r_ratio / b_ratio);
            G_GAIN = (ushort)(GAIN_DEFAULT * 512 / b_ratio);
            B_GAIN = GAIN_DEFAULT;
        }
    }
    else
    {
        if(b_ratio >= 512)
        {
            R_GAIN = GAIN_DEFAULT;
            G_GAIN =(ushort)(GAIN_DEFAULT * 512 / r_ratio);
            B_GAIN =(ushort)(GAIN_DEFAULT *  b_ratio / r_ratio);

        }
        else
        {
            Gr_GAIN = (ushort)(GAIN_DEFAULT * 512 / r_ratio );
            Gb_GAIN = (ushort)(GAIN_DEFAULT * 512 / b_ratio );

            if(Gr_GAIN >= Gb_GAIN)
            {
                R_GAIN = GAIN_DEFAULT;
                G_GAIN = (ushort)(GAIN_DEFAULT * 512 / r_ratio );
                B_GAIN = (ushort)(GAIN_DEFAULT * b_ratio / r_ratio);
            }
            else
            {
                R_GAIN =  (ushort)(GAIN_DEFAULT * r_ratio / b_ratio );
                G_GAIN = (ushort)(GAIN_DEFAULT * 512 / b_ratio );
                B_GAIN = GAIN_DEFAULT;
            }
        }
    }

    QVector<ST_RegData> vtRegData;
    ST_RegData REG_SET_TAB[]=
    {
        //{DPGA_USE_GLOBAL_GAIN,      DIGITAL_GAIN_BY_COLOR},
        {GAIN_RED_HIGH_ADDR,        MSB(R_GAIN)},
        {GAIN_RED_LOW_ADDR,         LSB(R_GAIN)},
        {GAIN_GREEN1_HIGH_ADDR,     MSB(G_GAIN)},
        {GAIN_GREEN1_LOW_ADDR,      LSB(G_GAIN)},
        {GAIN_GREEN2_HIGH_ADDR,     MSB(G_GAIN)},
        {GAIN_GREEN2_LOW_ADDR,      LSB(G_GAIN)},
        {GAIN_BLUE_HIGH_ADDR,       MSB(B_GAIN)},
        {GAIN_BLUE_LOW_ADDR,        LSB(B_GAIN)}
    };

    unsigned int i = 0;
    for (i = 0; i < sizeof (REG_SET_TAB) / sizeof (REG_SET_TAB[0]); i++)
    {
        vtRegData.push_back(REG_SET_TAB[i]);
    }

    return writeRegisters(vtRegData, 3);
}

int IMX576::GetSensorExposure(uint &value)
{
    uchar buf[] = {0, 0};
    int ec = i2cReadContinuous(EXPOSURE_REG_H_ADDR, 2, buf, ARRAY_SIZE(buf));
    if (ec < 0) {
        QString strLog = QString("IMX576::%1---ReadContinuousI2c() failed[%2]")
                                .arg(__FUNCTION__).arg(ec);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        qCritical() << strLog;
        return ec;
    }

    value = MAKEWORD(buf[0], buf[1]);
    return ERR_NoError;
}

int IMX576::SetSensorExposure(uint value)
{
    uchar buf[] = {MSB(value), LSB(value)};
    int ec = i2cWriteContinuous(EXPOSURE_REG_H_ADDR, 2, buf, ARRAY_SIZE(buf));
    if (ec < 0) {
        QString strLog = QString("IMX576::Call WriteContinuousI2c() failed[%1]").arg(ec);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        qCritical() << strLog;
        return ec;
    }

    return ERR_NoError;
}

int IMX576::GetSensorGain(uint &value)
{
    uchar buf[] = {0, 0};
    int ec = i2cReadContinuous(SENSOR_GAIN_REG_H_ADDR, 2, buf, ARRAY_SIZE(buf));
    if (ec < 0) {
        QString strLog = QString("IMX576::Call ReadContinuousI2c() failed[%1]").arg(ec);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        qCritical() << strLog;
        return ec;

    }

    value = MAKEWORD(buf[0], buf[1]);
    return ERR_NoError;
}

int IMX576::SetSensorGain(uint value, bool bMultiple)
{
    if (true == bMultiple)
    {
        value = 1024 - (1024 / value);
        //return SetSensorGain(value, false);
    }
    if (value > 960)
    {
        value = 960;  // 0 ~ 960
    }

    uchar buf[] = {MSB(value), LSB(value)};
    int ec = i2cWriteContinuous(SENSOR_GAIN_REG_H_ADDR, 2, buf, ARRAY_SIZE(buf));
    if (ec < 0) {
        QString strLog = QString("IMX519::Call WriteContinuousI2c() failed[%1]").arg(ec);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        qCritical() << strLog;
        return ec;
    }

    return ERR_NoError;
}

int IMX576::readOtpData(ushort startAddr, ushort endAddr, uchar *buf, ushort page)
{
    if (nullptr == buf) {
        qCritical("IMX576::%s Error---buf is NULL", __FUNCTION__);
        return ERR_InvalidParameter;
    }

    uint bufferLen = endAddr - startAddr + 1;
    if (bufferLen > PAGE_LEN) {
        qCritical("IMX576::%s Error---nLen:%d is outside PAGE_LEN:%d", __FUNCTION__, bufferLen, PAGE_LEN);
        return ERR_InvalidParameter;
    }

    int ec = ERR_NoError;
    ec = I2cWrite(0x0A02, page, I2C_MODE_ADDR16_DATA8);     // select otp page
    ec |= I2cWrite(0x0A00, 0x01, I2C_MODE_ADDR16_DATA8);    // turn on otp read mode
    if (ec < 0) {
        qCritical("Turn on IMX576 otp read mode failed[%d]", ec);
        return ec;
    }

    bool checkFailed = true;
    ushort flag = 0;
    for (uint i = 0; i < 20; i++) { // check status repeat 20 times
        if (ERR_NoError == I2cRead(0x0A01, flag, I2C_MODE_ADDR16_DATA8)) {
            if ((flag & 0x01) == 0x01) {
                checkFailed = false;
                break;
            }
        }
        QThread::msleep(10);
    }
    if (checkFailed) {
        qCritical("Check IMX576 OTP status[0x%x] failed", flag);
        return ERR_Failed;
    }

    //read otp data to buff
    ec = i2cReadContinuous(startAddr, 2, buf, bufferLen);
    if (ec < 0) {
        qCritical("IMX576 read otp addr[0x%x] failed[%d]", startAddr, ec);
        return ec;
    }

    return ec;
}

