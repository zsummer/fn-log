#include "fn_log.h"

static const std::string default_config_text =
R"----(
 # default is sync write channel.  
 - channel: 0
    sync: sync
    -device:0
        disable: false
        priority: debug
        out_type: vm
 - channel: 1
   sync: sync
   -device:0
        disable: false
        out_type: screen
   -device:1
        disable: false
        out_type: file
)----";

#define FNLOG_ASSERT(expr, prefix) \
{ \
    if (expr) \
    { \
        LogInfoStream(1, 0, 0) << "test " << prefix << " success."; \
    } \
    else \
    {   \
        LogErrorStream(1, 0, 0) << "test " << prefix << " failed."; \
        return __LINE__ * -1; \
    } \
}


long long g_virtual_device_write_count = 0;
void VirtualDevice(const FNLog::LogData& log)
{
    g_virtual_device_write_count++;
}


long long ChannelWrite(int channel_id)
{
    return FNLog::GetChannelLog(FNLog::GetDefaultLogger(), channel_id, FNLog::CHANNEL_LOG_PROCESSED);
}
long long DeviceWrite(int channel_id, int device_id)
{
    return FNLog::GetDeviceLog(FNLog::GetDefaultLogger(), 
        channel_id, device_id, FNLog::DEVICE_LOG_TOTAL_WRITE_LINE);
}


using namespace FNLog;
int main(int argc, char* argv[])
{
    int ret = FNLog::FastStartDefaultLogger(default_config_text);
    if (ret != 0)
    {
        return ret;
    }
    FNLog::SetVirtualDevice(&VirtualDevice);
    long long old_write = DeviceWrite(0, 0);
    LogInfoStream(0, 0, 0) << "1";
    FNLOG_ASSERT(DeviceWrite(0, 0) == old_write + 1, "check normal");

    old_write = DeviceWrite(0, 0);
    LogTraceStream(0, 0, 0) << "1";
    FNLOG_ASSERT(DeviceWrite(0, 0) == old_write, "check trace");

    old_write = DeviceWrite(0, 0);
    LogInfoStream(0, 1, 0) << "1";
    LogInfoStream(0, 8, 0) << "1";
    FNLOG_ASSERT(DeviceWrite(0, 0) == old_write + 2, "check normal category");

    old_write = DeviceWrite(0, 0);
    FNLog::SetDeviceConfig(FNLog::GetDefaultLogger(), 0, 0, FNLog::DEVICE_CFG_CATEGORY, 0);
    FNLog::SetDeviceConfig(FNLog::GetDefaultLogger(), 0, 0, FNLog::DEVICE_CFG_CATEGORY_EXTEND, 2);
    LogInfoStream(0, 1, 0) << "1";
    LogInfoStream(0, 8, 0) << "1";
    FNLOG_ASSERT(DeviceWrite(0, 0) == old_write + 1, "check device category");

    //clear
    FNLog::SetDeviceConfig(FNLog::GetDefaultLogger(), 0, 0, FNLog::DEVICE_CFG_CATEGORY_EXTEND, 0);

    old_write = DeviceWrite(0, 0);
    FNLog::SetDeviceConfig(FNLog::GetDefaultLogger(), 0, 0, FNLog::DEVICE_CFG_CATEGORY_FILTER, (1ULL <<8));
    LogInfoStream(0, 1, 0) << "1";
    LogInfoStream(0, 8, 0) << "1";
    FNLOG_ASSERT(DeviceWrite(0, 0) == old_write + 1, "check block category");

    old_write = DeviceWrite(0, 0);
    FNLog::SetDeviceConfig(FNLog::GetDefaultLogger(), 0, 0, FNLog::DEVICE_CFG_CATEGORY_FILTER, ((1ULL <<8) | (1ULL << 1)));
    LogInfoStream(0, 1, 0) << "1";
    LogInfoStream(0, 8, 0) << "1";
    FNLOG_ASSERT(DeviceWrite(0, 0) == old_write + 2, "check block category");

    old_write = DeviceWrite(0, 0);
    FNLog::SetDeviceConfig(FNLog::GetDefaultLogger(), 0, 0, FNLog::DEVICE_CFG_CATEGORY_FILTER, (1ULL << 63));
    LogInfoStream(0, 1, 0) << "1";
    LogInfoStream(0, 8, 0) << "1";
    FNLOG_ASSERT(DeviceWrite(0, 0) == old_write, "check block category");

    old_write = DeviceWrite(0, 0);
    FNLog::SetDeviceConfig(FNLog::GetDefaultLogger(), 0, 0, FNLog::DEVICE_CFG_CATEGORY_FILTER, (0));
    LogInfoStream(0, 1, 0) << "1";
    LogInfoStream(0, 8, 0) << "1";
    FNLOG_ASSERT(DeviceWrite(0, 0) == old_write + 2, "check block category");


    return 0;
}