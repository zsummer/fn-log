#include "fn_log.h"

using namespace FNLog;

#define FNLOG_ASSERT(expr, prefix) \
{ \
    if (expr) \
    { \
        LOGI() << "test " << prefix << " success."; \
    } \
    else \
    {   \
        LOGE() << "test " << prefix << " failed."; \
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

    char buffer[100];

    memset(buffer, 0, sizeof(buffer));
    FNLOG_ASSERT((write_integer_unsafe<10, 2>(buffer, 2ULL) == 2), "write integer wide1");
    FNLOG_ASSERT(buffer[0] == '0', nullptr);
    FNLOG_ASSERT(buffer[1] == '2', nullptr);

    memset(buffer, 0, sizeof(buffer));
    FNLOG_ASSERT((write_integer_unsafe<10, 3>(buffer, -2LL) == 3), "write integer wide2");
    FNLOG_ASSERT(buffer[0] == '-', nullptr);
    FNLOG_ASSERT(buffer[1] == '0', nullptr);
    FNLOG_ASSERT(buffer[2] == '2', nullptr);

    memset(buffer, 0, sizeof(buffer));
    FNLOG_ASSERT((write_integer_unsafe<16, 3>(buffer, (unsigned long long)UINT64_MAX) == 16), "write integer hex UINT64 MAX");
    FNLOG_ASSERT(buffer[0] == 'F', nullptr);
    FNLOG_ASSERT(buffer[7] == 'F', nullptr);
    FNLOG_ASSERT(buffer[16] == '\0', nullptr);

    memset(buffer, 0, sizeof(buffer));
    //18446744073709551615
    FNLOG_ASSERT((write_integer_unsafe<10, 0>(buffer, (unsigned long long)UINT64_MAX) == 20), "write integer dec UINT64 MAX");
    FNLOG_ASSERT(buffer[0] == '1', nullptr);
    FNLOG_ASSERT(buffer[1] == '8', nullptr);
    FNLOG_ASSERT(buffer[19] == '5', nullptr);

    memset(buffer, 0, sizeof(buffer));
    //-9223372036854775808
    FNLOG_ASSERT((write_integer_unsafe<10, 0>(buffer, (long long)INT64_MIN) == 20), "write integer dec INT64_MIN MAX");
    FNLOG_ASSERT(buffer[0] == '-', nullptr);
    FNLOG_ASSERT(buffer[1] == '9', nullptr);
    FNLOG_ASSERT(buffer[19] == '8', nullptr);


    memset(buffer, 0, sizeof(buffer));
    FNLOG_ASSERT((write_float_unsafe(buffer, 3.14f) == 4), "write float pi");
    FNLOG_ASSERT(buffer[0] == '3', nullptr);
    FNLOG_ASSERT(buffer[1] == '.', nullptr);
    FNLOG_ASSERT(buffer[2] == '1', nullptr);

    FNLOG_ASSERT((write_float_unsafe(buffer, 3.1426f) == 6), "write float pi");
    FNLOG_ASSERT((write_float_unsafe(buffer, 3.141f) == 5), "write float pi");
    FNLOG_ASSERT((write_float_unsafe(buffer, 3.14268f) == 6), "write float pi");

    memset(buffer, 0, sizeof(buffer));
    FNLOG_ASSERT((write_double_unsafe(buffer, 3.14f) == 4), "write double pi");
    FNLOG_ASSERT(buffer[0] == '3', nullptr);
    FNLOG_ASSERT(buffer[1] == '.', nullptr);
    FNLOG_ASSERT(buffer[2] == '1', nullptr);

    FNLOG_ASSERT((write_double_unsafe(buffer, 3.1426f) == 6), "write double pi");
    FNLOG_ASSERT((write_double_unsafe(buffer, 3.141f) == 5), "write double pi");
    FNLOG_ASSERT((write_double_unsafe(buffer, 3.14268f) == 6), "write double pi");
    
    FNLOG_ASSERT((write_pointer_unsafe(buffer, nullptr) == 4), "write nullptr");

    FNLOG_ASSERT((write_pointer_unsafe(buffer, (void*)1) == 3), "write 0x1");

    return 0;
}
