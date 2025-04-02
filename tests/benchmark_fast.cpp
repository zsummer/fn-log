


#define FN_LOG_MAX_CHANNEL_SIZE 6
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
        priority: info
    -device:2
        disable: false
        out_type: screen
        category: 2
        category_extend: 1
    -device:3
        disable: false
        out_type: screen
        category: 2
        category_extend: 1
    -device:4
        disable: false
        out_type: screen
        category: 2
        category_extend: 1
    -device:5
        disable: false
        out_type: screen
        category: 2
        category_extend: 1
    -device:6
        disable: false
        out_type: screen
        category: 2
        category_extend: 1
    -device:7
        disable: false
        out_type: screen
        category: 2
        category_extend: 1
    -device:8
        disable: false
        out_type: screen
        category: 2
        category_extend: 1
    -device:9
        disable: false
        out_type: screen
        category: 2
        category_extend: 1
    -device:10
        disable: false
        out_type: screen
        category: 2
        category_extend: 1
    -device:11
        disable: false
        out_type: screen
        category: 2
        category_extend: 1
    -device:12
        disable: false
        out_type: screen
        category: 2
        category_extend: 1


 - channel: 1
    sync: async  
    -device: 0
        disable: false
        out_type: empty
        priority: trace

 - channel: 2



 - channel: 3
    sync: sync  
    -device: 0
        disable: false
        out_type: file
        file: "$PNAME_$YEAR"
        rollback: 4
        limit_size: 100 m #only support M byte

 - channel: 4
    sync: sync  
    -device: 0
        disable: false
        out_type: empty

 - channel: 5
    sync: sync 

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

    

    FNLog::Logger& logger = FNLog::GetDefaultLogger();

    unsigned int total_count = 0;
    for (int channel_id = 0; channel_id < logger.shm_->channel_size_; channel_id++)
    {
        total_count = 0;
        double last = Now();
        FNLog::Channel& channel = logger.shm_->channels_[channel_id];
        do
        {

            LogDebugStream(channel_id, 0, 0).write_buffer("rrrrrrrrrrrrrrrrrrrradfads33333333333333rrd",
                sizeof("rrrrrrrrrrrrrrrrrrrradfads33333333333333rrd") - 1)
                << -23 << ": " << 32.2223 << (void*) nullptr;


            //LOG_STREAM_DEFAULT_LOGGER(channel_id, FNLog::PRIORITY_DEBUG, 0, 0, FNLog::LOG_PREFIX_NULL)
            //    .write_buffer("rrrrrrrrrrrrrrrrrrrradfads33333333333333rrd",
            //        sizeof("rrrrrrrrrrrrrrrrrrrradfads33333333333333rrd") - 1);

            if (total_count %1000000 == 0)
            {
                double now = Now();
                if (total_count > 0)
                {
                    LogInfo() << "channel:<" << (long long)channel_id << "> "
                        << ChannelDesc(channel.channel_type_) << " has write :<"
                        << channel.device_size_ << "> test " << total_count / (now - last) << " line/sec. ";

                    long long ticks = FNLog::GetDefaultLogger().tick_count_;
                    long long sum = FNLog::GetDefaultLogger().tick_sum_;
                    LogAlarmStream(0, 1, 0) << "logs:" << ticks << ", sum:" << sum
                        << ", avg:" << sum / (ticks > 0 ? ticks : 1);

                    break;
                }
            }
        } while (++total_count);
    }


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

    LogAlarmStream(0, 1, 0) << "finish";
    return 0;
}

