
#define FN_LOG_MAX_CHANNEL_SIZE 4
#include "fn_log.h"

static const std::string default_config_text =
R"----(
 # default is sync write channel.  
 - channel: 0
    -device:0 # default screen , info and high priority will trans to one file.  
        disable: false
        priority: info 
        out_type: vm
    -device:1
        disable: false
        priority: debug
        out_type: screen

 - channel: 1
   -device:0
        disable: false
        out_type: screen
   -device:1
        disable: false 
        out_type: vm

 - channel: 2
   -device:0 
        disable: false 
        out_type: file
        file: "$PNAME_$YEAR"
        path: ../log/
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
    FNLog::TransmitChannel(FNLog::GetDefaultLogger(), 2, log.category_, log.identify_, log);
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
    for (int i = 0; i < 100; i+=2)
    {
        LogInfoStream(0, 0, 0) << "channel 0, log:" <<  i;
        LogInfoStream(1, 0, 0) << "channel 0, log:" << i+1;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    FNLOG_ASSERT(DeviceWrite(2, 0) == 100, "check write");

    return 0;
}