

#define FN_LOG_MAX_LOG_SIZE 10000
#define FN_LOG_MAX_LOG_QUEUE_SIZE 10000
#include "fn_log.h"

using namespace FNLog;

static const std::string example_config_text =
R"----(
 # desc
 - channel: 0
    -device: 0
        disable: false
        priority: info
        out_type: screen
    -device:1
        disable: false
        out_type: udp
        priority: debug
        udp_addr: localhost:9909
 - channel: 1
    -device:0
        disable: false
        in_type: udp
        out_type: file
        udp_addr: localhost:9909
        out_type: file
        path: "./"
        file: "$PNAME_receiver"
        rollback: 4
        limit_size: 100 m #only support M byte

)----";


#define Now()  std::chrono::duration<double>(std::chrono::system_clock::now().time_since_epoch()).count()
int main(int argc, char* argv[])
{
#ifdef WIN32

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        printf("%s", "WSAStartup error.\n");
        return -1;
    }
#endif

    int ret = FNLog::FastStartDefaultLogger(example_config_text);
    if (ret != 0)
    {
        return ret;
    }

    LogInfo() << "log init success";

    //to listen udp
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    int total_count = 0;

    constexpr int max_count = 50000;

    double begin_s = Now();

    while (total_count++ < max_count)
    {
        LogDebugStream(0, 0, 0) << "udpdata.1233330000000000000000000000000000000000000000000000000000000";
    }
    double end_1s = Now();
    double end_2s = Now();

    FNLog::Channel& channel0 = FNLog::GetDefaultLogger().shm_->channels_[0];
    FNLog::Channel& channel1 = FNLog::GetDefaultLogger().shm_->channels_[1];


    long long lines =  FNLog::AtomicLoadDeviceLog(channel1, 0, FNLog::DEVICE_LOG_TOTAL_WRITE_LINE);
    do
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        long long news = FNLog::AtomicLoadDeviceLog(channel1, 0, FNLog::DEVICE_LOG_TOTAL_WRITE_LINE);

        if (news == lines)
        {
            break;
        }
        lines = news;
        end_2s = Now();
    } while (true);



    
    LogInfo() << "sender:" << FNLog::AtomicLoadDeviceLog(channel0, 1, FNLog::DEVICE_LOG_TOTAL_WRITE_LINE);
    if (FNLog::AtomicLoadDeviceLog(channel0, 1, FNLog::DEVICE_LOG_TOTAL_WRITE_LINE) > 0)
    {
        LogInfo() << "sender lose:" << FNLog::AtomicLoadDeviceLog(channel0, 1, FNLog::DEVICE_LOG_TOTAL_LOSE_LINE) * 100.0 / FNLog::AtomicLoadDeviceLog(channel0, 1, FNLog::DEVICE_LOG_TOTAL_WRITE_LINE) <<"%";
    }
    LogInfo() << "receiver:" << FNLog::AtomicLoadDeviceLog(channel1, 0, FNLog::DEVICE_LOG_TOTAL_WRITE_LINE);
    LogInfo() << "receiver lose:" << (max_count - lines) * 100.0 / max_count << "%";

    LogInfo() << "per second file write and send:" << lines / (end_1s - begin_s);
    LogInfo() << "per second udp recv:" << lines / (end_2s - begin_s);


    if (lines*100/max_count < 10)
    {
        //lose 90% 
        LogFatal() << "lose udp logs";
        //return -1;
    }


    LogAlarmStream(0, 1, 0) << "finish";

    return 0;
}