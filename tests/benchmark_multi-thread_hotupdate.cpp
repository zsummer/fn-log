
#define FN_LOG_MAX_LOG_SIZE 1000
#define FN_LOG_MAX_LOG_QUEUE_SIZE 100000
#define FN_LOG_HOTUPDATE_INTERVEL 2
#define FN_LOG_MAX_CHANNEL_SIZE 5
#include "fn_log.h"
#include <signal.h>


static const std::string default_config_text =
R"----(
 # default channel 0
   # write full log to pname.log 
   # write info log to pname_info.log 
   # view  info log to screen 
 # sync channel 1 
   # write full log to pname.log
   # write info log to pname_info.log
   # view  info log to screen 
 - define: BASE_CHANNEL 0
 - define: SECOND_CHANNEL 1
 - define: "DEFAULT_ROLL 4"
 - define: "DEFAULT_LIMIT 100 m"
 hotupdate: true
 - channel: BASE_CHANNEL
    sync: async
    -device: 0
        disable: false
        out_type: file
        file: "$PNAME_base"
        rollback: DEFAULT_ROLL
        limit_size: 100 m #only support M byte
    -device: 1
        disable: false
        out_type: screen
        priority: info
 - channel: SECOND_CHANNEL
    sync: async
    -device: 0
        disable: false
        out_type: file
        file: "$PNAME_second"
        rollback: DEFAULT_ROLL
        limit_size: 100 m #only support M byte
 - channel: 2
    sync: async
    -device: 0
        disable: false
        out_type: file
        file: "$PNAME_3"
        rollback: 1
        limit_size: 10 m #only support M byte
 - channel: 3
    sync: async
    -device: 0
        disable: false
        out_type: file
        file: "$PNAME_3"
        rollback: 1
        limit_size: 10 m #only support M byte


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

        LogDebug().write_buffer("rrrrrrrrrrrrrrrrrrrradfads33333333333333rrd",
            sizeof("rrrrrrrrrrrrrrrrrrrradfads33333333333333rrd") - 1)
            << -23 << ": " << 32.2223 << (void*) nullptr;
        LogDebugStream(1, 0, 0).write_buffer("rrrrrrrrrrrrrrrrrrrradfads33333333333333rrd",
            sizeof("rrrrrrrrrrrrrrrrrrrradfads33333333333333rrd") - 1)
            << -23 << ": " << 32.2223 << (void*) nullptr;

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


struct MyStruct
{
    int* a[3];
};
//constexpr static int s = sizeof(MyStruct);

int main(int argc, char* argv[])
{
    signal(SIGINT, Stop);
    std::string path = "./hotupdate.yaml";
    FNLog::FileHandler::remove_file(path);
    FNLog::FileHandler file;
    struct stat s;
    file.open(path.c_str(), "rb", s);
    if (file.is_open())
    {
        printf("%s", "has error");
        return 1;
    }
    file.close();

    file.open(path.c_str(), "wb", s);
    if (!file.is_open())
    {
        printf("%s", "write file error");
        return 1;
    }
    file.write(default_config_text.c_str(), default_config_text.length());
    file.close();
    //FNLog::FileHandler::write();
    int ret = FNLog::LoadAndStartDefaultLogger(path);
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
        long long last_writed = FNLog::AtomicLoadChannelLog(logger.shm_->channels_[0], FNLog::CHANNEL_LOG_PROCESSED);
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        long long now_writed = FNLog::AtomicLoadChannelLog(logger.shm_->channels_[0], FNLog::CHANNEL_LOG_PROCESSED);
        LogInfo() << "now thread:" << thread_id + 1 << ": writed:" << now_writed - last_writed;


        file.open(path.c_str(), "wb", s);
        if (!file.is_open())
        {
            printf("%s", "write file error");
            return 1;
        }
        file.write(default_config_text.c_str(), default_config_text.length());
        file.close();

        if (limit_second / 3 > thread_id && thread_id + 1 < WRITE_THREAD_COUNT)
        {
            thread_id++;
            LogInfo() << "add new thread:" << thread_id;
            g_multi_proc[thread_id] = std::thread(thread_proc, thread_id);
        }
        limit_second++;
    } while (limit_second < 15);

    LogAlarm() << "finish";

    for (int channel_id = 0; channel_id < logger.shm_->channel_size_; channel_id++)
    {
        FNLog::Channel& channel = logger.shm_->channels_[channel_id];
        for (int field = 0; field < FNLog::CHANNEL_LOG_MAX_ID; field++)
        {
            if (FNLog::AtomicLoadChannelLog(channel, field) <= 0)
            {
                continue;
            }
            LogInfoStream(0, 1, 0) << "channel[" << channel_id << "] log field[" << field << "]:" << FNLog::AtomicLoadChannelLog(channel, field);
        }
    }

    state = END;
    for (int i = 0; i <= thread_id; i++)
    {
        if (g_multi_proc[i].joinable())
        {
            g_multi_proc[i].join();
        }
    }
    LogAlarm() << "all thread exit";
    return 0;
}

