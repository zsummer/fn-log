#include "fn_log.h"

using namespace FNLog;

static const std::string example_config_text =
R"----(
 # 压测配表  
 # 0通道为屏显输出和UDP输出 
 - channel: 0
    sync: null
    filter_level: trace
    filter_cls_begin: 0
    filter_cls_count: 0
    -device: 0
        disable: false
        out_type: screen
    -device:1
        disable: false
        out_type: udp
        udp_addr: 127.0.0.1_9909


)----";

int main(int argc, char* argv[])
{
#ifdef WIN32

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        printf("WSAStartup 无法初始化！");
        return -1;
    }
#endif

    Logger& logger = GetDefaultLogger();
    GuardLogger gl(logger);
    InitFromYMAL(example_config_text, "", logger);
    int ret = FNLog::StartDefaultLogger();
    if (ret != 0 || FNLog::GetDefaultLogger().last_error_ != 0)
    {
        return ret || FNLog::GetDefaultLogger().last_error_;
    }

    LOGD() << "log init success";
    while (true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        LOGD() << "now time:" << (long long)time(nullptr) << ";";
    }

    LOGCA(0, 1) << "finish";

    return 0;
}