#include "fn_log.h"




static const std::string example_config_text =
R"----(
 # 压测配表  
 # 0通道为多线程文件输出和一个CLS筛选的屏显输出 
 - channel: 0
    sync: null
    filter_level: trace
    filter_cls_begin: 0
    filter_cls_count: 0
    -device: 0
        disable: false
        out_type: file
        filter_level: trace
        filter_cls_begin: 0
        filter_cls_count: 0
        path: "./log/"
        file: "$PNAME_$YEAR$MON$DAY"
        rollback: 4
        limit_size: 100 m #only support M byte
    -device:1
        disable: false
        out_type: screen
        filter_cls_begin: 1
        filter_cls_count: 1
 # 1通道为多线程不挂任何输出端 
 - channel: 1

 # 2通道为单线程异步写文件(回环队列) 
 - channel: 2
    sync: ring #only support single thread
    -device: 0
        disable: false
        out_type: file
        file: "$PNAME_$YEAR_ring"
        rollback: 4
        limit_size: 100 m #only support M byte

 # 3通道为单线程异步无输出端(回环队列) 
 - channel: 3
    sync: ring #only support single thread

 # 4通道为单线程同步写文件 
 - channel: 4
    sync: sync #only support single thread
    -device: 0
        disable: false
        out_type: file
        file: "$PNAME_$YEAR"
        rollback: 4
        limit_size: 100 m #only support M byte

 # 5通道为单线程无输出端 
 - channel: 5
    sync: sync #only support single thread

)----";

std::string ChannelDesc(int channel_type)
{
    switch (channel_type)
    {
        case FNLog::CHANNEL_MULTI:
            return "async write";
        case FNLog::CHANNEL_RING:
            return "ring write";
        case FNLog::CHANNEL_SYNC:
            return "sync write";
    }
    return "invalid channel";
}

int main(int argc, char *argv[])
{
    int ret = FNLog::FastStartDefaultLogger(example_config_text);
    if (ret != 0)
    {
        return ret;
    }

    FNLog::Logger& logger = FNLog::GetDefaultLogger();

    unsigned int total_count = 0;
    for (int i = 0; i < logger.channel_size_; i++)
    {
        total_count = 0;
        do
        {
            LOG_STREAM_IMPL(logger, i, FNLog::LOG_LEVEL_DEBUG, 0, FNLog::LOG_PREFIX_NULL)
                .write_buffer("rrrrrrrrrrrrrrrrrrrradfads33333333333333rrd",
                    sizeof("rrrrrrrrrrrrrrrrrrrradfads33333333333333rrd") - 1);

            if (total_count %1000000 == 0)
            {
                static long long last = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
                long long now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
                if (total_count > 0)
                {
                    LOGCI(0, 1) << "channel:<" << (long long)i << "> "
                        << ChannelDesc(logger.channels_[i].channel_type_) << " has write file:<"
                        << logger.channels_[i].device_size_ << "> test " << 1000000*1000 / (now - last) << " line/sec. cache hit:"
                        << (double)logger.channels_[i].log_fields_[FNLog::CHANNEL_LOG_ALLOC_CACHE].num_ 
                                / logger.channels_[i].log_fields_[FNLog::CHANNEL_LOG_ALLOC_CALL].num_ * 100 << "%";
                    last = now;
                    break;
                }
            }
        } while (++total_count);
    }

    LOGCA(0, 1) << "finish";
    return 0;
}

