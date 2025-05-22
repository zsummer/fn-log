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
    sync: null
    - device:0
        disable: false
        out_type: screen
        priority: debug

)----";

struct HexHead
{
    char a[10] = "head";
    short b = 127;
    int c = 7;
    long long e= -1;
    double f = -1.04;
};

using namespace FNLog;
int main(int argc, char* argv[])
{
    int ret = FastStartDefaultLogger();
    if (ret != 0 )
    {
        return ret ;
    }




    LogAlarm() << "log init success";  //sync write
    HexHead head;
    LogAlarm() << FNLog::LogHexText(&head, sizeof(head));
    LogAlarm() << FNLog::LogHexText(head);
    LogAlarm() << FNLog::LogBinText(&head, sizeof(head));
    LogAlarm() << FNLog::LogBinText(head);


    LogAlarm() << "finish";

    return 0;
}
