#include "fn_log.h"
#define FNLOG_ASSERT(expr, prefix) \
{ \
    if (expr) \
    { \
        LogInfo() << "test " << prefix << " success."; \
    } \
    else \
    {   \
        LogError() << "test " << prefix << " failed."; \
        return __LINE__ * -1; \
    } \
}

static const std::string test_text =
R"----(
 # info log print screen and all write file

 - channel: 0
    sync: sync
    - device:0
        disable: false
        out_type: screen
        priority: debug
)----";

using namespace FNLog;
int main(int argc, char* argv[])
{
    int ret = FastStartDefaultLogger(test_text);
    if (ret != 0 )
    {
        return ret ;
    }



    unsigned long long bitmap;
    std::string buf;
    

    LogInfo() << "begin:" << LogTimestamp();

    buf = "0";
    bitmap = FNLog::ParseBitArray(buf.c_str(), buf.c_str() + buf.length());
    FNLOG_ASSERT(bitmap == 1, "");


    buf = "0,1";
    bitmap = FNLog::ParseBitArray(buf.c_str(), buf.c_str() + buf.length());
    FNLOG_ASSERT(bitmap == 3, "");


    buf = "0,1,2";
    bitmap = FNLog::ParseBitArray(buf.c_str(), buf.c_str() + buf.length());
    FNLOG_ASSERT(bitmap == 7, "");

    buf = "0-2";
    bitmap = FNLog::ParseBitArray(buf.c_str(), buf.c_str() + buf.length());
    FNLOG_ASSERT(bitmap == 7, "");


    buf = "2-0";
    bitmap = FNLog::ParseBitArray(buf.c_str(), buf.c_str() + buf.length());
    FNLOG_ASSERT(bitmap == 7, "");




    buf = "2-2";
    bitmap = FNLog::ParseBitArray(buf.c_str(), buf.c_str() + buf.length());
    FNLOG_ASSERT(bitmap == 4, "");

    buf = "2";
    bitmap = FNLog::ParseBitArray(buf.c_str(), buf.c_str() + buf.length());
    FNLOG_ASSERT(bitmap == 4, "");


    buf = " 2 - 0  , 2 - 3";
    bitmap = FNLog::ParseBitArray(buf.c_str(), buf.c_str() + buf.length());
    FNLOG_ASSERT(bitmap == 15, "");

    buf = " , 2 - 0  , 2 - 3, ";
    bitmap = FNLog::ParseBitArray(buf.c_str(), buf.c_str() + buf.length());
    FNLOG_ASSERT(bitmap == 15, "");

    buf = " 0-18, 19- 24 ";
    bitmap = FNLog::ParseBitArray(buf.c_str(), buf.c_str() + buf.length());
    FNLOG_ASSERT(bitmap == (1ULL << 25)-1, "");

    buf = " 0-18, 19, 20, 24, 23, 22, 24, 21 ";
    bitmap = FNLog::ParseBitArray(buf.c_str(), buf.c_str() + buf.length());
    FNLOG_ASSERT(bitmap == (1ULL << 25) - 1, "");



    buf = " 0-18, 20- 24 ";
    bitmap = FNLog::ParseBitArray(buf.c_str(), buf.c_str() + buf.length());
    FNLOG_ASSERT(bitmap != (1ULL << 25) - 1, "");

    return 0;
}
