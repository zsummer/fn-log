#define FN_LOG_MAX_CHANNEL_SIZE 4
#include "fn_log.h"

static const std::string example_config_text =
R"----(
 # 压测配表  
 # 异步UDP
 - channel: 0
    sync: null
    priority: trace
    category: 0
    category_extend: 0
    -device: 0
        disable: false
        out_type: udp
        udp_addr: 127.0.0.1:9909
        priority: trace
        category: 0
        category_extend: 0
        path: "./log/"
        file: "$PNAME_$YEAR$MON$DAY"
        rollback: 4
        limit_size: 100 m #only support M byte
    -device:1
        disable: false
        out_type: screen
        category: 1
        category_extend: 1
 # 1通道为异步空设备
 - channel: 1

 # 2通道为同步UDP
 - channel: 2
    sync: sync #only support single thread
    -device: 0
        disable: false
        out_type: udp
        udp_addr: 127.0.0.1:9909
        file: "$PNAME_$YEAR"
        rollback: 4
        limit_size: 100 m #only support M byte

 # 3通道为同步空设备
 - channel: 3
    sync: sync #only support single thread

)----";

std::string ChannelDesc(int channel_type)
{
    switch (channel_type)
    {
    case FNLog::CHANNEL_ASYNC:
        return "async thread write";
    case FNLog::CHANNEL_SYNC:
        return "sync write";
    }
    return "invalid channel";
}

#define Now()  std::chrono::duration<double>(std::chrono::system_clock::now().time_since_epoch()).count()

int main(int argc, char *argv[])
{
    int ret = FNLog::FastStartDefaultLogger(example_config_text);
    if (ret != 0)
    {
        return ret;
    }
#ifdef WIN32

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        printf("%s", "WSAStartup error.\n");
        return -1;
    }
#endif
    FNLog::Logger& logger = FNLog::GetDefaultLogger();
    LogAlarmStream(0, 1, 0) << "begin";
    unsigned int total_count = 0;
    double begin = Now();
    double last = Now();
    for (int i = 0; i < logger.shm_->channel_size_; i++)
    {
        total_count = 0;
        do
        {
            LogDebugStream(i, 0, 0).write_buffer("rrrrrrrrrrrrrrrrrrrradfads33333333333333rrd",
                                sizeof("rrrrrrrrrrrrrrrrrrrradfads33333333333333rrd") - 1)
                << -23 << ": " << 32.2223 << (void*) nullptr;
            
            if (total_count %100000 == 0)
            {
                double now = Now();
                if (total_count > 0 && now - last > 0.0001f)
                {
                    LogInfoStream(0, 1, 0) << "channel:<" << (long long)i << "> "
                        << ChannelDesc(logger.shm_->channels_[i].channel_type_) << " <"
                        << logger.shm_->channels_[i].device_size_ << "> test " << 100000ULL / (now - last)  << "line/sec.";

  
                    double now = Now();
                    FNLog::Channel& channel = FNLog::GetDefaultLogger().shm_->channels_[i];

                    long long sc = FNLog::AtomicLoadDeviceLog(channel, 0, FNLog::DEVICE_LOG_TOTAL_WRITE_LINE);
                    long long sl = FNLog::AtomicLoadDeviceLog(channel, 0, FNLog::DEVICE_LOG_TOTAL_LOSE_LINE);
                    if (sc > 0)
                    {
                        LogInfoStream(0, 1, 0) << "channel:<" << (long long)i << "> "
                            << ChannelDesc(logger.shm_->channels_[i].channel_type_) << " <"
                            << logger.shm_->channels_[i].device_size_ << ">"
                            << " lose/total:" << sl << "/" << sc << ", lose:" << sl * 100.0 / sc << "%, real per second:" << (sc - sl) / ((now - begin));
                    }

                    break;
                }
            }

        } while (++total_count);
    }




    LogAlarmStream(0, 1, 0) << "finish";
    return 0;
}

