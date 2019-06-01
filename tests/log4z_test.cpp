#define USE_LOG4Z_FORMAT
#include "fn_log.h"

int main(int argc, char* argv[])
{
    int ret = FNLog::FastStartSimpleLogger();
    if (ret != 0)
    {
        return ret;
    }

    LOGA("log init success");

    LOGD("now time:" << time(nullptr) << ";");

    LOGFMTD("test:<%d>", 100);
    
    LOGA("finish");


    return 0;
}