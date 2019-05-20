#include "fn_log.h"

static const std::string example_config_text =
R"----(
 # 0通道为多线程文件输出和一个CLS筛选的屏显输出 
 - channel: 0
    sync: null
    -device: 0
        disable: false
        out_type: file
        filter_level: trace
        path: "./log/"
        file: "$PNAME_$YEAR$MON$DAY"
        rollback: 4
        limit_size: 100 m #only support M byte
    -device:1
        disable: false
        out_type: screen
        filter_level: info

)----";

int main(int argc, char* argv[])
{
    FNLog::GuardLogger gl(FNLog::GetDefaultLogger());
    FNLog::InitFromYMAL(example_config_text, "", FNLog::GetDefaultLogger());
    int ret = FNLog::StartDefaultLogger();
    if (ret != 0 || FNLog::GetDefaultLogger().last_error_ != 0)
    {
        return ret || FNLog::GetDefaultLogger().last_error_;
    }

    LOGA() << "log init success";  //sync write

    for (int i = 0; i < 1000; i++)
    {
        auto ls(LOGD());
        ls << "log begin test buffer";
        for (int j = 0; j < FNLog::LogData::MAX_LOG_SIZE; j++)
        {

            switch (rand() % 10)
            {
            case 0:
                ls.write_binary("aaa", 3);
                break;
            case 1:
                ls.write_buffer("bbb", 3);
                break;
            case 2:
                ls << std::map<int, double>({ {1,0.2}, {2, 0.3}, {4, 0.4} });
                break;
            case 3:
                ls << 3.24;
                break;
            case 4:
                ls << 3.14f;
                break;
            case 5:
                ls << 'c';
                break;
            case 6:
                ls << (unsigned char)('c');
                break;
            case 7:
                ls << -5555523;
                break;
            case 8:
                ls << -92342342342LL;
                break;
            case 9:
                ls << 3242343242342ULL;
                break;
            default:
                break;
            }
        }
        int length = ls.log_data_->content_len_;
        if (length > FNLog::LogData::MAX_LOG_SIZE)
        {
            LOGF() << "error len:" << length;
            return -1;
        }
    }
    LOGA() << "finish";

    return 0;
}
