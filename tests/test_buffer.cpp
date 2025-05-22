#include "fn_log.h"

static const std::string example_config_text =
R"----(
 # info log print screen and all write file
 - channel: 0
    sync: null
    -device: 0
        disable: false
        out_type: file
        priority: trace
        path: "./log/"
        file: "$PNAME_$YEAR$MON$DAY"
        rollback: 4
        limit_size: 100 m #only support M byte
    -device:1
        disable: false
        out_type: screen
        priority: info

)----";

int main(int argc, char* argv[])
{
    int ret = FNLog::FastStartDefaultLogger(example_config_text);
    if (ret != 0 )
    {
        return ret ;
    }

    LogAlarm() << "log init success";  //sync write

    for (int i = 0; i < 1000; i++)
    {
        auto ls(std::move(LogDebug()));
        ls << "log begin test buffer";
        int rd = 0;
        int last_len = 0;
        for (int j = 0; j < FNLog::LogData::LOG_SIZE; j++)
        {
            rd = rand() % 10;
            last_len = ls.log_data_->content_len_;
            switch (rd)
            {
            case 0:
                ls.write_hex_text("aaa", 3);
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
            int length = ls.log_data_->content_len_;
            if (length > FNLog::LogData::LOG_SIZE)
            {
                LogFatal() << "error len:" << length;
                return -1;
            }
        }
        (void) rd;
        (void) last_len;

    }
    LogAlarm() << "finish";

    return 0;
}
