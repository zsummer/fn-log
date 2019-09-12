#include "fn_log.h"




static const std::string example_config_text =
R"----(
 - channel: 0
    sync: sync
    -device: 0
        disable: false
        out_type: screen
        priority: info

)----";

int main(int argc, char *argv[])
{
    int ret = FNLog::FastStartDefaultLogger(example_config_text);
    if (ret != 0)
    {
        return ret;
    }

    unsigned int total_count = 0;

    total_count = 0;
    do
    {
        LogDebug().write_buffer("rrrrrrrrrrrrrrrrrrrradfads33333333333333rrd",
                            sizeof("rrrrrrrrrrrrrrrrrrrradfads33333333333333rrd") - 1)
            << -23 << ": " << 32.2223 << (void*) nullptr;
            
        if (total_count %10000000 == 0)
        {
            static long long last = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
            long long now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
            if (total_count > 0)
            {
                LogAlarm() << " test " << 10000000*1000ULL / (now - last) << " line/sec.";
                last = now;
                break;
            }
        }
    } while (++total_count);


    LogAlarm() << "finish";
    return 0;
}

