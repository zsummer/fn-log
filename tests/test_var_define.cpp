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

 - define: define1 $ai
 - define: define____________ define1, $ai_trace 


 - var: spell = 1

 - channel: 0
    sync: sync
    - device:0
        disable: false
        out_type: screen
        priority: debug
    - device:1
        disable: true
        out_type: file
        priority: debug
        category: ${spell}

 - var: {ai=3, ai_trace=4}

 - channel: 1
    sync: sync
    - device:0
        disable: true
        out_type: file
        priority: debug
        category_wlist: define____________   # define____________->define1->$ai, ${ai_trace}-> 3,4

)----";

using namespace FNLog;
int main(int argc, char* argv[])
{
    int ret = FastStartDefaultLogger(test_text);
    if (ret != 0 )
    {
        return ret ;
    }

    LogInfo() << "channel[0] now:" << LogTimestamp();

    long long cat01 = FNLog::GetDefaultLogger().shm_->channels_[0].devices_[1].config_fields_[FNLog::DEVICE_CFG_CATEGORY];
    FNLOG_ASSERT(cat01 == 1, "");

    long long cat10 = FNLog::GetDefaultLogger().shm_->channels_[1].devices_[0].config_fields_[FNLog::DEVICE_CFG_CATEGORY_MASK];
    FNLOG_ASSERT(cat10 == ((1 << 3) | (1 <<4)), "");



    std::unique_ptr<FNLog::LexState> lsptr(new FNLog::LexState);
    FNLog::LexState& ls = *lsptr;

    //empty config 
    if (true)
    {
        std::string content = R"----(
                            - define: sdfdsf  234  
                            file:sdfdsf)----";
        InitState(ls);
        ls.first_ = content.c_str();
        ls.end_ = ls.first_ + content.length();
        ls.current_ = ls.first_;
        int ret = 0;
        ret = FNLog::Lex(ls);
        FNLOG_ASSERT(ret == FNLog::PEC_NONE, "");
        ret = FNLog::Lex(ls);
        FNLOG_ASSERT(ret == FNLog::PEC_NONE, "");

        FNLOG_ASSERT(ls.line_.key_ == FNLog::RK_DEFINE, "");
        FNLOG_ASSERT(FNLog::PredefinedMacro(ls, content) == FNLog::PEC_NONE, "");

        std::string sub = content.substr(content.length() - 10);
        FNLOG_ASSERT(sub.find("234") != std::string::npos, "");
    }

    if (true)
    {
        std::string content = R"----(
                            - define: listxxxx  2,3, 4  
                            file:listxxxx)----";
        InitState(ls);
        ls.first_ = content.c_str();
        ls.end_ = ls.first_ + content.length();
        ls.current_ = ls.first_;
        int ret = 0;
        ret = FNLog::Lex(ls);
        FNLOG_ASSERT(ret == FNLog::PEC_NONE, "");
        ret = FNLog::Lex(ls);
        FNLOG_ASSERT(ret == FNLog::PEC_NONE, "");

        FNLOG_ASSERT(ls.line_.key_ == FNLog::RK_DEFINE, "");
        FNLOG_ASSERT(FNLog::PredefinedMacro(ls, content) == FNLog::PEC_NONE, "");

        std::string sub = content.substr(content.length() - 10);
        FNLOG_ASSERT(sub.find("2,3, 4") != std::string::npos, "");
    }

    if (true)
    {
        std::string content = R"----(
                            - def: listxxxx  2,3, 4  
                            file:listxxxx)----";
        InitState(ls);
        ls.first_ = content.c_str();
        ls.end_ = ls.first_ + content.length();
        ls.current_ = ls.first_;
        int ret = 0;
        ret = FNLog::Lex(ls);
        FNLOG_ASSERT(ret == FNLog::PEC_NONE, "");
        ret = FNLog::Lex(ls);
        FNLOG_ASSERT(ret == FNLog::PEC_NONE, "");

        FNLOG_ASSERT(ls.line_.key_ == FNLog::RK_DEFINE, "");
        FNLOG_ASSERT(FNLog::PredefinedMacro(ls, content) == FNLog::PEC_NONE, "");

        std::string sub = content.substr(content.length() - 10);
        FNLOG_ASSERT(sub.find("2,3, 4") != std::string::npos, "");
    }

    if (true)
    {
        std::string content = R"----(
                            - def: listxxxx  2,3, 4  
                            file:1listxxxx)----"; //not match
        InitState(ls);
        ls.first_ = content.c_str();
        ls.end_ = ls.first_ + content.length();
        ls.current_ = ls.first_;
        int ret = 0;
        ret = FNLog::Lex(ls);
        FNLOG_ASSERT(ret == FNLog::PEC_NONE, "");
        ret = FNLog::Lex(ls);
        FNLOG_ASSERT(ret == FNLog::PEC_NONE, "");

        FNLOG_ASSERT(ls.line_.key_ == FNLog::RK_DEFINE, "");
        FNLOG_ASSERT(FNLog::PredefinedMacro(ls, content) == FNLog::PEC_NONE, "");

        std::string sub = content.substr(content.length() - 10);
        FNLOG_ASSERT(sub.find("2,3, 4") == std::string::npos, "");
    }

    if (true)
    {
        std::string content = R"----(
                            - def: listxxxx  2,3, 4  
                            file:listxxxx0)----"; //not match
        InitState(ls);
        ls.first_ = content.c_str();
        ls.end_ = ls.first_ + content.length();
        ls.current_ = ls.first_;
        int ret = 0;
        ret = FNLog::Lex(ls);
        FNLOG_ASSERT(ret == FNLog::PEC_NONE, "");
        ret = FNLog::Lex(ls);
        FNLOG_ASSERT(ret == FNLog::PEC_NONE, "");

        FNLOG_ASSERT(ls.line_.key_ == FNLog::RK_DEFINE, "");
        FNLOG_ASSERT(FNLog::PredefinedMacro(ls, content) == FNLog::PEC_NONE, "");

        std::string sub = content.substr(content.length() - 10);
        FNLOG_ASSERT(sub.find("2,3, 4") == std::string::npos, "");
    }
    if (true)
    {
        std::string content = R"----(
                            - def: listxxxx  2,3, 4  
                            file:listxxxxx)----"; //not match
        InitState(ls);
        ls.first_ = content.c_str();
        ls.end_ = ls.first_ + content.length();
        ls.current_ = ls.first_;
        int ret = 0;
        ret = FNLog::Lex(ls);
        FNLOG_ASSERT(ret == FNLog::PEC_NONE, "");
        ret = FNLog::Lex(ls);
        FNLOG_ASSERT(ret == FNLog::PEC_NONE, "");

        FNLOG_ASSERT(ls.line_.key_ == FNLog::RK_DEFINE, "");
        FNLOG_ASSERT(FNLog::PredefinedMacro(ls, content) == FNLog::PEC_NONE, "");

        std::string sub = content.substr(content.length() - 10);
        FNLOG_ASSERT(sub.find("2,3, 4") == std::string::npos, "");
    }

    if (true)
    {
        std::string content = R"----(
                            - def: listxxxx  2,3, 4  
                            file:llistxxxxx)----"; //not match
        InitState(ls);
        ls.first_ = content.c_str();
        ls.end_ = ls.first_ + content.length();
        ls.current_ = ls.first_;
        int ret = 0;
        ret = FNLog::Lex(ls);
        FNLOG_ASSERT(ret == FNLog::PEC_NONE, "");
        ret = FNLog::Lex(ls);
        FNLOG_ASSERT(ret == FNLog::PEC_NONE, "");

        FNLOG_ASSERT(ls.line_.key_ == FNLog::RK_DEFINE, "");
        FNLOG_ASSERT(FNLog::PredefinedMacro(ls, content) == FNLog::PEC_NONE, "");

        std::string sub = content.substr(content.length() - 10);
        FNLOG_ASSERT(sub.find("2,3, 4") == std::string::npos, "");
    }

    if (true)
    {
        std::string content = R"----(
                            - var: tag=  2,3, 4  
                            file:tag)----";
        InitState(ls);
        ls.first_ = content.c_str();
        ls.end_ = ls.first_ + content.length();
        ls.current_ = ls.first_;
        int ret = 0;
        ret = FNLog::Lex(ls);
        FNLOG_ASSERT(ret == FNLog::PEC_NONE, "");
        ret = FNLog::Lex(ls);
        FNLOG_ASSERT(ret == FNLog::PEC_NONE, "");

        FNLOG_ASSERT(ls.line_.key_ == FNLog::RK_VARIABLE, "");
        FNLOG_ASSERT(FNLog::PredefinedVar(ls, content) != FNLog::PEC_NONE, "");
    }

    if (true)
    {
        std::string content = R"----(
                            - var: tag=  2,tag1=3, tag2=4  
                            file:tag)----";
        InitState(ls);
        ls.first_ = content.c_str();
        ls.end_ = ls.first_ + content.length();
        ls.current_ = ls.first_;
        int ret = 0;
        ret = FNLog::Lex(ls);
        FNLOG_ASSERT(ret == FNLog::PEC_NONE, "");
        ret = FNLog::Lex(ls);
        FNLOG_ASSERT(ret == FNLog::PEC_NONE, "");

        FNLOG_ASSERT(ls.line_.key_ == FNLog::RK_VARIABLE, "");
        FNLOG_ASSERT(FNLog::PredefinedVar(ls, content) == FNLog::PEC_NONE, "");
        std::string sub = content.substr(content.length() - 10);
        FNLOG_ASSERT(sub.find("2") == std::string::npos, "");
    }

    if (true)
    {
        std::string content = R"----(
                            - var: tag=  2,tag1=3, tag2=4  
                            file:$tag1)----";
        InitState(ls);
        ls.first_ = content.c_str();
        ls.end_ = ls.first_ + content.length();
        ls.current_ = ls.first_;
        int ret = 0;
        ret = FNLog::Lex(ls);
        FNLOG_ASSERT(ret == FNLog::PEC_NONE, "");
        ret = FNLog::Lex(ls);
        FNLOG_ASSERT(ret == FNLog::PEC_NONE, "");

        FNLOG_ASSERT(ls.line_.key_ == FNLog::RK_VARIABLE, "");
        FNLOG_ASSERT(FNLog::PredefinedVar(ls, content) == FNLog::PEC_NONE, "");
        std::string sub = content.substr(content.length() - 10);
        FNLOG_ASSERT(sub.find("2") == std::string::npos, "");
    }


    if (true)
    {
        std::string content = R"----(
                            - var: tag=  2,tag1=3, tag2=4  
                            file:$tag)----";
        InitState(ls);
        ls.first_ = content.c_str();
        ls.end_ = ls.first_ + content.length();
        ls.current_ = ls.first_;
        int ret = 0;
        ret = FNLog::Lex(ls);
        FNLOG_ASSERT(ret == FNLog::PEC_NONE, "");
        ret = FNLog::Lex(ls);
        FNLOG_ASSERT(ret == FNLog::PEC_NONE, "");

        FNLOG_ASSERT(ls.line_.key_ == FNLog::RK_VARIABLE, "");
        FNLOG_ASSERT(FNLog::PredefinedVar(ls, content) == FNLog::PEC_NONE, "");
        std::string sub = content.substr(content.length() - 10);
        FNLOG_ASSERT(sub.find("2") != std::string::npos, "");
    }

    if (true)
    {
        std::string content = R"----(
                            - var: tag=  2,tag1=3, tag2=4  
                            file:$tag2)----";
        InitState(ls);
        ls.first_ = content.c_str();
        ls.end_ = ls.first_ + content.length();
        ls.current_ = ls.first_;
        int ret = 0;
        ret = FNLog::Lex(ls);
        FNLOG_ASSERT(ret == FNLog::PEC_NONE, "");
        ret = FNLog::Lex(ls);
        FNLOG_ASSERT(ret == FNLog::PEC_NONE, "");

        FNLOG_ASSERT(ls.line_.key_ == FNLog::RK_VARIABLE, "");
        FNLOG_ASSERT(FNLog::PredefinedVar(ls, content) == FNLog::PEC_NONE, "");
        std::string sub = content.substr(content.length() - 10);
        FNLOG_ASSERT(sub.find("2") == std::string::npos, "");
        FNLOG_ASSERT(sub.find("4") != std::string::npos, "");
    }

    if (true)
    {
        std::string content = R"----(
                            - var: {tag=  2,tag1=3, tag2=4}  
                            file:$tag2)----";
        InitState(ls);
        ls.first_ = content.c_str();
        ls.end_ = ls.first_ + content.length();
        ls.current_ = ls.first_;
        int ret = 0;
        ret = FNLog::Lex(ls);
        FNLOG_ASSERT(ret == FNLog::PEC_NONE, "");
        ret = FNLog::Lex(ls);
        FNLOG_ASSERT(ret == FNLog::PEC_NONE, "");

        FNLOG_ASSERT(ls.line_.key_ == FNLog::RK_VARIABLE, "");
        FNLOG_ASSERT(FNLog::PredefinedVar(ls, content) == FNLog::PEC_NONE, "");
        std::string sub = content.substr(content.length() - 10);
        FNLOG_ASSERT(sub.find("2") == std::string::npos, "");
        FNLOG_ASSERT(sub.find("4") != std::string::npos, "");
    }

    if (true)
    {
        std::string content = R"----(
                            - var: [tag=  2,tag1=3, tag2=4]  
                            file:$tag2)----";
        InitState(ls);
        ls.first_ = content.c_str();
        ls.end_ = ls.first_ + content.length();
        ls.current_ = ls.first_;
        int ret = 0;
        ret = FNLog::Lex(ls);
        FNLOG_ASSERT(ret == FNLog::PEC_NONE, "");
        ret = FNLog::Lex(ls);
        FNLOG_ASSERT(ret == FNLog::PEC_NONE, "");

        FNLOG_ASSERT(ls.line_.key_ == FNLog::RK_VARIABLE, "");
        FNLOG_ASSERT(FNLog::PredefinedVar(ls, content) == FNLog::PEC_NONE, "");
        std::string sub = content.substr(content.length() - 10);
        FNLOG_ASSERT(sub.find("2") == std::string::npos, "");
        FNLOG_ASSERT(sub.find("4") != std::string::npos, "");
    }

    if (true)
    {
        std::string content = R"----(
                            - var: "tag=  2,tag1=3, tag2=4"  
                            file:$tag2)----";
        InitState(ls);
        ls.first_ = content.c_str();
        ls.end_ = ls.first_ + content.length();
        ls.current_ = ls.first_;
        int ret = 0;
        ret = FNLog::Lex(ls);
        FNLOG_ASSERT(ret == FNLog::PEC_NONE, "");
        ret = FNLog::Lex(ls);
        FNLOG_ASSERT(ret == FNLog::PEC_NONE, "");

        FNLOG_ASSERT(ls.line_.key_ == FNLog::RK_VARIABLE, "");
        FNLOG_ASSERT(FNLog::PredefinedVar(ls, content) == FNLog::PEC_NONE, "");
        std::string sub = content.substr(content.length() - 10);
        FNLOG_ASSERT(sub.find("2") == std::string::npos, "");
        FNLOG_ASSERT(sub.find("4") != std::string::npos, "");
    }

    if (true)
    {
        std::string content = R"----(
                            - var: {tag="tag  ,tag1=3, tag2=4}  
                            file:$tag)----";
        InitState(ls);
        ls.first_ = content.c_str();
        ls.end_ = ls.first_ + content.length();
        ls.current_ = ls.first_;
        int ret = 0;
        ret = FNLog::Lex(ls);
        FNLOG_ASSERT(ret == FNLog::PEC_NONE, "");
        ret = FNLog::Lex(ls);
        FNLOG_ASSERT(ret == FNLog::PEC_NONE, "");

        FNLOG_ASSERT(ls.line_.key_ == FNLog::RK_VARIABLE, "");
        FNLOG_ASSERT(FNLog::PredefinedVar(ls, content) == FNLog::PEC_NONE, "");
        std::string sub = content.substr(content.length() - 10);
        FNLOG_ASSERT(sub.find("$tag") == std::string::npos, "");
        FNLOG_ASSERT(sub.find("\"tag") != std::string::npos, "");
    }


    if (true)
    {
        std::string content = R"----(
                            - var: "tag=  2,tag1=3, tag2=4  
                            file:$tag2)----";
        InitState(ls);
        ls.first_ = content.c_str();
        ls.end_ = ls.first_ + content.length();
        ls.current_ = ls.first_;
        int ret = 0;
        ret = FNLog::Lex(ls);
        FNLOG_ASSERT(ret == FNLog::PEC_NONE, "");
        ret = FNLog::Lex(ls);
        FNLOG_ASSERT(ret != FNLog::PEC_NONE, "");
    }

    if (true)
    {
        std::string content = R"----(
                            - var: {tag=  2,tag1=3, tag2=4  
                            file:$tag2)----";
        InitState(ls);
        ls.first_ = content.c_str();
        ls.end_ = ls.first_ + content.length();
        ls.current_ = ls.first_;
        int ret = 0;
        ret = FNLog::Lex(ls);
        FNLOG_ASSERT(ret == FNLog::PEC_NONE, "");
        ret = FNLog::Lex(ls);
        FNLOG_ASSERT(ret != FNLog::PEC_NONE, "");
    }




    //---------- ill syntax  ------

    if (true)
    {
        std::string content =
            R"----(
        - var: {tag2=  2}  
        file:$tag2)----";

        InitState(ls);
        ls.first_ = content.c_str();
        ls.end_ = ls.first_ + content.length();
        ls.current_ = ls.first_;
        int ret = 0;
        ret = FNLog::Lex(ls);
        FNLOG_ASSERT(ret == FNLog::PEC_NONE, "");
        ret = FNLog::Lex(ls);
        FNLOG_ASSERT(ret == FNLog::PEC_NONE, "");
        FNLOG_ASSERT(ls.line_.key_ == FNLog::RK_VARIABLE, "");
        FNLOG_ASSERT(FNLog::PredefinedVar(ls, content) == FNLog::PEC_NONE, "");//tag2 is key
    }

    if (true)
    {
        std::string content = 
        R"----(
        - var: {0tag=  2}  
        file:$tag2)----";

        InitState(ls);
        ls.first_ = content.c_str();
        ls.end_ = ls.first_ + content.length();
        ls.current_ = ls.first_;
        int ret = 0;
        ret = FNLog::Lex(ls);
        FNLOG_ASSERT(ret == FNLog::PEC_NONE, "");
        ret = FNLog::Lex(ls);
        FNLOG_ASSERT(ret == FNLog::PEC_NONE, "");  
        FNLOG_ASSERT(ls.line_.key_ == FNLog::RK_VARIABLE, "");
        FNLOG_ASSERT(FNLog::PredefinedVar(ls, content) != FNLog::PEC_NONE, "");//0tag not key
    }




    LogAlarm() << "finish";

    return 0;
}
