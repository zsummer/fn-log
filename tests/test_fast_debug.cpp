#include "fn_log.h"

int main(int argc, char* argv[])
{
    int ret = FNLog::FastStartDebugLogger();
    if (ret != 0)
    {
        return ret;
    }

    LogAlarm() << "log init success. shm size:" 
        << sizeof(FNLog::SHMLogger) << ", logger size:" << sizeof(FNLog::Logger);
    //FNLog::SetAllChannelCategory(FNLog::GetDefaultLogger(), 100, 1);
    //FNLog::SetAllScreenCategory(FNLog::GetDefaultLogger(), 100, 1);
    LogTrace() << "now time:" << time(nullptr) << ";";
    LogDebug() << "now time:" << time(nullptr) << ";";
    LogInfo() << "now time:" << time(nullptr) << ";";
    LogWarn() << "now time:" << time(nullptr) << ";";
    LogError() << "now time:" << time(nullptr) << ";";
    LogAlarm() << "now time:" << time(nullptr) << ";";
    LogFatal() << "now time:" << time(nullptr) << ";";
    //FNLog::SetAllChannelCategory(FNLog::GetDefaultLogger(), 0, 0);
    //FNLog::SetAllScreenCategory(FNLog::GetDefaultLogger(), 0, 0);
    LOGT("now time:" << time(nullptr) << ";");
    LOGD("now time:" << time(nullptr) << ";");
    LOGI("now time:" << time(nullptr) << ";");
    LOGW("now time:" << time(nullptr) << ";");
    LOGE("now time:" << time(nullptr) << ";");
    LOGA("now time:" << time(nullptr) << ";");
    LOGF("now time:" << time(nullptr) << ";");

    LOGFMTT("now time:<%llu>", (unsigned long long)time(nullptr));
    LOGFMTD("now time:<%llu>", (unsigned long long)time(nullptr));
    LOGFMTI("now time:<%llu>", (unsigned long long)time(nullptr));
    LOGFMTW("now time:<%llu>", (unsigned long long)time(nullptr));
    LOGFMTE("now time:<%llu>", (unsigned long long)time(nullptr));
    LOGFMTA("now time:<%llu>", (unsigned long long)time(nullptr));
    LOGFMTF("now time:<%llu>", (unsigned long long)time(nullptr));


    PackTrace("now time:", time(nullptr), ";");
    PackDebug("now time:", time(nullptr), ";");
    PackInfo("now time:", time(nullptr), ";");
    PackWarn("now time:", time(nullptr), ";");
    PackError("now time:", time(nullptr), ";");
    PackAlarm("now time:", time(nullptr), ";");
    PackFatal("now time:", time(nullptr), ";");

    LogInfo() << "random1:" << ((unsigned long long)rand() * rand()) << FNLog::LogBlankAlign<110>() << "random2:" << ((unsigned long long)rand() * rand()) << ";";
    LogInfo() << "random1:" << ((unsigned long long)rand() * rand()) << FNLog::LogBlankAlign<110>() << "random2:" << ((unsigned long long)rand() * rand()) << ";";
    LogInfo() << "random1:" << ((unsigned long long)rand() * rand()) << FNLog::LogBlankAlign<110>() << "random2:" << ((unsigned long long)rand() * rand()) << ";";
    LogInfo() << "random1:" << ((unsigned long long)rand() * rand()) << FNLog::LogBlankAlign<110>() << "random2:" << ((unsigned long long)rand() * rand()) << ";";
    LogInfo() << "random1:" << ((unsigned long long)rand() * rand()) << FNLog::LogBlankAlign<110>() << "random2:" << ((unsigned long long)rand() * rand()) << ";";
    LogInfo() << "random1:" << ((unsigned long long)rand() * rand()) << FNLog::LogBlankAlign<110>() << "random2:" << ((unsigned long long)rand() * rand()) << ";";
    LogInfo() << "random1:" << ((unsigned long long)rand() * rand()) << FNLog::LogBlankAlign<110>() << "random2:" << ((unsigned long long)rand() * rand()) << ";";

    LogAlarm() << "finish";


    return 0;
}