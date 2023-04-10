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
FNLog::LogStream& operator << (FNLog::LogStream& ls, const Base& base);

//impl
FNLog::LogStream& operator << (FNLog::LogStream& ls, const Base& base)
{
    return ls << base.name();
}

int main(int argc, char* argv[])
{
    int ret = FNLog::FastStartDebugLogger();
    if (ret != 0)
    {
        return ret;
    }

    Base b;
    Derived d;

    LogInfo() << "Base name:" << b << ", Derived name:" << d;

    LogInfo() << "Base name:" << Base() << ", Derived name:" << Derived();


    LogAlarm() << "finish";
    return 0;
}

