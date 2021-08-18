#define FN_LOG_MAX_CHANNEL_SIZE 4
#define FN_LOG_HOTUPDATE_INTERVEL 1

#include "fn_log.h"
#include <regex>

#ifdef WIN32 //VS
#define LOG_CONFIG_FILE "../log.yaml"
#else
#define LOG_CONFIG_FILE "./log.yaml"
#endif // VS


int main(int argc, char* argv[])
{
    int ret = FNLog::LoadAndStartDefaultLogger(LOG_CONFIG_FILE);
    if (ret != 0)
    {
        return ret;
    }

    int limit_second = 15;
    while (limit_second > 0)
    {
        if (limit_second % 4 == 0)
        {
            FNLog::FileHandler file;
            struct stat stt;
            file.open(LOG_CONFIG_FILE, "r+", stt);
            if (file.is_open())
            {
                std::string content = file.read_content();
                file.close();
                if (std::regex_search(content, std::regex("priority: trace", std::regex_constants::grep)))
                {
                    content = std::regex_replace(content, std::regex("priority: trace"), "priority: info");
                    LogAlarm() << "change log config file priority to info.";
                }
                else
                {
                    content = std::regex_replace(content, std::regex("priority: info"), "priority: trace");
                    LogAlarm() << "change log config file priority to trace.";
                }
                file.open(LOG_CONFIG_FILE, "w", stt);
                if (file.is_open())
                {
                    file.write(content.c_str(), content.length());
                    file.close();
                }
            }
        }
        LogDebug() << "default channel debug.";
        LogInfo() << "default channel info.";
        LogDebugStream(1, 0, 0) << "channel:1, category:0.";
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        limit_second--;
    }


    LogAlarm() << "finish";
    return 0;
}

