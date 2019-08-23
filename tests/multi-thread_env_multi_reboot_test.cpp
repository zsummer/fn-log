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

int total_loop_ = 0;
void thread_proc(int index)
{
    LogInfo() << "thread:<" << index << "> begin.";
    FNLog::Logger& logger = FNLog::GetDefaultLogger();
    while (total_loop_ < 1000000)
    {
        if (logger.logger_state_ == FNLog::LOGGER_STATE_RUNNING)
        {
            total_loop_++;
        }
        LogDebug().write_buffer("rrrrrrrrrrrrrrrrrrrradfads33333333333333rrd",
                                sizeof("rrrrrrrrrrrrrrrrrrrradfads33333333333333rrd") - 1);
        if (total_loop_%1000 == 0)
        {
            LogInfo() << "already loop:" << total_loop_;
            if (logger.logger_state_ == FNLog::LOGGER_STATE_RUNNING)
            {
                FNLog::StopAndCleanLogger(FNLog::GetDefaultLogger());
            }
        }
        if (logger.logger_state_ == FNLog::LOGGER_STATE_UNINIT)
        {
            FNLog::FastStartSimpleLogger();
        }
    }
    LogInfo() << "thread:<" << index << "> end.";
}

const int WRITE_THREAD_COUNT = 6;
std::thread g_multi_proc[WRITE_THREAD_COUNT];

void Stop(int signo)
{
    printf("oops! stop!!!\n");
    _exit(0);
}


int main(int argc, char* argv[])
{
    signal(SIGINT, Stop);

    int test_thread = 5;
    for (int i = 0; i < test_thread; i++)
    {
        g_multi_proc[i] = std::thread(thread_proc, i);
    }

    for (int i = 0; i <= test_thread; i++)
    {
        if (g_multi_proc[i].joinable())
        {
            g_multi_proc[i].join();
        }
    }
    return 0;
}

