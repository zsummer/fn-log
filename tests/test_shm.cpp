
#define FN_LOG_USE_SHM 1

#include "fn_log.h"

using namespace FNLog;


int main(int argc, char* argv[])
{
    int ret = FNLog::FastStartDefaultLogger();
    if (ret != 0)
    {
        return ret;
    }

    LogAlarm() << "log init success:" << FNLog::GetChannelLog(FNLog::GetDefaultLogger(), 0, FNLog::CHANNEL_LOG_PUSH);
    
    do
    {
        long long proc_count = FNLog::GetChannelLog(FNLog::GetDefaultLogger(), 0, FNLog::CHANNEL_LOG_PUSH);
        if (proc_count % 100000 == 0)
        {
            auto ss(LogInfo());
            ss << "now write:" << proc_count;
            if (proc_count == 800000)
            {
                _exit(0);
            }
            ss << " line.";
        }
        else
        {
            LogDebug() << "now write:" << proc_count << " line.";
        }
        if (proc_count > 1600000)
        {
            LogAlarm() << "now finish:" << proc_count << " line.";
            return 0;
        }
    } while (true);

    LogAlarm() << "finish:" << FNLog::GetChannelLog(FNLog::GetDefaultLogger(), 0, FNLog::CHANNEL_LOG_PUSH);

    return 0;
}