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

int total_loop_ = 80000;
int cur_loop_ = 0;
std::atomic<int> stop_cnt = 0.0;
std::atomic<int> stop_ok = 0.0;
std::atomic<int> start_cnt = 0.0;
std::atomic<int> start_ok = 0.0;
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
        if (cur_loop_%200 == 0)
        {
            LogInfo() << "already loop:" << cur_loop_;
            if (logger.logger_state_ == FNLog::LOGGER_STATE_RUNNING)
            {
                int ret = FNLog::StopLogger(FNLog::GetDefaultLogger());
                if (ret == FNLog::E_SUCCESS)
                {
                    stop_ok += 1;
                }
                stop_cnt += 1;
            }
        }
        if (logger.logger_state_ == FNLog::LOGGER_STATE_UNINIT)
        {
            int ret = FNLog::FastStartDefaultLogger();
            if (ret == FNLog::E_SUCCESS)
            {
                start_ok += 1;
            }
            start_cnt += 1;
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

#ifndef WIN32
#define dup _dup
#define dup2 _dup2
#define fileno _fileno
#endif // WIN32


int main(int argc, char* argv[])
{
    signal(SIGINT, Stop);

    FNLog::FastStartDefaultLogger();
    LogInfo() << "begin test...";
    std::this_thread::sleep_for(std::chrono::milliseconds(500));


    int fd = _dup(_fileno(stdout));
    dup2(0, 1);
    //freopen("./printf.log", "a+", stdout);

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

    dup2(fd, 1);

    FNLog::FastStartDefaultLogger();

    LogInfo() << "start ok/total:" << start_ok << "/" << start_cnt << ", start ok rate:" << start_ok * 100 / start_cnt << "%";
    LogInfo() << "stop ok/total:" << stop_ok << "/" << stop_cnt << ", stop ok rate:" << stop_ok * 100 / stop_cnt << "%";
    if (start_ok <= 0 || stop_ok <= 0)
    {
        return -1;
    }
    return 0;
}

