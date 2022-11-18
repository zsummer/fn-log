


#define FN_LOG_MAX_CHANNEL_SIZE 4
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
        path: "./$PNAME_%F"
        file: "$PNAME"
        rollback: 4
        stuff: false
        limit_size: 100 m #only support M byte
    -device:1
        disable: false
        out_type: screen
        category: 1
        category_extend: 1
    -device:2
        disable: false
        out_type: screen
        category: 1
        category_extend: 1
    -device:3
        disable: false
        out_type: screen
        category: 1
        category_extend: 1
    -device:4
        disable: false
        out_type: screen
        category: 1
        category_extend: 1
    -device:5
        disable: false
        out_type: screen
        category: 1
        category_extend: 1
    -device:6
        disable: false
        out_type: screen
        category: 1
        category_extend: 1
    -device:7
        disable: false
        out_type: screen
        category: 1
        category_extend: 1
    -device:8
        disable: false
        out_type: screen
        category: 1
        category_extend: 1
    -device:9
        disable: false
        out_type: screen
        category: 1
        category_extend: 1
    -device:10
        disable: false
        out_type: screen
        category: 1
        category_extend: 1
    -device:11
        disable: false
        out_type: screen
        category: 1
        category_extend: 1
    -device:12
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
        path: "./$PNAME_$YEAR-$MON-$DAY"
        file: "$PNAME_$YEAR"
        rollback: 4
        stuff: false
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

int main(int argc, char* argv[])
{
    int ret = FNLog::FastStartDefaultLogger(example_config_text);
    if (ret != 0)
    {
        return ret;
    }


    if (true)
    {
        tm t = FNLog::FileHandler::time_to_tm(time(NULL));

        std::string ret = FNLog::PreFmtName("%F");
        if (ret != "$YEAR-$MON-$DAY")
        {
            LogErrorStream(0, 1, 0) << "error";
            return -1;
        }

        ret = FNLog::FmtName("%F", 0, 0, t);
        if (ret.size() != strlen("2022-02-02"))
        {
            LogErrorStream(0, 1, 0) << "error";
            return -1;
        }

        std::string ret2 = FNLog::FmtName("$YEAR-$MON-$DAY", 0, 0, t);
        if (ret != ret2)
        {
            LogErrorStream(0, 1, 0) << "error";
            return -2;
        }
        ret2 = FNLog::FmtName("$YEAR-$MON-$DAY/", 0, 0, t);
        if (ret + "/" != ret2)
        {
            LogErrorStream(0, 1, 0) << "error";
            return -2;
        }

        ret = FNLog::FmtName("$", 0, 0, t);
        if (ret != "$")
        {
            LogErrorStream(0, 1, 0) << "error";
            return -11;
        }

        ret = FNLog::FmtName("%", 0, 0, t);
        if (ret != "%")
        {
            LogErrorStream(0, 1, 0) << "error";
            return -12;
        }

        ret = FNLog::FmtName("", 0, 0, t);
        if (ret != "")
        {
            LogErrorStream(0, 1, 0) << "error";
            return -13;
        }
        ret = FNLog::FmtName("$%$%$$$%%%$$%%%$234df%ee%dws$ds$", 0, 0, t);
        if (ret != "$%$%$$$%%%$$%%%$234df%ee%dws$ds$")
        {
            LogErrorStream(0, 1, 0) << "error";
            return -14;
        }
    }


    double now = Now();
    for (size_t i = 0; i < 500000; i++)
    {
        LogTraceStream(3, 0, 0) << "asdf" << i << ", " << 2.3 << "asdfasdf";
    }
    LogAlarmStream(0, 1, 0) << "use " << Now() - now << " secend";
    now = Now();
    for (size_t i = 0; i < 500000; i++)
    {
        LOG_TRACE(3, 0, 0, "asdf" << i << ", " << 2.3 << "asdfasdf");
    }
    LogAlarmStream(0, 1, 0) << "use " << Now() - now << " secend";
    now = Now();
    for (size_t i = 0; i < 500000; i++)
    {
        LogTracePack(3, 0, 0, "asdf", i, ", ", 2.3, "asdfasdf");
    }
    LogAlarmStream(0, 1, 0) << "use " << Now() - now << " secend";

    FNLog::Logger& logger = FNLog::GetDefaultLogger();

    unsigned int total_count = 0;
    for (int i = 0; i < logger.shm_->channel_size_; i++)
    {
        total_count = 0;
        int current_cc = 0;
        do
        {
            current_cc++;
            if (true)
            {
                auto x = std::move(LOG_STREAM_DEFAULT_LOGGER(i, FNLog::PRIORITY_DEBUG, 0, 0, FNLog::LOG_PREFIX_DEFAULT));
                if (x.log_data_)
                {
                    x.log_data_->timestamp_ += current_cc/4;
                }
                else
                {
                    break;
                }
                
            }


            if (total_count % 1000000 == 0)
            {
                static long long last = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
                long long now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
                if (total_count > 0)
                {
                    LogInfoStream(0, 1, 0) << "channel:<" << (long long)i << "> "
                        << ChannelDesc(logger.shm_->channels_[i].channel_type_) << " has write file:<"
                        << logger.shm_->channels_[i].device_size_ << "> test " << 1000000 * 1000 / (now - last) << " line/sec. ";
                    last = now;
                    break;
                }
            }
        } while (++total_count);
    }

    LogAlarmStream(0, 1, 0) << "finish";
    return 0;
}

