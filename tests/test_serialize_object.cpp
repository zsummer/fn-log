#include "fn_log.h"

class Base
{
public:
    virtual ~Base() {}
    virtual std::string name() const
    {
        return "base";
    }
};

class Derived : public Base
{
public:
    virtual std::string name() const
    {
        return "derived";
    }
};

//declare  

//impl
FNLog::LogStream& operator << (FNLog::LogStream&& ls, const Base& base)
{
    return ls << base.name();
}

FNLog::LogStream& operator << (FNLog::LogStream& ls, const Base& base)
{
    return ls << base.name();
}


int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;
    int ret = FNLog::FastStartDebugLogger();
    if (ret != 0)
    {
        return ret;
    }

    Base b;
    Derived d;

    LogInfo() << "Base name:" << b << ", Derived name:" << d;

    LogInfo() << "Base name:" << Base() << ", Derived name:" << Derived();


    LogInfo() << b << d << b; 
    FNLog::LogStream (std::move(LogInfo())) << b << d << b;

    FNLog::LogStream(std::move(LogInfo() << "d")) << b << d << b;
    FNLog::LogStream(std::move(LogInfo() << b)) << b << d << b;



    LogAlarm() << "finish";
    return 0;
}

