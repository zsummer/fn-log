


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
        out_type: screen
        priority: trace
        category: 1
        category_extend: 1
        path: "./log/"
        file: "$PNAME_00"
        rollback: 4
        limit_size: 100 m #only support M byte
    -device: 1
        disable: false
        out_type: file
        priority: debug
        category: 0
        category_extend: 5
        path: "./log/"
        file: "$PNAME_01"
        rollback: 4
        limit_size: 100 m #only support M byte
    -device: 2
        disable: false
        out_type: file
        priority: trace
        category: 8
        category_extend: 9
        path: "./log/"
        file: "$PNAME_02"
        rollback: 4
        limit_size: 100 m #only support M byte
    -device: 3
        disable: false
        out_type: file
        priority: trace
        category: 3
        category_extend: 3
        path: "./log/"
        file: "$PNAME_03"
        rollback: 4
        limit_size: 100 m #only support M byte
    -device: 4
        disable: false
        out_type: file
        priority: trace
        category_filter: 4,5
        path: "./log/"
        file: "$PNAME_04"
        rollback: 4
        limit_size: 100 m #only support M byte
    -device: 5
        disable: false
        out_type: file
        priority: trace
        category_filter: 4,5
        identify_filter: 0,1,4,7,16,36,46
        path: "./log/"
        file: "$PNAME_05"
        rollback: 4
        limit_size: 100 m #only support M byte
    -device: 6
        disable: false
        out_type: file
        priority: trace
        category_filter: 4,8
        identify_filter: 0,1,4,7,16,36,46
        path: "./log/"
        file: "$PNAME_06"
        rollback: 4
        limit_size: 100 m #only support M byte
    -device: 7
        disable: false
        out_type: file
        priority: trace
        identify: 0
        identify_extend: 1
        path: "./log/"
        file: "$PNAME_07"
        rollback: 4
        limit_size: 100 m #only support M byte
    -device: 8
        disable: false
        out_type: file
        priority: trace
        category: 0
        category_extend: 1
        identify: 0
        identify_extend: 1
        path: "./log/"
        file: "$PNAME_08"
        rollback: 4
        limit_size: 100 m #only support M byte
    -device: 9
        disable: false
        out_type: file
        priority: trace
        category: 9
        category_extend: 10
        identify: 9
        identify_extend: 10
        path: "./log/"
        file: "$PNAME_09"
        rollback: 4
        limit_size: 100 m #only support M byte
    -device: 10
        disable: false
        out_type: file
        priority: trace
        category: 9
        category_extend: 10
        identify: 9
        identify_extend: 10
        path: "./log/"
        file: "$PNAME_10"
        rollback: 4
        limit_size: 100 m #only support M byte
    -device: 11
        disable: false
        out_type: file
        priority: trace
        category: 9
        category_extend: 10
        identify: 9
        identify_extend: 10
        path: "./log/"
        file: "$PNAME_11"
        rollback: 4
        limit_size: 100 m #only support M byte
    -device: 12
        disable: false
        out_type: file
        priority: debug
        category: 9
        category_extend: 10
        identify: 9
        identify_extend: 10
        path: "./log/"
        file: "$PNAME_12"
        rollback: 4
        limit_size: 100 m #only support M byte
    -device: 13
        disable: false
        out_type: file
        priority: trace
        category: 9
        category_extend: 10
        identify: 9
        identify_extend: 10
        path: "./log/"
        file: "$PNAME_13"
        rollback: 4
        limit_size: 100 m #only support M byte
    -device: 14
        disable: false
        out_type: file
        priority: trace
        category: 9
        category_extend: 10
        identify: 9
        identify_extend: 10
        path: "./log/"
        file: "$PNAME_14"
        rollback: 4
        limit_size: 100 m #only support M byte
    -device: 15
        disable: false
        out_type: file
        priority: trace
        category: 9
        category_extend: 10
        identify: 9
        identify_extend: 10
        path: "./log/"
        file: "$PNAME_15"
        rollback: 4
        limit_size: 100 m #only support M byte
    -device: 16
        disable: false
        out_type: file
        priority: trace
        category: 9
        category_extend: 10
        identify: 9
        identify_extend: 10
        path: "./log/"
        file: "$PNAME_16"
        rollback: 4
        limit_size: 100 m #only support M byte

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
    for (size_t i = 0; i < 500000; i++)
    {
        LogTraceStream(0, 0, 0) << "asdf" << i << ", " << 2.3 << "asdfasdffffffffffffffffffffffffffffffffffffffff";
    }
    LogAlarmStream(0, 1, 0) << "per log used " << (Now() - now)*1000*1000*1000 / 500000 << "ns";


    now = Now();
    for (size_t i = 0; i < 500000; i++)
    {
        LogTraceStream(0, 100, 0) << "asdf" << i << ", " << 2.3 << "asdfasdffffffffffffffffffffffffffffffffffffffff";
    }
    LogAlarmStream(0, 1, 0) << "per log used " << (Now() - now) * 1000 * 1000 * 1000 / 500000 << "ns";

    LogAlarmStream(0, 1, 0) << "finish";
    return 0;
}

