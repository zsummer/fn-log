
#define FN_LOG_MAX_LOG_SIZE 1000
#define FN_LOG_MAX_LOG_QUEUE_SIZE 10000
#define FN_LOG_HOTUPDATE_INTERVEL 3
#define FN_LOG_MAX_CHANNEL_SIZE 2
#include "fn_log.h"
#include <signal.h>
#include <chrono>
#include <thread>


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



)----";



#define FNLOG_ASSERT(expr, prefix) \
{ \
    if (expr) \
    { \
        LogInfoStream(0, 0, 0) << "test " << prefix << " success."; \
    } \
    else \
    {   \
        LogErrorStream(0, 0, 0) << "test " << prefix << " failed."; \
        return __LINE__ * -1; \
    } \
}


void Stop(int signo)
{
    printf("%s", "oops! stop!!!\n");
    _exit(0);
}



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

    LogInfo() << "hotupdate begin ";


    FNLog::Logger& logger = FNLog::GetDefaultLogger();
    FNLog::Channel& channel = logger.shm_->channels_[0];

    for (int i = 0; i < 40; i++)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    long long old_checks = FNLog::AtomicLoadChannelLog(channel, FNLog::CHANNEL_LOG_HOTUPDATE_CHECK);
    long long old_changes = FNLog::AtomicLoadChannelLog(channel, FNLog::CHANNEL_LOG_HOTUPDATE_CHANGE);
    LogInfo() << "hotupdate checks:" << old_checks << ", changes:" << old_changes ;
    FNLOG_ASSERT(old_checks > 0, "");
    FNLOG_ASSERT(old_checks <= 4, ""); //not key decision. app env has some pollution when in mac-OS @github action 
    FNLOG_ASSERT(old_changes == 0, "");

    

    LogInfo() << "apend blank to " << path  << " ... ";
    file.open(path.c_str(), "ab", s);
    if (!file.is_open())
    {
        printf("%s", "write file error");
        return 1;
    }
    file.write(" ", 1);
    file.close();


    LogInfo() << "apended blank to " << path;
    
    for (int i = 0; i < 40; i++)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    long long checks = FNLog::AtomicLoadChannelLog(channel, FNLog::CHANNEL_LOG_HOTUPDATE_CHECK) - old_checks;
    long long changes = FNLog::AtomicLoadChannelLog(channel, FNLog::CHANNEL_LOG_HOTUPDATE_CHANGE) - old_changes;
    LogInfo() << "check state: inc checks:" << checks <<", inc changes:" << changes;
    FNLOG_ASSERT(checks > 0, "");
    FNLOG_ASSERT(checks <= 2, "");
    FNLOG_ASSERT(changes == 1, "");
    LogInfo() << "hotupdate checks:  ok";

    LogAlarm() << "all thread exit";
    return 0;
}

