#include "fn_log.h"
#define FNLOG_ASSERT(expr, prefix) \
{ \
    if (expr) \
    { \
        LogInfo() << "test " << prefix << " success."; \
    } \
    else \
    {   \
        LogError() << "test " << prefix << " failed."; \
        return __LINE__ * -1; \
    } \
}

static const std::string test_text =
R"----(
 # info log print screen and all write file

 - define: "disable_all_device:false"
 - define: {disable_device_0:disable_all_device}

 - var: {output1:screen, priority1:trace}

 - channel: 0
    sync: sync
    - device:0
        disable: disable_device_0
        out_type: ${output1}
        priority: ${priority1}
    - device:1
        disable: disable_all_device
        out_type: ${output1}
        priority: ${priority1}

 - var: {output2:screen, priority2:trace}
 - var:{ai:0, ai_debug:1, move:2, move_debug:3, spell:4}
 - channel: 1
    sync: sync
    - device:0
        disable: disable_device_0
        out_type: $output2
        priority: $priority2  #$ai $ai_debug
    - device:1
        disable: disable_all_device
        out_type: $output2
        priority: $priority2


)----";

using namespace FNLog;
int main(int argc, char* argv[])
{
    int ret = FastStartDefaultLogger(test_text);
    if (ret != 0 )
    {
        return ret ;
    }

    LogInfo() << "channel[0] now:" << LogTimestamp();

    FNLOG_ASSERT(FNLog::GetDefaultLogger().shm_->channels_[0].devices_[0].log_fields_[FNLog::DEVICE_LOG_TOTAL_WRITE_LINE] == 1, "");
    FNLOG_ASSERT(FNLog::GetDefaultLogger().shm_->channels_[0].devices_[1].log_fields_[FNLog::DEVICE_LOG_TOTAL_WRITE_LINE] == 2, "");


    LogInfoStream(1, 0, 0) << "channel[1] now:" << LogTimestamp();
    FNLOG_ASSERT(FNLog::GetDefaultLogger().shm_->channels_[1].devices_[0].log_fields_[FNLog::DEVICE_LOG_TOTAL_WRITE_LINE] == 1, "");
    FNLOG_ASSERT(FNLog::GetDefaultLogger().shm_->channels_[1].devices_[1].log_fields_[FNLog::DEVICE_LOG_TOTAL_WRITE_LINE] == 1, "");
    LogAlarm() << "finish";

    return 0;
}
