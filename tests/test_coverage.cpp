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
 - define: "tag0 0"
 - define: "tag2 2"
 - channel: 0
    sync: null
    - device:0
        disable: false
        out_type: screen
        priority: xxxxxxx
    - device:1
        disable: false
        out_type: screen
        priority: null
    - device:2
        disable: false
        out_type: screen
        priority: trace
    - device:3
        disable: false
        out_type: screen
        priority: debug
    - device:4
        disable: false
        out_type: screen
        priority: info
    - device:5
        disable: false
        out_type: screen
        priority: warn
    - device:6
        disable: false
        out_type: screen
        priority: error
    - device:7
        disable: false
        out_type: screen
        priority: alarm
    - device:8
        disable: false
        out_type: screen
        priority: fatal

)----";

using namespace FNLog;
int main(int argc, char* argv[])
{
    int ret = FastStartDefaultLogger();
    if (ret != 0 )
    {
        return ret ;
    }


    LogAlarm() << "log init success";  //sync write
    if (true)
    {
        time_t now = time(NULL);
        struct tm* ptm = localtime(&now);
        std::string result = MakeFileName("$PID$YEAR$MON$DAY$HOUR$MIN$SEC", 0, 0, *ptm);
        FNLOG_ASSERT(std::regex_match(result, std::regex("[0-9]*\\.log")), "filename fmt");        
        result = MakeFileName("$PNAME", 0, 0, *ptm);
        FNLOG_ASSERT(std::regex_match(result, std::regex("test_coverage\\.log")), "filename fmt");
        result = MakeFileName("", 0, 0, *ptm);
        FNLOG_ASSERT(std::regex_match(result, std::regex("test_coverage.*log")), "filename fmt");
        result = MakeFileName("$a$dddddd$PID$", 0, 0, *ptm);
        FNLOG_ASSERT(std::regex_search(result, std::regex("\\$a")), "filename fmt");
        FNLOG_ASSERT(!std::regex_search(result, std::regex("PID")), "filename fmt");
        result = MakeFileName("dddd", 0, 0, *ptm);
        FNLOG_ASSERT(std::regex_match(result, std::regex("dddd\\.log")), "filename fmt");
    }

    if (true)
    {
        char* pbig = new char[FN_LOG_MAX_LOG_SIZE + 100];
        for (int i = 0; i < FN_LOG_MAX_LOG_SIZE + 100; i++)
        {
            pbig[i] = '1';
        }
        pbig[FN_LOG_MAX_LOG_SIZE + 99] = '\0';
        LOGFMTI("%s", pbig);
        LogInfo() << pbig;
        PackInfo(pbig);
        
    }
    
    if (true)
    {
        static Logger logger;
        int ret = InitFromYMAL(logger, "ddddddd", "");
        FNLOG_ASSERT(ret != 0, "init error");
        ret = InitFromYMAL(logger, test_text, "");
        FNLOG_ASSERT(ret == 0, "init error");
        FNLOG_ASSERT(GetDeviceConfig(logger, 0, 0, DEVICE_CFG_PRIORITY) == PRIORITY_TRACE, "parse priority");
        FNLOG_ASSERT(GetDeviceConfig(logger, 0, 1, DEVICE_CFG_PRIORITY) == PRIORITY_TRACE, "parse priority");
        FNLOG_ASSERT(GetDeviceConfig(logger, 0, 2, DEVICE_CFG_PRIORITY) == PRIORITY_TRACE, "parse priority");
        FNLOG_ASSERT(GetDeviceConfig(logger, 0, 3, DEVICE_CFG_PRIORITY) == PRIORITY_DEBUG, "parse priority");
        FNLOG_ASSERT(GetDeviceConfig(logger, 0, 4, DEVICE_CFG_PRIORITY) == PRIORITY_INFO, "parse priority");
        FNLOG_ASSERT(GetDeviceConfig(logger, 0, 5, DEVICE_CFG_PRIORITY) == PRIORITY_WARN, "parse priority");
        FNLOG_ASSERT(GetDeviceConfig(logger, 0, 6, DEVICE_CFG_PRIORITY) == PRIORITY_ERROR, "parse priority");
        FNLOG_ASSERT(GetDeviceConfig(logger, 0, 7, DEVICE_CFG_PRIORITY) == PRIORITY_ALARM, "parse priority");
        FNLOG_ASSERT(GetDeviceConfig(logger, 0, 8, DEVICE_CFG_PRIORITY) == PRIORITY_FATAL, "parse priority");
    }                                                                      


    LogInfo() << 1.23f / 0.000000001;
    LogInfo() << 1.23f / 0.00000001;
    LogInfo() << 1.23f / 0.0000001;
    LogInfo() << 1.23f / 0.000001;
    LogInfo() << 1.23f / 0.00001;
    LogInfo() << 1.23f / 0.0001;
    LogInfo() << 1.23f / 0.001;
    LogInfo() << 1.23f / 0.1;
    LogInfo() << 1.23f / 1;
    LogInfo() << 1.23f / 10;
    LogInfo() << 1.23f / 100;
    LogInfo() << 1.23f / 1000;
    LogInfo() << 1.23f / 10000;
    LogInfo() << 1.23f / 100000;
    LogInfo() << 1.23f / 1000000;
    LogInfo() << 1.23f / 10000000;
    LogInfo() << 1.23f / 100000000;
    LogInfo() << 1.23 / 0.000000001;
    LogInfo() << 1.23 / 0.00000001;
    LogInfo() << 1.23 / 0.0000001;
    LogInfo() << 1.23 / 0.000001;
    LogInfo() << 1.23 / 0.00001;
    LogInfo() << 1.23 / 0.0001;
    LogInfo() << 1.23 / 0.001;
    LogInfo() << 1.23 / 0.1;
    LogInfo() << 1.23 / 1;
    LogInfo() << 1.23 / 10;
    LogInfo() << 1.23 / 100;
    LogInfo() << 1.23 / 1000;
    LogInfo() << 1.23 / 10000;
    LogInfo() << 1.23 / 100000;
    LogInfo() << 1.23 / 1000000;
    LogInfo() << 1.23 / 10000000;
    LogInfo() << 1.23 / 100000000;

    LogInfo() << LogPercent(1.23 / 0.000000001);
    LogInfo() << LogPercent(1.23 / 0.00000001);
    LogInfo() << LogPercent(1.23 / 0.0000001);
    LogInfo() << LogPercent(1.23 / 0.000001);
    LogInfo() << LogPercent(1.23 / 0.00001);
    LogInfo() << LogPercent(1.23 / 0.0001);
    LogInfo() << LogPercent(1.23 / 0.001);
    LogInfo() << LogPercent(1.23 / 0.1);
    LogInfo() << LogPercent(1.23 / 1);
    LogInfo() << LogPercent(1.23 / 10);
    LogInfo() << LogPercent(1.23 / 100);
    LogInfo() << LogPercent(1.23 / 1000);
    LogInfo() << LogPercent(1.23 / 10000);
    LogInfo() << LogPercent(1.23 / 100000);
    LogInfo() << LogPercent(1.23 / 1000000);
    LogInfo() << LogPercent(1.23 / 10000000);
    LogInfo() << LogPercent(1.23 / 100000000);
    LogInfo() << LogPercent(1.23 / 1000000000) ;


    LogInfo() << "now:" << LogTimestamp();
    LogInfo() << "now:" << LogTimestamp(0);
    LogAlarm() << "finish";

    return 0;
}
