


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
    priority: debug
    category: 0
    category_extend: 0
    -device: 0
        disable: false
        out_type: file
        priority: debug
        category: 0
        category_extend: 0
        path: "./log/"
        file: "$PNAME"
        rollback: 4
        limit_size: 100 m #only support M byte
    -device:1
        disable: false
        priority: info
        out_type: screen
        category: 1
        category_extend: 1
 # 1 异步空 
 - channel: 1
    priority: debug

 # 2通道为同步写文件 
 - channel: 2
    sync: sync #only support single thread
    -device: 0
        priority: debug
        disable: false
        out_type: file
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

int main(int argc, char *argv[])
{
    int ret = FNLog::FastStartDefaultLogger(example_config_text);
    if (ret != 0)
    {
        return ret;
    }

    double now = Now();

    LogAlarmStream(0, 1) << "input format: LogDebug() << xxxx << xxx; test:";
    now = Now();
    for (size_t i = 0; i < 1000000; i++)
    {
        LogDebugStream(0, 0) << "asdf" << i << ", " << 2.3 << "asdfasdf";
    }
    LogAlarmStream(0, 1) << "1000w debug(write) use" << (Now() - now) * (10) << " secend";
    now = Now();
    for (size_t i = 0; i < 1000000; i++)
    {
        LogTraceStream(0, 0) << "asdf" << i << ", " << 2.3 << "asdfasdf";
    }
    LogAlarmStream(0, 1) << "1000w trace(no write) use" << (Now() - now) * (10) << " secend";

    now = Now();
    for (size_t i = 0; i < 1000000; i++)
    {
        LogDebugStream(1, 0) << "asdf" << i << ", " << 2.3 << "asdfasdf";
    }
    LogAlarmStream(0, 1) << "1000w debug(no device no write) use" << (Now() - now) * (10) << " secend";
    now = Now();
    for (size_t i = 0; i < 1000000; i++)
    {
        LogTraceStream(1, 0) << "asdf" << i << ", " << 2.3 << "asdfasdf";
    }
    LogAlarmStream(0, 1) << "1000w trace(no device no write) use" << (Now() - now) * (10) << " secend";


    now = Now();
    for (size_t i = 0; i < 100000; i++)
    {
        LogDebugStream(2, 0) << "asdf" << i << ", " << 2.3 << "asdfasdf";
    }
    LogAlarmStream(0, 1) << "1000w debug(sync write) use" << (Now() - now) * (100) << " secend";
    now = Now();
    for (size_t i = 0; i < 1000000; i++)
    {
        LogTraceStream(2, 0) << "asdf" << i << ", " << 2.3 << "asdfasdf";
    }
    LogAlarmStream(0, 1) << "1000w trace(sync no write) use" << (Now() - now) * (10) << " secend";


    LogAlarmStream(0, 1) << "\n\n";
    LogAlarmStream(0, 1) << "input format: LOGFMTD(\"sss\", xxx, xxx); test:";
    now = Now();
    for (size_t i = 0; i < 1000000; i++)
    {
        LOGFMT_DEBUG(0, 0, "%s, %llu, %g, %s",  "asdf" , i ,  2.3 , "asdfasdf");
    }
    LogAlarmStream(0, 1) << "1000w debug(write) use" << (Now() - now) * (10) << " secend";
    now = Now();
    for (size_t i = 0; i < 1000000; i++)
    {
        LOGFMT_TRACE(0, 0, "%s, %llu, %g, %s", "asdf", i, 2.3, "asdfasdf");
    }
    LogAlarmStream(0, 1) << "1000w trace(no write) use" << (Now() - now) * (10) << " secend";

    now = Now();
    for (size_t i = 0; i < 1000000; i++)
    {
        LOGFMT_DEBUG(1, 0, "%s, %llu, %g, %s", "asdf", i, 2.3, "asdfasdf");
    }
    LogAlarmStream(0, 1) << "1000w debug(no device no write) use" << (Now() - now) * (10) << " secend";
    now = Now();
    for (size_t i = 0; i < 1000000; i++)
    {
        LOGFMT_TRACE(1, 0, "%s, %llu, %g, %s", "asdf", i, 2.3, "asdfasdf");
    }
    LogAlarmStream(0, 1) << "1000w trace(no device no write) use" << (Now() - now) * (10) << " secend";


    now = Now();
    for (size_t i = 0; i < 100000; i++)
    {
        LOGFMT_DEBUG(2, 0, "%s, %llu, %g, %s", "asdf", i, 2.3, "asdfasdf");
    }
    LogAlarmStream(0, 1) << "1000w debug(sync write) use" << (Now() - now) * (100) << " secend";
    now = Now();
    for (size_t i = 0; i < 1000000; i++)
    {
        LOGFMT_TRACE(2, 0, "%s, %llu, %g, %s", "asdf", i, 2.3, "asdfasdf");
    }
    LogAlarmStream(0, 1) << "1000w trace(sync no write) use" << (Now() - now) * (10) << " secend";





    LogAlarmStream(0, 1) << "finish";
    return 0;
}

