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

    FNLog::Logger& logger = FNLog::GetDefaultLogger();

    unsigned int total_count = 0;

        total_count = 0;
        unsigned long long active = 0;
        do
        {
            char buf[30];
            
            FNLog::write_dec_unsafe<2>(buf, (long long)rand());
            active += buf[3];
            if (total_count %10000000 == 0)
            {
                static long long last = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
                long long now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
                if (total_count > 0)
                {
                    LogAlarm() << " test " << 10000000*1000ULL / (now - last) << " line/sec. cache hit:"
                        << (double)logger.channels_[0].log_fields_[FNLog::CHANNEL_LOG_ALLOC_CACHE].num_ 
                                / logger.channels_[0].log_fields_[FNLog::CHANNEL_LOG_ALLOC_CALL].num_ * 100 << "%" << active;
                    last = now;
                    break;
                }
            }
        } while (++total_count);


    LogAlarm() << "finish";
    return 0;
}

