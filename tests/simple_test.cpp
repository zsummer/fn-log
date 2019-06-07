#include "fn_log.h"

int main(int argc, char* argv[])
{
    int ret = FNLog::FastStartDefaultLogger();
    if (ret != 0)
    {
        return ret;
    }

    LogAlarm() << "log init success";

    LogDebug() << "now time:" << time(nullptr) << ";";
    LOGD("now time:" << time(nullptr) << ";");
    LogDebugPack(0, 0, "now time:", time(nullptr), ";");
    
    LogAlarm() << "finish";


    return 0;
}