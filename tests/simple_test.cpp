#include "fn_log.h"

int main(int argc, char* argv[])
{
    int ret = FNLog::FastStartDefaultLogger();
    if (ret != 0)
    {
        return ret;
    }

    LOGA() << "log init success";

    LOGD() << "now time:" << time(nullptr) << ";";
    
    LOGA() << "finish";


    return 0;
}