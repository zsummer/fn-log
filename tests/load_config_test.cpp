#include "fn_log.h"
#include "fn_load.h"



int main(int argc, char* argv[])
{
    FNLog::GuardLogger gl(FNLog::GetDefaultLogger());
    FNLog::InitFromYMALFile("./log.yaml", FNLog::GetDefaultLogger());
    int ret = FNLog::StartDefaultLogger();
    if (ret != 0 || FNLog::GetDefaultLogger().last_error_ != 0)
    {
        return ret || FNLog::GetDefaultLogger().last_error_;
    }
    int limit_second = 50;
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

