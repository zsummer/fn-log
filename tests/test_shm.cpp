
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

    LogInfo() << "log init success:" << FNLog::GetChannelLog(FNLog::GetDefaultLogger(), 0, FNLog::CHANNEL_LOG_PUSH);
    
    do
    {
        long long proc_count = FNLog::GetChannelLog(FNLog::GetDefaultLogger(), 0, FNLog::CHANNEL_LOG_PUSH);
        if (proc_count % 100000 == 0)
        {
            LogInfo() << "now write:" << proc_count;
        }
        else if (proc_count == 800001)
        {
            LogInfo() << "now has dump:" << proc_count;
            _exit(0);
        }
        else
        {
            LogDebug() << "now write:" << proc_count;
        }
        if (proc_count > 1600000)
        {
            LogInfo() << "now finish:" << proc_count;
            return 0;
        }
    } while (true);

    LogInfo() << "finish:" << FNLog::GetChannelLog(FNLog::GetDefaultLogger(), 0, FNLog::CHANNEL_LOG_PUSH);

    return 0;
}