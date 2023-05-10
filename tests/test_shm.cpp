
#include "fn_log.h"

using namespace FNLog;


int main(int argc, char* argv[])
{
    static const std::string default_config_text =
        R"----(
 # default channel 0
   # write full log to pname.log 
   # write error log to pname_error.log 
   # view  info log to screen 
 # sync channel 1 
   # write full log to pname.log
   # write info log to pname_info.log
   # view  info log to screen 
 shm_key: 0x998822
 - channel: 0
    sync: async
    -device: 0
        disable: false
        out_type: file
        file: "$PNAME"
        rollback: 4
        limit_size: 100 m #only support M byte
    -device: 1
        disable: false
        out_type: file
        priority: error
        file: "$PNAME_error"
        rollback: 4
        limit_size: 100 m #only support M byte
    -device:2
        disable: false
        out_type: screen
        priority: info
 - channel: 1
    sync: sync
    -device: 0
        disable: false
        out_type: file
        file: "$PNAME_sync"
        rollback: 4
        limit_size: 100 m #only support M byte
    -device: 1
        disable: false
        out_type: file
        priority: info
        file: "$PNAME_sync_info"
        rollback: 4
        limit_size: 100 m #only support M byte
    -device:2
        disable: false
        out_type: screen
        priority: info 
)----";
    int ret = FastStartDefaultLogger(default_config_text);
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
            auto ss(std::move(LogInfo()));
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