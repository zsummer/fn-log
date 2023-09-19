

#define FN_LOG_MAX_DEVICE_SIZE 36
#define FN_LOG_MAX_CHANNEL_SIZE 3
#define FN_LOG_MAX_LOG_SIZE 1000
#define FN_LOG_MAX_LOG_QUEUE_SIZE 100000

#include "fn_log.h"

static const std::string config_text1 =
R"----( # line1
 - channel: 0
    priority: trace
    -device: 0
        disable: false
        out_type: screen
        1priority: trace  #line = 17 -11 + 1 = 7  
)----";

static const std::string config_text2 =
R"----( # line1
 - channel: 0
    priority: trace
    -device: 0
        disable: false
        out_type: screen
        1priority: trace  #line = 17 -11 + 1 = 7  )----";

static const std::string config_text3 =
R"----( # line1
 - channel: 0
    priority: trace
    -device: 0
        disable: false
        out_type: screen
        priority: trace  
    -1device: #error line 8
)----";


static const std::string config_text4 =
R"----( # line1
 - channel: 0
    priority: trace
    -device: 0
        disable: false
        out_type: screen
        priority: trace  
    -device: 1
 - 1channel: # error line 9
)----";


static const std::string config_text5 =
R"----( # line1
 - channel: 0
    priority: trace
    -device: 0
        disable: false
        out_type: screen
        priority: trace  

 - 1channel: # error line 9
)----";





static const std::string config_text10 =
R"----(#line1
 - channel: 0
    priority: trace
    -device: 0
        disable: false
        out_type: screen
        priority: trace

 - channel: 1
    -device:0
        disable: false
        out_type: empty
        priority: debug
        category: 20
        category_extend: 1
        idenfity: 20
        idenfity_extend: 1

 - channel: 2
    -device:0
        disable: false
        out_type: empty
        priority: error
        category: 0
        category_extend: 1
        idenfity: 0
        idenfity_extend: 1



 - def:xxxxx 
)----";




#define Now()  std::chrono::duration<double>(std::chrono::system_clock::now().time_since_epoch()).count()

#define FNLOG_ASSERT(expr, prefix) \
{ \
    if (expr) \
    { \
        LogInfo() << "test " << prefix << " success."; \
    } \
    else \
    {   \
        LogError() << "test " << prefix << " failed. "; \
        return __LINE__ * -1; \
    } \
}


int main(int argc, char* argv[])
{
    int ret = FNLog::FastStartDefaultLogger();
    if (ret != 0)
    {
        return ret;
    }

    std::unique_ptr<FNLog::LexState> ls(new FNLog::LexState);
    std::string text;


    text = "";
    ret = FNLog::ParseLogger(*ls, text);
    FNLOG_ASSERT(ret == 0, "");
    FNLOG_ASSERT(ls->line_no_ == 1, "");

    text = " ";
    ret = FNLog::ParseLogger(*ls, text);
    FNLOG_ASSERT(ret == 0, "");
    FNLOG_ASSERT(ls->line_no_ == 2, "");


    text = "\n";
    ret = FNLog::ParseLogger(*ls, text);
    FNLOG_ASSERT(ret == 0, "");
    FNLOG_ASSERT(ls->line_no_ == 2, "");

    text = "\r\n";
    ret = FNLog::ParseLogger(*ls, text);
    FNLOG_ASSERT(ret == 0, "");
    FNLOG_ASSERT(ls->line_no_ == 2, "");

    text = "\n\r";
    ret = FNLog::ParseLogger(*ls, text);
    FNLOG_ASSERT(ret == 0, "");
    FNLOG_ASSERT(ls->line_no_ == 2, "");

    text = "\n\n";
    ret = FNLog::ParseLogger(*ls, text);
    FNLOG_ASSERT(ret == 0, "");
    FNLOG_ASSERT(ls->line_no_ == 3, "");

    text = "\r\r";
    ret = FNLog::ParseLogger(*ls, text);
    FNLOG_ASSERT(ret == 0, "");
    FNLOG_ASSERT(ls->line_no_ == 3, "");

    text = "\r\n\r";
    ret = FNLog::ParseLogger(*ls, text);
    FNLOG_ASSERT(ret == 0, "");
    FNLOG_ASSERT(ls->line_no_ == 3, "");

    text = "\n\r\n";
    ret = FNLog::ParseLogger(*ls, text);
    FNLOG_ASSERT(ret == 0, "");
    FNLOG_ASSERT(ls->line_no_ == 3, "");


    text = " #line1   \n\r    line2 #line 2 error ";
    ret = FNLog::ParseLogger(*ls, text);
    FNLOG_ASSERT(ret != 0, "");
    FNLOG_ASSERT(ls->line_no_ == 2, "");


    text = " #line1   \n\r    #line2 \r\n  #line 3 \r  #line 4 \n #line5 \n\n\r\n  #line8 \n fdasfasdfasdf #line 9 error";
    ret = FNLog::ParseLogger(*ls, text);
    FNLOG_ASSERT(ret != 0, "");
    FNLOG_ASSERT(ls->line_no_ == 9, "");


    text = " #line1   \n\r    #line2 \r\n  #line 3 \r  #line 4 \n #line5 \n\n\r\n  #line8 \n fdasfasdfasdf #line 9 error\n\r\n\n";
    ret = FNLog::ParseLogger(*ls, text);
    FNLOG_ASSERT(ret != 0, "");
    FNLOG_ASSERT(ls->line_no_ == 9, "");


    text = " #line1   \n\r -def:xxx   #line2 \r\n \n\r\n\n";
    ret = FNLog::ParseLogger(*ls, text);
    FNLOG_ASSERT(ret != 0, "");
    FNLOG_ASSERT(ls->line_no_ == 2, "");

    text = " #line1   \n\r -var:xxx   #line2 \r\n \n\r\n\n";
    ret = FNLog::ParseLogger(*ls, text);
    FNLOG_ASSERT(ret != 0, "");
    FNLOG_ASSERT(ls->line_no_ == 2, "");

    text = " #line1   \n\r channel:999   #line2 \r\n \n\r\n\n";
    ret = FNLog::ParseLogger(*ls, text);
    FNLOG_ASSERT(ret != 0, "");
    FNLOG_ASSERT(ls->line_no_ == 2, "");

    text = "channel:0#line1\n\r  -device:999  #line2 \r\n \n\r\n\n";
    ret = FNLog::ParseLogger(*ls, text);
    FNLOG_ASSERT(ret != 0, "");
    FNLOG_ASSERT(ls->line_no_ == 2, "");

    text = "channel:0#line1\n\r  -device:0  #line2 \r\n    disable:false\n    udp_addr:xxx     \n\r\n\n";
    ret = FNLog::ParseLogger(*ls, text);
    FNLOG_ASSERT(ret != 0, "");
    FNLOG_ASSERT(ls->line_no_ == 4, "");


    text = config_text1;
    ret = FNLog::ParseLogger(*ls, text);
    FNLOG_ASSERT(ret != 0, "");
    FNLOG_ASSERT(ls->line_no_ == 7, "");

    text = config_text2;
    ret = FNLog::ParseLogger(*ls, text);
    FNLOG_ASSERT(ret != 0, "");
    FNLOG_ASSERT(ls->line_no_ == 7, "");

    text = config_text3;
    ret = FNLog::ParseLogger(*ls, text);
    FNLOG_ASSERT(ret != 0, "");
    FNLOG_ASSERT(ls->line_no_ == 8, "");

    text = config_text4;
    ret = FNLog::ParseLogger(*ls, text);
    FNLOG_ASSERT(ret != 0, "");
    FNLOG_ASSERT(ls->line_no_ == 9, "");

    text = config_text5;
    ret = FNLog::ParseLogger(*ls, text);
    FNLOG_ASSERT(ret != 0, "");
    FNLOG_ASSERT(ls->line_no_ == 9, "");

    text = config_text10;
    ret = FNLog::ParseLogger(*ls, text);
    FNLOG_ASSERT(ret != 0, "");
    FNLOG_ASSERT(ls->line_no_ == 31, "");

    return 0;
}

