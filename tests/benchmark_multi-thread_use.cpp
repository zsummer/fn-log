

#include "fn_log.h"
#include <signal.h>


void thread_proc(int index)
{
    int proc_count = 10;
    int* brk = new int;
    int* mm = (int*)new int[1024 * 1024*10];
    LogInfo() << "thread:<" << index << "> addr:<" << &proc_count << "> begin.  brk:<" << brk << "> mm:<" << mm << ">.";
    while (proc_count-- > 0)
    {
        //LogDebug() << "thread:<" << index << "> addr:<" << &proc_count << "> proc:<" << proc_count << ">.";
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
    LogInfo() << "thread:<" << index << "> addr:<" << &proc_count << "> end.brk:<" << brk << "> mm:<" << mm << ">.";
    delete brk;
    delete[]mm;
}

const int WRITE_THREAD_COUNT = 5;
std::thread g_multi_proc[WRITE_THREAD_COUNT];

void Stop(int signo)
{
    printf("%s", "oops! stop!!!\n");
    _exit(0);
}


int main(int argc, char* argv[])
{
    signal(SIGINT, Stop);
    int ret = FNLog::FastStartDebugLogger();
    if (ret != 0)
    {
        return ret;
    }
    int proc_count = 10;
    int* brk = new int;
    int* mm = (int*)new int[1024 * 1024];
    void* patoi = (void*)&atoi;
    LogInfo() << "       main       addr:<" << &proc_count << "> begin.  brk:<" << brk << "> mm:<" << mm << ">  atoi:<" << patoi << ">.";

    for (int i = 0; i < WRITE_THREAD_COUNT; i++)
    {
        g_multi_proc[i] = std::thread(thread_proc, i);
    }

    for (int i = 0; i < WRITE_THREAD_COUNT; i++)
    {
        if (g_multi_proc[i].joinable())
        {
            g_multi_proc[i].join();
        }
    }



    LogInfo() << "       main       addr:<" << &proc_count << "> end.  brk:<" << brk << "> mm:<" << mm << ">.";
    delete brk;
    delete[]mm;
    return 0;
}

