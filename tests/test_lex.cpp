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

 - define: "define1:$ai"
 - define: {define2:define1, define3__:$ai_trace, }

 - var: {spell:1}

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

 - var: {ai:3, ai_trace:4}

 - channel: 1
    sync: sync
    - device:0
        disable: true
        out_type: file
        priority: debug
        category_wlist: define2, define3__  # define2->define1->$ai->3,  define3__->${ai_trace}->4

)----";



using namespace FNLog;
int main(int argc, char* argv[])
{
    int ret = FastStartDefaultLogger();
    if (ret != 0 )
    {
        return ret ;
    }

    std::unique_ptr<FNLog::LexState> lsptr(new FNLog::LexState);
    FNLog::LexState& ls = *lsptr;

    //empty config 
    if (true)
    {
        std::string content = "  ";
        InitState(ls);
        ls.first_ = content.c_str();
        ls.end_ = ls.first_ + content.length();
        ls.current_ = ls.first_;
        int ret = 0;
        ret = FNLog::Lex(ls);

        FNLOG_ASSERT(ret == FNLog::PEC_NONE, "");
        FNLOG_ASSERT(ls.line_no_ == 1, "");
        FNLOG_ASSERT(ls.line_.key_end_ == ls.line_.key_begin_, "");
        FNLOG_ASSERT(ls.line_.val_end_ == ls.line_.val_begin_, "");
        FNLOG_ASSERT(ls.current_ <= ls.end_, "");
    }


    if (true)
    {
        std::string content = "\r\n\r\n";
        InitState(ls);
        ls.first_ = content.c_str();
        ls.end_ = ls.first_ + content.length();
        ls.current_ = ls.first_;
        int ret = 0;

        ret = FNLog::Lex(ls);
        FNLOG_ASSERT(ret == FNLog::PEC_NONE, "");
        FNLOG_ASSERT(ls.line_no_ == 1, "");
        FNLOG_ASSERT(ls.line_.key_end_ == ls.line_.key_begin_, "");
        FNLOG_ASSERT(ls.line_.val_end_ == ls.line_.val_begin_, "");
        FNLOG_ASSERT(ls.current_ < ls.end_, "");

        ret = FNLog::Lex(ls);
        FNLOG_ASSERT(ret == FNLog::PEC_NONE, "");
        FNLOG_ASSERT(ls.line_no_ == 2, "");
        FNLOG_ASSERT(ls.line_.key_end_ == ls.line_.key_begin_, "");
        FNLOG_ASSERT(ls.line_.val_end_ == ls.line_.val_begin_, "");
        FNLOG_ASSERT(ls.current_ == ls.end_, "");

        ret = FNLog::Lex(ls);
        FNLOG_ASSERT(ret == FNLog::PEC_NONE, "");
        FNLOG_ASSERT(ls.line_no_ == 2, "");
        FNLOG_ASSERT(ls.line_.key_end_ == ls.line_.key_begin_, "");
        FNLOG_ASSERT(ls.line_.val_end_ == ls.line_.val_begin_, "");
        FNLOG_ASSERT(ls.current_ == ls.end_, "");

        ret = FNLog::Lex(ls);
        FNLOG_ASSERT(ret == FNLog::PEC_NONE, "");
        FNLOG_ASSERT(ls.line_no_ == 2, "");
        FNLOG_ASSERT(ls.line_.key_end_ == ls.line_.key_begin_, "");
        FNLOG_ASSERT(ls.line_.val_end_ == ls.line_.val_begin_, "");
        FNLOG_ASSERT(ls.current_ == ls.end_, "");
    }

    {
        std::string content = "#dfsdf\r\n";
        InitState(ls);
        ls.first_ = content.c_str();
        ls.end_ = ls.first_ + content.length();
        ls.current_ = ls.first_;
        int ret = 0;

        ret = FNLog::Lex(ls);
        FNLOG_ASSERT(ret == FNLog::PEC_NONE, "");
        FNLOG_ASSERT(ls.line_no_ == 1, "");
        FNLOG_ASSERT(ls.line_.key_end_ == ls.line_.key_begin_, "");
        FNLOG_ASSERT(ls.line_.val_end_ == ls.line_.val_begin_, "");
        FNLOG_ASSERT(ls.current_ == ls.end_, "");
    }

    {
        std::string content = " \t  #dfsdf\r\n";
        InitState(ls);
        ls.first_ = content.c_str();
        ls.end_ = ls.first_ + content.length();
        ls.current_ = ls.first_;
        ls.line_no_ = 0;
        int ret = 0;

        ret = FNLog::Lex(ls);
        FNLOG_ASSERT(ret == FNLog::PEC_NONE, "");
        FNLOG_ASSERT(ls.line_no_ == 1, "");
        FNLOG_ASSERT(ls.line_.key_end_ == ls.line_.key_begin_, "");
        FNLOG_ASSERT(ls.line_.val_end_ == ls.line_.val_begin_, "");
        FNLOG_ASSERT(ls.current_ == ls.end_, "");
    }

    {
        std::string content = " file: {::\": }  #dfsdf";
        InitState(ls);
        ls.first_ = content.c_str();
        ls.end_ = ls.first_ + content.length();
        ls.current_ = ls.first_;
        int ret = 0;

        ret = FNLog::Lex(ls);
        FNLOG_ASSERT(ret == FNLog::PEC_NONE, "");
        FNLOG_ASSERT(ls.line_no_ == 1, "");
        FNLOG_ASSERT(ls.line_.key_end_ > ls.line_.key_begin_, "");
        FNLOG_ASSERT(ls.line_.val_end_ > ls.line_.val_begin_, "");
        FNLOG_ASSERT(ls.current_ == ls.end_, "");

        FNLOG_ASSERT(std::string(ls.line_.key_begin_, ls.line_.key_end_ - ls.line_.key_begin_) == "file", "");
        FNLOG_ASSERT(std::string(ls.line_.val_begin_, ls.line_.val_end_ - ls.line_.val_begin_) == "::\":", "");
    }

    {
        std::string content = " file  : \",::{}[]: \"";
        InitState(ls);
        ls.first_ = content.c_str();
        ls.end_ = ls.first_ + content.length();
        ls.current_ = ls.first_;
        ls.line_no_ = 0;
        int ret = 0;

        ret = FNLog::Lex(ls);
        FNLOG_ASSERT(ret == FNLog::PEC_NONE, "");
        FNLOG_ASSERT(ls.line_no_ == 1, "");
        FNLOG_ASSERT(ls.line_.key_end_ > ls.line_.key_begin_, "");
        FNLOG_ASSERT(ls.line_.val_end_ > ls.line_.val_begin_, "");
        FNLOG_ASSERT(ls.current_ == ls.end_, "");

        FNLOG_ASSERT(std::string(ls.line_.key_begin_, ls.line_.key_end_ - ls.line_.key_begin_) == "file", "");
        FNLOG_ASSERT(std::string(ls.line_.val_begin_, ls.line_.val_end_ - ls.line_.val_begin_) == ",::{}[]:", "");
    }

    {
        std::string content = " file  : \",::{}[]: \" \n   \r  \r\n #dfsdf";
        InitState(ls);
        ls.first_ = content.c_str();
        ls.end_ = ls.first_ + content.length();
        ls.current_ = ls.first_;
        int ret = 0;

        ret = FNLog::Lex(ls);
        FNLOG_ASSERT(ret == FNLog::PEC_NONE, "");
        ret = FNLog::Lex(ls);
        FNLOG_ASSERT(ret == FNLog::PEC_NONE, "");
        ret = FNLog::Lex(ls);
        FNLOG_ASSERT(ret == FNLog::PEC_NONE, "");
        ret = FNLog::Lex(ls);
        FNLOG_ASSERT(ret == FNLog::PEC_NONE, "");
        ret = FNLog::Lex(ls);
        FNLOG_ASSERT(ret == FNLog::PEC_NONE, "");
        FNLOG_ASSERT(ls.line_no_ == 4, "");
    }


    {
        std::string content = " -file  : \" path  \"";
        InitState(ls);
        ls.first_ = content.c_str();
        ls.end_ = ls.first_ + content.length();
        ls.current_ = ls.first_;
        int ret = 0;

        ret = FNLog::Lex(ls);
        FNLOG_ASSERT(ret == FNLog::PEC_NONE, "");
        FNLOG_ASSERT(std::string(ls.line_.key_begin_, ls.line_.key_end_ - ls.line_.key_begin_) == "file", "");
        FNLOG_ASSERT(std::string(ls.line_.val_begin_, ls.line_.val_end_ - ls.line_.val_begin_) == "path", "");
    }

    {
        std::string content = " -file  : \"   \"";
        InitState(ls);
        ls.first_ = content.c_str();
        ls.end_ = ls.first_ + content.length();
        ls.current_ = ls.first_;
        int ret = 0;

        ret = FNLog::Lex(ls);
        FNLOG_ASSERT(ret == FNLog::PEC_NONE, "");
        FNLOG_ASSERT(std::string(ls.line_.key_begin_, ls.line_.key_end_ - ls.line_.key_begin_) == "file", "");
        FNLOG_ASSERT(std::string(ls.line_.val_begin_, ls.line_.val_end_ - ls.line_.val_begin_) == "", "");
    }

    {
        std::string content = " -file  :";
        InitState(ls);
        ls.first_ = content.c_str();
        ls.end_ = ls.first_ + content.length();
        ls.current_ = ls.first_;
        int ret = 0;

        ret = FNLog::Lex(ls);
        FNLOG_ASSERT(ret == FNLog::PEC_NONE, "");
        FNLOG_ASSERT(std::string(ls.line_.key_begin_, ls.line_.key_end_ - ls.line_.key_begin_) == "file", "");
        FNLOG_ASSERT(std::string(ls.line_.val_begin_, ls.line_.val_end_ - ls.line_.val_begin_) == "", "");
    }

    {
        std::string content = " -file  :#fdsafdasf";
        InitState(ls);
        ls.first_ = content.c_str();
        ls.end_ = ls.first_ + content.length();
        ls.current_ = ls.first_;
        int ret = 0;

        ret = FNLog::Lex(ls);
        FNLOG_ASSERT(ret == FNLog::PEC_NONE, "");
        FNLOG_ASSERT(std::string(ls.line_.key_begin_, ls.line_.key_end_ - ls.line_.key_begin_) == "file", "");
        FNLOG_ASSERT(std::string(ls.line_.val_begin_, ls.line_.val_end_ - ls.line_.val_begin_) == "", "");
    }

    {
        std::string content = "- file dfafa : \" path  \"";
        InitState(ls);
        ls.first_ = content.c_str();
        ls.end_ = ls.first_ + content.length();
        ls.current_ = ls.first_;
        int ret = 0;

        ret = FNLog::Lex(ls);
        FNLOG_ASSERT(ret == FNLog::PEC_NONSUPPORT_SYNTAX, "");
    }

    {
        std::string content = " -file  :,,,,,,";
        InitState(ls);
        ls.first_ = content.c_str();
        ls.end_ = ls.first_ + content.length();
        ls.current_ = ls.first_;
        int ret = 0;

        ret = FNLog::Lex(ls);
        FNLOG_ASSERT(ret == FNLog::PEC_NONE, "");
        FNLOG_ASSERT(std::string(ls.line_.key_begin_, ls.line_.key_end_ - ls.line_.key_begin_) == "file", "");
        FNLOG_ASSERT(std::string(ls.line_.val_begin_, ls.line_.val_end_ - ls.line_.val_begin_) == ",,,,,,", "");
    }


    {
        std::string content = " -file  :::::::";
        InitState(ls);
        ls.first_ = content.c_str();
        ls.end_ = ls.first_ + content.length();
        ls.current_ = ls.first_;
        int ret = 0;

        ret = FNLog::Lex(ls);
        FNLOG_ASSERT(ret == FNLog::PEC_NONE, "");
        FNLOG_ASSERT(std::string(ls.line_.key_begin_, ls.line_.key_end_ - ls.line_.key_begin_) == "file", "");
        FNLOG_ASSERT(std::string(ls.line_.val_begin_, ls.line_.val_end_ - ls.line_.val_begin_) == "::::::", "");
    }

    {
        //not support stack block  
        std::string content = " -file  :{[32423]}";
        InitState(ls);
        ls.first_ = content.c_str();
        ls.end_ = ls.first_ + content.length();
        ls.current_ = ls.first_;
        int ret = 0;

        ret = FNLog::Lex(ls);
        FNLOG_ASSERT(ret == FNLog::PEC_NONE, "");
        FNLOG_ASSERT(std::string(ls.line_.key_begin_, ls.line_.key_end_ - ls.line_.key_begin_) == "file", "");
        FNLOG_ASSERT(std::string(ls.line_.val_begin_, ls.line_.val_end_ - ls.line_.val_begin_) == "[32423]", "");
    }



    {
        //indent 
        std::string content = " -file  :";
        InitState(ls);
        ls.first_ = content.c_str();
        ls.end_ = ls.first_ + content.length();
        ls.current_ = ls.first_;
        int ret = 0;

        ret = FNLog::Lex(ls);
        FNLOG_ASSERT(ret == FNLog::PEC_NONE, "");
        FNLOG_ASSERT(ls.line_.blank_ == 1, "");
    }

    {
        //indent 
        std::string content = " file  :";
        InitState(ls);
        ls.first_ = content.c_str();
        ls.end_ = ls.first_ + content.length();
        ls.current_ = ls.first_;
        int ret = 0;

        ret = FNLog::Lex(ls);
        FNLOG_ASSERT(ret == FNLog::PEC_NONE, "");
        FNLOG_ASSERT(ls.line_.blank_ == 1, "");
    }

    {
        //indent 
        std::string content = " - file  :";
        InitState(ls);
        ls.first_ = content.c_str();
        ls.end_ = ls.first_ + content.length();
        ls.current_ = ls.first_;
        int ret = 0;

        ret = FNLog::Lex(ls);
        FNLOG_ASSERT(ret == FNLog::PEC_NONE, "");
        FNLOG_ASSERT(ls.line_.blank_ == 1, "");
    }


    {
        //indent 
        std::string content = " - file  : 1, 23,  2";
        InitState(ls);
        ls.first_ = content.c_str();
        ls.end_ = ls.first_ + content.length();
        ls.current_ = ls.first_;
        int ret = 0;

        ret = FNLog::Lex(ls);
        FNLOG_ASSERT(ret == FNLog::PEC_NONE, "");
        FNLOG_ASSERT(std::string(ls.line_.key_begin_, ls.line_.key_end_ - ls.line_.key_begin_) == "file", "");
        FNLOG_ASSERT(std::string(ls.line_.val_begin_, ls.line_.val_end_ - ls.line_.val_begin_) == "1, 23,  2", "");
    }


    //---------- ill syntax  ------





    LogAlarm() << "all finish";

    return 0;
}
