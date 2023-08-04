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

 - define: "define1:$ai"
 - define: {define2:define1, define3__:$ai_trace, }

 - var: {spell:1}

 - channel: 0
    sync: sync
    - device:0
        disable: false
        out_type: screen
        priority: debug
    - device:1
        disable: true
        out_type: file
        priority: debug
        category: ${spell}

 - var: {ai:3, ai_trace:4}

 - channel: 1
    sync: sync
    - device:0
        disable: true
        out_type: file
        priority: debug
        category_wlist: define2, define3__  # define2->define1->$ai->3,  define3__->${ai_trace}->4

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

    long long cat01 = FNLog::GetDefaultLogger().shm_->channels_[0].devices_[1].config_fields_[FNLog::DEVICE_CFG_CATEGORY];
    FNLOG_ASSERT(cat01 == 1, "");

    long long cat10 = FNLog::GetDefaultLogger().shm_->channels_[1].devices_[0].config_fields_[FNLog::DEVICE_CFG_CATEGORY_MASK];
    FNLOG_ASSERT(cat10 == ((1 << 3) | (1 <<4)), "");


    LogAlarm() << "finish";

    return 0;
}
