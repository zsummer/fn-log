
#define FN_LOG_MAX_CHANNEL_SIZE 5
#define FN_LOG_MAX_LOG_SIZE 1000
#define FN_LOG_MAX_LOG_QUEUE_SIZE 100000
#include "fn_log.h"

static const std::string example_config_text =
R"----(
 # 压测配表  
 # 0通道为异步模式写文件, info多线程文件输出和一个CATEGORY筛选的屏显输出 
 - channel: 0
    sync: null
    priority: trace
    category: 0
    category_extend: 0
    -device: 0
        disable: false
        out_type: file
        priority: trace
        category: 0
        category_extend: 0
        path: "./log/"
        file: "$PNAME"
        rollback: 4
        limit_size: 100 m #only support M byte
    -device:1
        disable: false
        out_type: screen
        category: 1
        category_extend: 1
 # 1 异步空 
 - channel: 1

 # 2通道为同步写文件 
 - channel: 2
    sync: sync #only support single thread
    -device: 0
        disable: false
        out_type: file
        file: "$PNAME_$YEAR"
        rollback: 4
        limit_size: 100 m #only support M byte

 # 3通道为同步空 
 - channel: 3
    sync: sync #only support single thread


 # 4通道为异步,输出到虚拟设备 
 - channel: 4
    sync: null #only support single thread
    -device: 0
        disable: false
        out_type: virtual
        file: "$PNAME_virtual"
        rollback: 4
        limit_size: 100 m #only support M byte
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


void MyVirtualDevice(const FNLog::LogData& log)
{
    //printf("%s", log.content_ + log.prefix_len_);
}

int main(int argc, char *argv[])
{
    int ret = FNLog::FastStartDefaultLogger(example_config_text);
    if (ret != 0)
    {
        return ret;
    }
    FNLog::SetVirtualDevice(&MyVirtualDevice);
    FNLog::Logger& logger = FNLog::GetDefaultLogger();

    unsigned int total_count = 0;
    for (int i = 0; i < logger.shm_->channel_size_; i++)
    {
        total_count = 0;
        do
        {
            LogDebugStream(i, 0, 0).write_buffer("rrrrrrrrrrrrrrrrrrrradfads33333333333333rrd",
                                sizeof("rrrrrrrrrrrrrrrrrrrradfads33333333333333rrd") - 1)
                << -23 << ": " << 32.2223 << (void*) nullptr;
            
            if (total_count %1000000 == 0)
            {
                static long long last = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
                long long now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
                if (total_count > 0)
                {
                    LogInfoStream(0, 1, 0) << "channel:<" << (long long)i << "> "
                        << ChannelDesc(logger.shm_->channels_[i].channel_type_) << " <"
                        << logger.shm_->channels_[i].device_size_ << "> test " << 1000000*1000 / (now - last) << " line/sec. ";
                    last = now;
                    break;
                }
            }
        } while (++total_count);
    }

    LogAlarmStream(0, 1, 0) << "finish";
    return 0;
}

