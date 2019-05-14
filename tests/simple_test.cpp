#include "fn_log.h"

static const std::string example_config_text =
R"----(
 # 0通道为支持多线程写入的屏显输出和文件输出, 只有cls为1的日志会屏显 
 - channel: 0
    sync: null
    filter_level: trace
    filter_cls_begin: 0
    filter_cls_count: 0
    -device: 0
        disable: false
        out_type: screen
        filter_cls_begin: 1
        filter_cls_count: 1
    -device:1
        disable: false
        out_type: file
        filter_level: trace
        filter_cls_begin: 0
        filter_cls_count: 0
        path: "./"  #当前路径  
        file: "$PNAME_$YEAR$MON$DAY" #文件名为进程名+年月日.log 
        rollback: 4 #回滚4个文件
        limit_size: 100 m #only support M byte

)----";

int main(int argc, char* argv[])
{
    FNLog::GuardLogger gl(FNLog::GetDefaultLogger());
    int ret = FNLog::InitFromYMAL(example_config_text, "", FNLog::GetDefaultLogger());
    ret |= FNLog::StartDefaultLogger();
    if (ret != 0 || FNLog::GetDefaultLogger().last_error_ != 0)
    {
        return ret || FNLog::GetDefaultLogger().last_error_;
    }

    LOGD() << "log init success";

    LOGD() << "now time:" << (long long)time(nullptr) << ";";
    
    (LOGD() <<"hex text:").write_binary("fnlog", sizeof("fnlog"));

    LOGCA(0, 1) << "finish";

    return 0;
}