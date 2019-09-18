#include "fn_log.h"

static const std::string default_config_text =
R"----(
 # default is sync write channel.  
 - channel: 0
    sync: sync
    -device:0
        disable: false
        out_type: file
    -device:1
        disable: false
        out_type: file
 - channel: 1
   sync: sync
   -device:0
        disable: false
        out_type: screen
)----";

#define FNLOG_ASSERT(expr, prefix) \
{ \
    if (expr) \
    { \
        LogInfoStream(1, 0) << "test " << prefix << " success."; \
    } \
    else \
    {   \
        LogErrorStream(1, 0) << "test " << prefix << " failed."; \
        return __LINE__ * -1; \
    } \
}

long long ChannelWrite(int channel_id)
{
    return FNLog::GetChannelLog(FNLog::GetDefaultLogger(), channel_id, FNLog::CHANNEL_LOG_PROCESSED);
}
long long DeviceWrite(int channel_id, int device_id)
{
    return FNLog::GetDeviceLog(FNLog::GetDefaultLogger(), 
        channel_id, device_id, FNLog::DEVICE_LOG_TOTAL_WRITE_LINE);
}
using namespace FNLog;
int main(int argc, char* argv[])
{
    int ret = FNLog::FastStartDefaultLogger(default_config_text);
    if (ret != 0)
    {
        return ret;
    }
    LogAlarmStream(1, 0) << "1";
    LogAlarmStream(0, 0) << "1";
    FNLOG_ASSERT(ChannelWrite(0) == DeviceWrite(1, 0), "channel begin");
    FNLOG_ASSERT(ChannelWrite(0) > 0, "channel begin");
    FNLOG_ASSERT(ChannelWrite(0) == DeviceWrite(0, 0), "channel begin");
    FNLOG_ASSERT(ChannelWrite(0) == DeviceWrite(0, 1), "channel begin");

    long long last_channel_write = ChannelWrite(0);
    long long last_device_write = DeviceWrite(0, 0);
    LogTraceStream(0, 0);
    LogDebugStream(0, 1);
    LogInfoStream(0, 2);
    LogWarnStream(0, 3);
    LogErrorStream(0, 4);
    LogAlarmStream(0, 5);
    LogFatalStream(0, 6);
    FNLOG_ASSERT(ChannelWrite(0) - last_channel_write == 7, "all log stream test ");
    FNLOG_ASSERT(DeviceWrite(0, 0) - last_device_write == 7, "all log stream test ");
    FNLOG_ASSERT(DeviceWrite(0, 0) == ChannelWrite(0), "all log stream test ");

    last_channel_write = ChannelWrite(0);
    last_device_write = DeviceWrite(0, 0);
    SetDeviceConfig(GetDefaultLogger(), 0, 0, DEVICE_CFG_PRIORITY, PRIORITY_ERROR);
    LogTraceStream(0, 0);
    LogDebugStream(0, 1);
    LogInfoStream(0, 2);
    LogWarnStream(0, 3);
    LogErrorStream(0, 4);
    LogAlarmStream(0, 5);
    LogFatalStream(0, 6);
    FNLOG_ASSERT(ChannelWrite(0) - last_channel_write == 7, "priority test ");
    FNLOG_ASSERT(DeviceWrite(0, 0) - last_device_write == 3, "priority test ");

    SetDeviceConfig(GetDefaultLogger(), 0, 0, DEVICE_CFG_PRIORITY, PRIORITY_TRACE);
    SetDeviceConfig(GetDefaultLogger(), 0, 0, DEVICE_CFG_CATEGORY, 1);
    last_channel_write = ChannelWrite(0);
    last_device_write = DeviceWrite(0, 0);
    LogTraceStream(0, 0);
    LogDebugStream(0, 1);
    LogInfoStream(0, 2);
    FNLOG_ASSERT(ChannelWrite(0) - last_channel_write == 3, "category test ");
    FNLOG_ASSERT(DeviceWrite(0, 0) - last_device_write == 1, "category test ");

    SetDeviceConfig(GetDefaultLogger(), 0, 0, DEVICE_CFG_CATEGORY_EXTEND, 1);
    last_channel_write = ChannelWrite(0);
    last_device_write = DeviceWrite(0, 0);
    LogTraceStream(0, 0);
    LogDebugStream(0, 1);
    LogInfoStream(0, 2);
    FNLOG_ASSERT(ChannelWrite(0) - last_channel_write == 3, "category test ");
    FNLOG_ASSERT(DeviceWrite(0, 0) - last_device_write == 2, "category test ");

    SetDeviceConfig(GetDefaultLogger(), 0, 0, DEVICE_CFG_CATEGORY, 0);
    last_channel_write = ChannelWrite(0);
    last_device_write = DeviceWrite(0, 0);
    LogTraceStream(0, 0);
    LogDebugStream(0, 1);
    LogInfoStream(0, 2);
    FNLOG_ASSERT(ChannelWrite(0) - last_channel_write == 3, "category test ");
    FNLOG_ASSERT(DeviceWrite(0, 0) - last_device_write == 3, "category test ");

    SetChannelConfig(GetDefaultLogger(), 0, CHANNEL_CFG_CATEGORY, 2);
    SetChannelConfig(GetDefaultLogger(), 0, CHANNEL_CFG_CATEGORY_EXTEND, 2);
    SetChannelConfig(GetDefaultLogger(), 0, CHANNEL_CFG_PRIORITY, PRIORITY_ERROR);
    last_channel_write = ChannelWrite(0);
    last_device_write = DeviceWrite(0, 0);
    LogTraceStream(0, 0);
    LogDebugStream(0, 1);
    LogInfoStream(0, 2);
    LogWarnStream(0, 3);
    LogErrorStream(0, 4);
    LogAlarmStream(0, 5);
    LogFatalStream(0, 6);
    FNLOG_ASSERT(ChannelWrite(0) - last_channel_write == 1, "channel category and priority test ");
    FNLOG_ASSERT(DeviceWrite(0, 0) - last_device_write == 1, "channel category and priority test ");

    SetChannelConfig(GetDefaultLogger(), 0, CHANNEL_CFG_CATEGORY, 0);
    SetChannelConfig(GetDefaultLogger(), 0, CHANNEL_CFG_PRIORITY, PRIORITY_TRACE);


    last_channel_write = ChannelWrite(0);
    last_device_write = DeviceWrite(0, 0);
    LogTrace();
    LogDebug();
    LogInfo();
    LogWarn();
    LogError();
    LogAlarm();
    LogFatal();

    LogTrace()<< "dd" << 0.1f << 2;
    LogDebug()<< "dd" << 0.1f << 2;
    LogInfo() << "dd" << 0.1f << 2;
    LogWarn() << "dd" << 0.1f << 2;
    LogError()<< "dd" << 0.1f << 2;
    LogAlarm()<< "dd" << 0.1f << 2;
    LogFatal()<< "dd" << 0.1f << 2;

    LogTracePack(0, 0);
    LogDebugPack(0, 0);
    LogInfoPack(0,  0);
    LogWarnPack(0,  0);
    LogErrorPack(0, 0);
    LogAlarmPack(0, 0);
    LogFatalPack(0, 0);

    LogTracePack(0, 0, "aa:", -2.3f, "int:", 3);
    LogDebugPack(0, 0, "aa:", -2.3f, "int:", 3);
    LogInfoPack(0,  0, "aa:", -2.3f, "int:", 3);
    LogWarnPack(0,  0, "aa:", -2.3f, "int:", 3);
    LogErrorPack(0, 0, "aa:", -2.3f, "int:", 3);
    LogAlarmPack(0, 0, "aa:", -2.3f, "int:", 3);
    LogFatalPack(0, 0, "aa:", -2.3f, "int:", 3);

    LOGT("dd" << 3.0 << nullptr);
    LOGD("dd" << 3.0 << nullptr);
    LOGI("dd" << 3.0 << nullptr);
    LOGW("dd" << 3.0 << nullptr);
    LOGE("dd" << 3.0 << nullptr);
    LOGA("dd" << 3.0 << nullptr);
    LOGF("dd" << 3.0 << nullptr);

    LOGFMTT("dd:%s, ch:<%c>", "ee", 'c');
    LOGFMTD("dd:%s, ch:<%c>", "ee", 'c');
    LOGFMTI("dd:%s, ch:<%c>", "ee", 'c');
    LOGFMTW("dd:%s, ch:<%c>", "ee", 'c');
    LOGFMTE("dd:%s, ch:<%c>", "ee", 'c');
    LOGFMTA("dd:%s, ch:<%c>", "ee", 'c');
    LOGFMTF("dd:%s, ch:<%c>", "ee", 'c');

    FNLOG_ASSERT(ChannelWrite(0) - last_channel_write == 42, "all style test ");
    FNLOG_ASSERT(DeviceWrite(0, 0) - last_device_write == 42, "all style test ");

    LogInfoStream(1, 0) << "log filter test success";

    LogInfoStream(1, 0).write_binary("fn-log", sizeof("fn-log") - 1);


    return 0;
}