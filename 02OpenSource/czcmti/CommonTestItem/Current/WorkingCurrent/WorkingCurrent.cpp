#include "WorkingCurrent.h"
#include <sstream>
#include <QDebug>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

CZPLUGIN_API ICzPlugin *CreatePlugin(void *arg)
{
    return new WorkingCurrent((const char *)arg);
}

CZPLUGIN_API void DestroyPlugin(ICzPlugin **plugin)
{
    WorkingCurrent *pTestItem = (WorkingCurrent *)(*plugin);
    if (pTestItem != nullptr) {
        delete pTestItem; pTestItem = nullptr;
        *plugin = nullptr;
    }
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

WorkingCurrent::WorkingCurrent(const char *instanceName)
{
    m_instanceName = QString::fromLatin1(instanceName);
    m_wdtConf = new ConfWidget();
    m_option = Option::GetInstance();
    m_context = nullptr;
    if (m_instanceName.isEmpty())
        m_instanceName = "Working Current";

    m_current_nA = nullptr;
    m_size = 0;
}

WorkingCurrent::~WorkingCurrent()
{
    delete m_wdtConf;
    Option::FreeInstance();
    m_context = nullptr;
    if (nullptr != m_current_nA)
    {
        delete[] m_current_nA;
        m_current_nA = nullptr;
    }
}

int WorkingCurrent::GetPluginInfo(T_PluginInfo &pluginInfo)
{
    pluginInfo.Version = 0x09000004;
    strcpy(pluginInfo.Description, "Working current.(20180319)");
    strcpy(pluginInfo.FriendlyName, "Working Current");    
    strcpy(pluginInfo.InstanceName, m_instanceName.toLatin1().constData());
    strcpy(pluginInfo.VendorName, "CZTEK");
    pluginInfo.OptionDlgHandle = (long)m_wdtConf;

    return ERR_NoError;
}

int WorkingCurrent::LoadOption()
{
    QMap<QString, QString> configurations;
    m_context->ModuleSettings->ReadSection(m_instanceName, configurations);
    // Enabled
    if (configurations.contains("dvddEnabled")) {
        m_option->DvddEnabled = (configurations["dvddEnabled"] == "true");
    }
    if (configurations.contains("avddEnabled")) {
        m_option->AvddEnabled = (configurations["avddEnabled"] == "true");
    }
    if (configurations.contains("dovddEnabled")) {
        m_option->DovddEnabled = (configurations["dovddEnabled"] == "true");
    }
    if (configurations.contains("afvccEnabled")) {
        m_option->AfvccEnabled = (configurations["afvccEnabled"] == "true");
    }
    if (configurations.contains("vppEnabled")) {
        m_option->VppEnabled = (configurations["vppEnabled"] == "true");
    }
    if (configurations.contains("totalEnabled")) {
        m_option->TotalEnabled = (configurations["totalEnabled"] == "true");
    }
    // Range
    if (configurations.contains("dvddRange")) {
        m_option->DvddRange = configurations["dvddRange"].toInt();
    }
    if (configurations.contains("avddRange")) {
        m_option->AvddRange = configurations["avddRange"].toInt();
    }
    if (configurations.contains("dovddRange")) {
        m_option->DovddRange = configurations["dovddRange"].toInt();
    }
    if (configurations.contains("afvccRange")) {
        m_option->AfvccRange = configurations["afvccRange"].toInt();
    }
    if (configurations.contains("vppRange")) {
        m_option->VppRange = configurations["vppRange"].toInt();
    }
    if (configurations.contains("totalRange")) {
        m_option->TotalRange = configurations["totalRange"].toInt();
    }
    // LowerLimit
    if (configurations.contains("dvddLowerLimit")) {
        m_option->DvddLowerLimit = configurations["dvddLowerLimit"].toInt();
    }
    if (configurations.contains("avddLowerLimit")) {
        m_option->AvddLowerLimit = configurations["avddLowerLimit"].toInt();
    }
    if (configurations.contains("dovddLowerLimit")) {
        m_option->DovddLowerLimit = configurations["dovddLowerLimit"].toInt();
    }
    if (configurations.contains("afvccLowerLimit")) {
        m_option->AfvccLowerLimit = configurations["afvccLowerLimit"].toInt();
    }
    if (configurations.contains("vppLowerLimit")) {
        m_option->VppLowerLimit = configurations["vppLowerLimit"].toInt();
    }
    if (configurations.contains("totalLowerLimit")) {
        m_option->TotalLowerLimit = configurations["totalLowerLimit"].toInt();
    }
    // UpperLimit
    if (configurations.contains("dvddUpperLimit")) {
        m_option->DvddUpperLimit = configurations["dvddUpperLimit"].toInt();
    }
    if (configurations.contains("avddUpperLimit")) {
        m_option->AvddUpperLimit = configurations["avddUpperLimit"].toInt();
    }
    if (configurations.contains("dovddUpperLimit")) {
        m_option->DovddUpperLimit = configurations["dovddUpperLimit"].toInt();
    }
    if (configurations.contains("afvccUpperLimit")) {
        m_option->AfvccUpperLimit = configurations["afvccUpperLimit"].toInt();
    }
    if (configurations.contains("vppUpperLimit")) {
        m_option->VppUpperLimit = configurations["vppUpperLimit"].toInt();
    }
    if (configurations.contains("totalUpperLimit")) {
        m_option->TotalUpperLimit = configurations["totalUpperLimit"].toInt();
    }

    return m_wdtConf->Cache2Ui();
}

int WorkingCurrent::SaveOption()
{
    int ec = m_wdtConf->Ui2Cache();
    if (ec == 0) {
        QMap<QString, QString> configurations;
        // Enabled
        configurations["dvddEnabled"] =  m_option->DvddEnabled ? "true" : "false";
        configurations["avddEnabled"] =  m_option->AvddEnabled ? "true" : "false";
        configurations["dovddEnabled"] =  m_option->DovddEnabled ? "true" : "false";
        configurations["afvccEnabled"] =  m_option->AfvccEnabled ? "true" : "false";
        configurations["vppEnabled"] =  m_option->VppEnabled ? "true" : "false";
        configurations["totalEnabled"] =  m_option->TotalEnabled ? "true" : "false";
        // Range
        configurations["dvddRange"] = QString::number(m_option->DvddRange);
        configurations["avddRange"] = QString::number(m_option->AvddRange);
        configurations["dovddRange"] = QString::number(m_option->DovddRange);
        configurations["afvccRange"] = QString::number(m_option->AfvccRange);
        configurations["vppRange"] = QString::number(m_option->VppRange);
        configurations["totalRange"] = QString::number(m_option->TotalRange);
        // LowerLimit
        configurations["dvddLowerLimit"] = QString::number(m_option->DvddLowerLimit);
        configurations["avddLowerLimit"] = QString::number(m_option->AvddLowerLimit);
        configurations["dovddLowerLimit"] = QString::number(m_option->DovddLowerLimit);
        configurations["afvccLowerLimit"] = QString::number(m_option->AfvccLowerLimit);
        configurations["vppLowerLimit"] = QString::number(m_option->VppLowerLimit);
        configurations["totalLowerLimit"] = QString::number(m_option->TotalLowerLimit);
        // UpperLimit
        configurations["dvddUpperLimit"] = QString::number(m_option->DvddUpperLimit);
        configurations["avddUpperLimit"] = QString::number(m_option->AvddUpperLimit);
        configurations["dovddUpperLimit"] = QString::number(m_option->DovddUpperLimit);
        configurations["afvccUpperLimit"] = QString::number(m_option->AfvccUpperLimit);
        configurations["vppUpperLimit"] = QString::number(m_option->VppUpperLimit);
        configurations["totalUpperLimit"] = QString::number(m_option->TotalUpperLimit);
        // save to file
        ec = m_context->ModuleSettings->WriteSection(m_instanceName, configurations);
    }
    return ec;
}

int WorkingCurrent::RestoreDefaults()
{
    return m_wdtConf->RestoreDefaults();
}

int WorkingCurrent::BindChannelContext(T_ChannelContext *context)
{
    m_context = context;
    LoadOption();
    return ERR_NoError;
}

ITestItem::E_ItemType WorkingCurrent::GetItemType() const
{
    return ItemType_Hardware;
}

bool WorkingCurrent::GetIsSynchronous() const
{
    return true;
}

QString WorkingCurrent::GetReportHeader() const
{
    QString strHeader = "AFVDD(mA),VPP(mA),AVDD(mA),DOVDD(mA),DVDD(mA),";
    return strHeader;
}

QString WorkingCurrent::GetReportContent() const
{
    QString strContent = "";
    for (uint i=0; i<m_size; ++i)
    {
        strContent += QString::asprintf("%0.2f,", m_current_nA[i]/1000);
    }

    return strContent;
}

bool WorkingCurrent::GetIsEngineeringMode() const
{
    return false;
}

IHardwareTest::E_HardwareTestType WorkingCurrent::GetHardwareType() const
{
    return HardwareTest_Normal;
}

bool WorkingCurrent::GetContinueWhenFailed() const
{
    return true;
}

int WorkingCurrent::Initialize()
{
    return ERR_NoError;
}

int WorkingCurrent::RunTest(std::vector<std::string> &resultTable)
{
    resultTable.clear();
    resultTable.push_back("Name,Result,Current");
    QVector<uint> powerIdVector;
    QVector<uint> currRangeVector;
    powerIdVector.push_back(PI_DVDD);
    currRangeVector.push_back(m_option->DvddRange);
    powerIdVector.push_back(PI_AVDD);
    currRangeVector.push_back(m_option->AvddRange);
    powerIdVector.push_back(PI_DOVDD);
    currRangeVector.push_back(m_option->DovddRange);
    powerIdVector.push_back(PI_AFVCC);
    currRangeVector.push_back(m_option->AfvccRange);
    powerIdVector.push_back(PI_VPP);
    currRangeVector.push_back(m_option->VppRange);
    m_size = powerIdVector.size();
    m_current_nA = new float[m_size];
    for (uint i = 0; i < m_size; i++)
        m_current_nA[i] = 0.0;

    bool flag = true;
    int ec = m_context->ChannelController->GetCurrent(powerIdVector.constData(), currRangeVector.constData(), m_current_nA, m_size);
    if (ec == ERR_NoError) {
        auto checkOneItem = [this, &resultTable](uint powerId, float curr_nA) -> bool {
            std::string powerIdNames[] = { "DVDD", "AVDD", "DOVDD", "AFVCC", "VPP" };
            std::string rangeNames[] = { "mA", "uA", "nA" };
            int ranges[] = {
                m_option->DvddRange, m_option->AvddRange, m_option->DovddRange,
                m_option->AfvccRange, m_option->VppRange
            };
            int lowerLimits[] = {
                m_option->DvddLowerLimit, m_option->AvddLowerLimit, m_option->DovddLowerLimit,
                m_option->AfvccLowerLimit, m_option->VppLowerLimit
            };
            int upperLimits[] = {
                m_option->DvddUpperLimit, m_option->AvddUpperLimit, m_option->DovddUpperLimit,
                m_option->AfvccUpperLimit, m_option->VppUpperLimit
            };
            float val = curr_nA;
            if (ranges[powerId] == CurrentRange_mA)
                val = curr_nA / 1000000;
            else if (ranges[powerId] == CurrentRange_uA)
                val = curr_nA / 1000;
            bool isOk = (val >= lowerLimits[powerId]) && (val <= upperLimits[powerId]);
            // Name,Result,Current
            std::ostringstream ossRow;
            ossRow << powerIdNames[powerId] << "," << (isOk ? "OK" : "NG") << "," << std::to_string(val) + rangeNames[ranges[powerId]];
            // std::cout << ossRow.str() << std::endl;
            resultTable.push_back(ossRow.str());
            return isOk;
        };

        bool bEnable[] = {m_option->DvddEnabled, m_option->AvddEnabled, m_option->DovddEnabled,
                            m_option->AfvccEnabled, m_option->VppEnabled};
        for (uint i = 0; i < m_size; i++)
        {
            if (true == bEnable[i])
            {
                flag &= checkOneItem(powerIdVector[i], m_current_nA[i]);
            }
        }
    }

    //delete[] m_current_nA;  //在GetReportContent中使用
    return flag ? ERR_NoError : ERR_Failed;
}
