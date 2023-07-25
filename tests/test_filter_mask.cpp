#include "fn_log.h"

static const std::string config_text_249 =
R"----(
 # default is sync write channel.  
 - channel: 0
    sync: sync
    -device:0
        disable: false
        priority: debug
        out_type: file
        file: $PNAME_debug
        category_mask: 249
 - channel: 1
   sync: sync
   -device:0
        disable: false
        out_type: screen
)----";

static const std::string config_text_249_list =
R"----(
 # default is sync write channel.  
 - channel: 0
    sync: sync
    -device:0
        disable: false
        priority: debug
        out_type: file
        file: $PNAME_debug
        category_filter: "0, 3, 4,5,6,7"
 - channel: 1
   sync: sync
   -device:0
        disable: false
        out_type: screen
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





using namespace FNLog;
int main(int argc, char* argv[])
{
    FNLOG_ASSERT(FNLog::FastStartDefaultLogger(config_text_249_list) == 0, "start");
    FNLOG_ASSERT(FNLog::GetDefaultLogger().shm_->channels_[0].devices_[0].config_fields_[FNLog::DEVICE_CFG_CATEGORY_FILTER] == 249, "");

    FNLog::StopLogger(FNLog::GetDefaultLogger());
    FNLOG_ASSERT(FNLog::FastStartDefaultLogger(config_text_249) == 0, "start");
    FNLOG_ASSERT(FNLog::GetDefaultLogger().shm_->channels_[0].devices_[0].config_fields_[FNLog::DEVICE_CFG_CATEGORY_FILTER] == 249, "");
    LogInfoStream(0, 0, 0); //ok
    FNLOG_ASSERT(FNLog::GetDefaultLogger().shm_->channels_[0].devices_[0].log_fields_[FNLog::DEVICE_LOG_TOTAL_WRITE_LINE] == 1, "");
    LogInfoStream(0, 1, 0); //block
    FNLOG_ASSERT(FNLog::GetDefaultLogger().shm_->channels_[0].devices_[0].log_fields_[FNLog::DEVICE_LOG_TOTAL_WRITE_LINE] == 1, "");

    LogInfoStream(0, 2, 0); //block
    FNLOG_ASSERT(FNLog::GetDefaultLogger().shm_->channels_[0].devices_[0].log_fields_[FNLog::DEVICE_LOG_TOTAL_WRITE_LINE] == 1, "");

    LogInfoStream(0, 3, 0); //ok
    FNLOG_ASSERT(FNLog::GetDefaultLogger().shm_->channels_[0].devices_[0].log_fields_[FNLog::DEVICE_LOG_TOTAL_WRITE_LINE] == 2, "");


    return 0;
}