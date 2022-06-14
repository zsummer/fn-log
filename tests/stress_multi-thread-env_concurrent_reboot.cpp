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

int total_loop_ = 800000;
int cur_loop_ = 0;
void thread_proc(int index)
{
    LogInfo() << "thread:<" << index << "> begin.";
    FNLog::Logger& logger = FNLog::GetDefaultLogger();
    while (cur_loop_ < total_loop_)
    {
        if (logger.logger_state_ == FNLog::LOGGER_STATE_RUNNING)
        {
            cur_loop_++;
        }
        LogDebug().write_buffer("rrrrrrrrrrrrrrrrrrrradfads33333333333333rrd",
                                sizeof("rrrrrrrrrrrrrrrrrrrradfads33333333333333rrd") - 1);
        if (cur_loop_%1000 == 0)
        {
            LogInfo() << "already loop:" << cur_loop_;
            if (logger.logger_state_ == FNLog::LOGGER_STATE_RUNNING)
            {
                FNLog::StopLogger(FNLog::GetDefaultLogger());
            }
        }
        if (logger.logger_state_ == FNLog::LOGGER_STATE_UNINIT)
        {
            FNLog::FastStartDefaultLogger();
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
    if (argc > 1)
    {
        total_loop_ = atoi(argv[1]);
    }
    for (int i = 0; i < WRITE_THREAD_COUNT; i++)
    {
        g_multi_proc[i] = std::thread(thread_proc, i);
    }

    for (int i = 0; i < WRITE_THREAD_COUNT; i++)
    {
        if (g_multi_proc[i].joinable())
        {
            g_multi_proc[i].join();
        }
    }
    return 0;
}

