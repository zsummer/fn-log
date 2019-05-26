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
        LOGD() << "default channel.";
        LOGCD(1, 0) << "channel:1, cls:0.";
        LOGCD(2, 0) << "channel:2, cls:0.";
        LOGCD(3, 0) << "channel:3, cls:0.";
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        limit_second--;
    }


    LOGA() << "finish";
    return 0;
}

