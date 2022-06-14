#define FN_LOG_MAX_LOG_SIZE 1000
#define FN_LOG_MAX_LOG_QUEUE_SIZE 100000

#include "fn_log.h"
#include <signal.h>

static const std::string default_config_text =
R"----(
 # default is mult-thread async write channel.  
 # the first device is write rollback file  
 # the second device is print to screen.  
 - channel: 0
    sync: null
    -device: 0
        disable: false
        out_type: file
        file: "$PNAME"
        rollback: 1
        limit_size: 100 m #only support M byte
    -device:1
        disable: false
        out_type: screen
        priority: info
 - channel: 1
    sync: sync
    -device: 0
        disable: false
        out_type: file
        file: "$PNAME_SYNC"
        rollback: 1
        limit_size: 100 m #only support M byte
    -device:1
        disable: false
        out_type: screen
        priority: info
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
    int ret = FNLog::FastStartDefaultLogger(default_config_text);
    if (ret != 0)
    {
        return ret;
    }

    FNLog::Logger& logger = FNLog::GetDefaultLogger();

    int limit_second = 0;
    int thread_id = 0;
    g_multi_proc[thread_id] = std::thread(thread_proc, thread_id);

    do
    {
        long long last_writed = AtomicLoadL(logger.shm_->channels_[0], FNLog::CHANNEL_LOG_PROCESSED);
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        long long now_writed = AtomicLoadL(logger.shm_->channels_[0], FNLog::CHANNEL_LOG_PROCESSED);
        LogInfo() << "now thread:" << thread_id+1 << ": writed:" << now_writed - last_writed ;
        
        if (limit_second/3 > thread_id && thread_id+1 < WRITE_THREAD_COUNT)
        {
            thread_id++;
            LogInfo() << "add new thread:" << thread_id;
            g_multi_proc[thread_id] = std::thread(thread_proc, thread_id);
        }
        limit_second++;
    } while (limit_second < 15);

    LogAlarm() << "finish";
    state = END;
    for (int i = 0; i <= thread_id; i++)
    {
        if (g_multi_proc[i].joinable())
        {
            g_multi_proc[i].join();
        }
    }
    return 0;
}

