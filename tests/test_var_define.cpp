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

 - var: {output:screen, priority:debug}

 - channel: 0
    sync: sync
    - device:0
        disable: disable_device_0
        out_type: ${output}
        priority: ${priority}
    - device:1
        disable: disable_all_device
        out_type: ${output}
        priority: ${priority}

)----";

using namespace FNLog;
int main(int argc, char* argv[])
{
    int ret = FastStartDefaultLogger(test_text);
    if (ret != 0 )
    {
        return ret ;
    }

    LogInfo() << "now:" << LogTimestamp();

    FNLOG_ASSERT(FNLog::GetDefaultLogger().shm_->channels_[0].devices_[0].log_fields_[FNLog::DEVICE_LOG_TOTAL_WRITE_LINE] == 1, "");
    FNLOG_ASSERT(FNLog::GetDefaultLogger().shm_->channels_[0].devices_[1].log_fields_[FNLog::DEVICE_LOG_TOTAL_WRITE_LINE] == 2, "");


    LogAlarm() << "finish";

    return 0;
}
