#include "fn_log.h"

int main(int argc, char* argv[])
{
    int ret = FNLog::FastStartSimpleLogger();
    //FNLog::GetDefaultLogger().channels_[0].devices_[1].config_fields_[FNLog::DEVICE_CFG_PRIORITY].num_ = FNLog::PRIORITY_TRACE;
    if (ret != 0)
    {
        return ret;
    }

    LOGA("log init success");

    LOGD("now time:" << time(nullptr) << ";");

    LOGFMTD("test:<%d>", 100);
    
    LOGA("finish log4z.");

    return 0;
}