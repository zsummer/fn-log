#include "fn_log.h"
#include <signal.h>



static const std::string example_config_text =
R"----(
 # info and high priority log print to screen  
 - channel: 0
    sync: null
    -device:0
        disable: false
        out_type: screen
        priority: info
        
 - channel: 1

)----";



enum State
{
    WAIT,
    WRITE,
    END
};

State state = WRITE;

void thread_proc(int index)
{
    LogInfo() << "thread:<" << index << "> begin.";
    while (state != END)
    {
        if (state == WRITE)
        {
            LogDebug().write_buffer("rrrrrrrrrrrrrrrrrrrradfads33333333333333rrd",
                sizeof("rrrrrrrrrrrrrrrrrrrradfads33333333333333rrd") - 1)
                << -23 << ": " << 32.2223 << (void*) nullptr;
        }
        else
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
    LogInfo() << "thread:<" << index << "> end.";
}

const int WRITE_THREAD_COUNT = 6;
std::thread g_multi_proc[WRITE_THREAD_COUNT];

void Stop(int signo)
{
    printf("%s", "oops! stop!!!\n");
    _exit(0);
}


int main(int argc, char* argv[])
{
    signal(SIGINT, Stop);
    int ret = FNLog::FastStartSimpleLogger();
    if (ret != 0)
    {
        return ret;
    }

    FNLog::Logger& logger = FNLog::GetDefaultLogger();

    int limit_second = 12;
    for (int i = 0; i < WRITE_THREAD_COUNT; i++)
    {
        g_multi_proc[i] = std::thread(thread_proc, i);
    }

    for (int i = 0; i < limit_second; i++)
    {
        long long last_writed = logger.channels_[0].log_fields_[FNLog::CHANNEL_LOG_PROCESSED].num_;
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        long long now_writed = logger.channels_[0].log_fields_[FNLog::CHANNEL_LOG_PROCESSED].num_;
        LogInfo() << "writed:" << now_writed - last_writed << ", cache hit:"
            << (double)logger.channels_[0].log_fields_[FNLog::CHANNEL_LOG_ALLOC_CACHE].num_
            / logger.channels_[0].log_fields_[FNLog::CHANNEL_LOG_ALLOC_CALL].num_ * 100.0;

        if (i%3 == 0)
        {
            int ret = FNLog::StopAndCleanLogger(logger);
            if (ret != 0)
            {
                return -1;
            }
            ret = FNLog::FastStartSimpleLogger();
            if (ret != 0)
            {
                return -2;
            }
        }
    }

    LogAlarm() << "finish";
    state = END;
    for (int i = 0; i <= WRITE_THREAD_COUNT; i++)
    {
        if (g_multi_proc[i].joinable())
        {
            g_multi_proc[i].join();
        }
    }
    return 0;
}

