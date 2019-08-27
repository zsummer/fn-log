#include "fn_log.h"



int main(int argc, char* argv[])
{
    int ret = FNLog::LoadAndStartDefaultLogger("./log.yaml");
    if (ret != 0)
    {
        return ret;
    }
    
    int limit_second = 10;
    while (limit_second > 0)
    {
        LogDebug() << "default channel.";
        LogDebugStream(1, 0) << "channel:1, category:0.";
        LogDebugStream(2, 0) << "channel:2, category:0.";
        LogDebugStream(3, 0) << "channel:3, category:0.";
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        limit_second--;
    }


    LogAlarm() << "finish";
    return 0;
}

