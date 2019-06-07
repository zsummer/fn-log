#include "fn_log.h"

using namespace FNLog;

static const std::string example_config_text =
R"----(
 # desc
 - channel: 0
    sync: null
    priority: trace
    category: 0
    category_extend: 0
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
        printf("WSAStartup �޷���ʼ����");
        return -1;
    }
#endif

    int ret = FNLog::FastStartDefaultLogger(example_config_text);
    if (ret != 0)
    {
        return ret;
    }

    LogDebug() << "log init success";
    while (true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        LogDebug() << "now time:" << (long long)time(nullptr) << ";";
    }

    LogAlarmStream(0, 1) << "finish";

    return 0;
}