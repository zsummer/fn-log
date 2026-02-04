


#define FN_LOG_MAX_CHANNEL_SIZE 4
#define FN_LOG_MAX_LOG_SIZE 1000
#define FN_LOG_MAX_LOG_QUEUE_SIZE 100000

#include "fn_log.h"
#include <chrono>

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
    -device:2
        disable: false
        out_type: screen
        category: 10
        category_extend: 1
    -device:3
        disable: false
        out_type: screen
        category: 100
        category_extend: 1
    -device:4
        disable: false
        out_type: screen
        category: 10
        category_extend: 1
    -device:5
        disable: false
        out_type: screen
        category: 10
        category_extend: 1
    -device:6
        disable: false
        out_type: screen
        category: 10
        category_extend: 1
    -device:7
        disable: false
        out_type: screen
        category: 10
        category_extend: 1
    -device:8
        disable: false
        out_type: screen
        category: 10
        category_extend: 1
    -device:9
        disable: false
        out_type: screen
        category: 100
        category_extend: 1
    -device:10
        disable: false
        out_type: screen
        category: 10
        category_extend: 1
    -device:11
        disable: false
        out_type: screen
        category: 11
        category_extend: 1
    -device:12
        disable: false
        out_type: screen
        category: 12
        category_extend: 1

 # 1 异步空 
 - channel: 1
    -device: 0
        disable: false
        out_type: vm
        priority: trace
        category: 0
        category_extend: 0
 # 2通道为同步写文件 
 - channel: 2
    sync: sync #only support single thread
    priority: trace
    -device: 0
        disable: false
        out_type: file
        priority: trace
        file: "$PNAME_$YEAR"
        rollback: 4
        limit_size: 100 m #only support M byte

 # 3通道为同步空 
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

long long g_virtual_device_write_count = 0;
void VirtualDevice(const FNLog::LogData& log)
{
    FNLog::TransmitChannel(FNLog::GetDefaultLogger(), 0, log.category_, log.identify_, log);
}

int main(int argc, char *argv[])
{
    int ret = FNLog::FastStartDefaultLogger(example_config_text);
    if (ret != 0)
    {
        return ret;
    }
    FNLog::SetVirtualDevice(&VirtualDevice);

    LogAlarmStream(0, 1, 0) << "begin test. screen channel 0, 1, 0 ";

    long long loop_count = 500000;

    double now = Now();

    now = Now();
    for (long long i = 0; i < loop_count; i++)
    {
        LogTraceStream(0, 0, 0) << "asdf" << i << ", " << 2.3 << "asdfasdf   from channel 2" << "asdfasdf   from channel 1";
    }
    LogAlarmStream(0, 1, 0) << "loop:" << loop_count << ", per log used:" << (Now() - now) * 1000 * 1000 * 1000 / loop_count << " ns";
    now = Now();

    if (true)
    {
        now = Now();
        FNLog::LogStream ls(std::move(LogTraceStream(1, 0, 0) << "s"));
        ls << "asdf" << 0 << ", " << 2.3 << "asdfasdf   from channel 2" << "asdfasdf   from channel 1";
        for (long long i = 0; i < loop_count; i++)
        {
            (*FNLog::RefVirtualDevice())(*ls.log_data_);
        }
        LogAlarmStream(0, 1, 0) << "loop:" << loop_count << ", per log used:" << (Now() - now) * 1000 * 1000 * 1000 / loop_count << " ns";
        now = Now();
    }





    now = Now();
    for (long long i = 0; i < loop_count; i++)
    {
        LogTraceStream(1, 0, 0) << "asdf" << i << ", " << 2.3 << "asdfasdf   from channel 2" << "asdfasdf   from channel 1";
    }
    LogAlarmStream(0, 1, 0) << "loop:" << loop_count <<", per log used:" << (Now() - now)*1000*1000*1000/ loop_count << " ns";
    now = Now();
    for (long long i = 0; i < loop_count; i++)
    {
        LOG_TRACE(1, 0, 0, "asdf" << i << ", " << 2.3 << "asdfasdf   from channel 1");
    }
    LogAlarmStream(0, 1, 0) << "loop:" << loop_count << ", per log used:" << (Now() - now) * 1000 * 1000 * 1000 / loop_count << " ns";
    now = Now();
    for (long long i = 0; i < loop_count; i++)
    {
        LogTracePack(1, 0, 0, "asdf" , i , ", " , 2.3 , "asdfasdf. from channel 1" );
    }
    LogAlarmStream(0, 1, 0) << "loop:" << loop_count << ", per log used:" << (Now() - now) * 1000 * 1000 * 1000 / loop_count << " ns";

    FNLog::Logger& logger = FNLog::GetDefaultLogger();

   
    for (int i = 0; i < logger.shm_->channel_size_; i++)
    {
        unsigned int total_count = 0;
        now = Now();
        do
        {

            LOG_STREAM_DEFAULT_LOGGER(i, FNLog::PRIORITY_DEBUG, 0, 0, FNLog::LOG_PREFIX_NULL)
                .write_buffer("rrrrrrrrrrrrrrrrrrrradfads33333333333333rrd",
                    sizeof("rrrrrrrrrrrrrrrrrrrradfads33333333333333rrd") - 1);

            //LOG_TRACE(i, 0, 0, "asdf" << i << ", " << 2.3 << "asdfasdf   from channel 2");

            if (total_count % loop_count == 0 && total_count != 0)
            {
                LogInfoStream(0, 1, 0) << "channel:<" << (long long)i << "> "
                    << ChannelDesc(logger.shm_->channels_[i].channel_type_) << " write file:<"
                    << logger.shm_->channels_[i].device_size_ << ">  prof: " << (Now() - now) * 1000.0 * 1000.0 * 1000.0 / total_count << " ns. ";
                break;
            }
        } while (++total_count);
    }

    LogAlarmStream(0, 1, 0) << "finish";
    return 0;
}

