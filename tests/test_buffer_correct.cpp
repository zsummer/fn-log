#include "fn_log.h"

using namespace FNLog;

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

int main(int argc, char* argv[])
{
    int ret = FNLog::FastStartDefaultLogger();
    if (ret != 0)
    {
        return ret;
    }

    char buffer[100];

    memset(buffer, 0, sizeof(buffer));
    FNLOG_ASSERT((write_dec_unsafe<2>(buffer, 2ULL) == 2), "write integer wide1");
    FNLOG_ASSERT(buffer[0] == '0', nullptr);
    FNLOG_ASSERT(buffer[1] == '2', nullptr);

    memset(buffer, 0, sizeof(buffer));
    FNLOG_ASSERT((write_dec_unsafe<3>(buffer, -2LL) == 3), "write integer wide2");
    FNLOG_ASSERT(buffer[0] == '-', nullptr);
    FNLOG_ASSERT(buffer[1] == '0', nullptr);
    FNLOG_ASSERT(buffer[2] == '2', nullptr);

    memset(buffer, 0, sizeof(buffer));
    FNLOG_ASSERT((write_hex_unsafe<3>(buffer, (unsigned long long)UINT64_MAX) == 16), "write integer hex UINT64 MAX");
    FNLOG_ASSERT(buffer[0] == 'F', nullptr);
    FNLOG_ASSERT(buffer[7] == 'F', nullptr);
    FNLOG_ASSERT(buffer[16] == '\0', nullptr);

    memset(buffer, 0, sizeof(buffer));
    FNLOG_ASSERT((write_hex_unsafe<3>(buffer, (unsigned long long)UINT64_MAX/2) == 16), "write integer hex UINT64 HALF");
    FNLOG_ASSERT(buffer[0] == '7', nullptr);
    FNLOG_ASSERT(buffer[7] == 'F', nullptr);
    FNLOG_ASSERT(buffer[16] == '\0', nullptr);

    memset(buffer, 0, sizeof(buffer));
    FNLOG_ASSERT((write_hex_unsafe<3>(buffer, ~((unsigned long long)0xf0000000 << 32)) == 15), "write integer hex 63 ");
    FNLOG_ASSERT(buffer[0] == 'F', nullptr);
    FNLOG_ASSERT(buffer[7] == 'F', nullptr);
    FNLOG_ASSERT(buffer[15] == '\0', nullptr);

    memset(buffer, 0, sizeof(buffer));
    FNLOG_ASSERT((write_hex_unsafe<3>(buffer, ~((unsigned long long)0xe0000000 << 32)) == 16), "write integer hex 63 ");
    FNLOG_ASSERT(buffer[0] == '1', nullptr);
    FNLOG_ASSERT(buffer[7] == 'F', nullptr);
    FNLOG_ASSERT(buffer[16] == '\0', nullptr);

    memset(buffer, 0, sizeof(buffer));
    //18446744073709551615
    FNLOG_ASSERT((write_dec_unsafe<0>(buffer, (unsigned long long)UINT64_MAX) == 20), "write integer dec UINT64 MAX");
    FNLOG_ASSERT(buffer[0] == '1', nullptr);
    FNLOG_ASSERT(buffer[1] == '8', nullptr);
    FNLOG_ASSERT(buffer[19] == '5', nullptr);

    memset(buffer, 0, sizeof(buffer));
    //-9223372036854775808
    FNLOG_ASSERT((write_dec_unsafe<0>(buffer, (long long)INT64_MIN) == 20), "write integer dec INT64_MIN MAX");
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
    FNLOG_ASSERT((write_float_unsafe(buffer, -3.14268f) == 7), "write float -pi");
    FNLOG_ASSERT((write_float_unsafe(buffer, -3.14268E-15f) > 3), "write float -pi");
    FNLOG_ASSERT(std::regex_match(buffer, std::regex("-3\\.14.*15")), "write float -pi");
    FNLOG_ASSERT((write_float_unsafe(buffer, 3.14268E-15f) > 3), "write float pi");
    FNLOG_ASSERT(std::regex_match(buffer, std::regex("3\\.14.*15")), "write float pi");
    FNLOG_ASSERT((write_float_unsafe(buffer, 3.14268E15f) > 3), "write float pi");
    FNLOG_ASSERT(std::regex_match(buffer, std::regex("3\\.14.*15")), "write float pi");

    memset(buffer, 0, sizeof(buffer));
    FNLOG_ASSERT((write_double_unsafe(buffer, 3.14f) == 4), "write double pi");
    FNLOG_ASSERT(buffer[0] == '3', nullptr);
    FNLOG_ASSERT(buffer[1] == '.', nullptr);
    FNLOG_ASSERT(buffer[2] == '1', nullptr);

    FNLOG_ASSERT((write_double_unsafe(buffer, 3.1426) == 6), "write double pi");
    FNLOG_ASSERT((write_double_unsafe(buffer, 3.141) == 5), "write double pi");
    FNLOG_ASSERT((write_double_unsafe(buffer, 3.14268) == 6), "write double pi");
    FNLOG_ASSERT((write_double_unsafe(buffer, -3.14268) == 7), "write double -pi");
    FNLOG_ASSERT((write_double_unsafe(buffer, -3.14268E-15) > 3), "write double -pi");
    FNLOG_ASSERT(std::regex_match(buffer, std::regex("-3\\.14.*15")), "write double -pi");
    FNLOG_ASSERT((write_double_unsafe(buffer, 3.14268E-15) > 3), "write double pi");
    FNLOG_ASSERT(std::regex_match(buffer, std::regex("3\\.14.*15")), "write double pi");
    FNLOG_ASSERT((write_double_unsafe(buffer, 3.14268E15) > 3), "write double pi");
    FNLOG_ASSERT(!std::regex_match(buffer, std::regex("3\\.14.*15")), "write double pi");
    FNLOG_ASSERT((write_double_unsafe(buffer, 3.14268E150) > 3), "write double pi");
    FNLOG_ASSERT(std::regex_match(buffer, std::regex("3\\.14.*150")), "write double pi");

    FNLOG_ASSERT((write_pointer_unsafe(buffer, nullptr) == 4), "write nullptr");

    FNLOG_ASSERT((write_pointer_unsafe(buffer, (void*)1) == 3), "write 0x1");

    if (true)
    {
        auto log = (LOG_STREAM_DEFAULT_LOGGER(0, FNLog::PRIORITY_INFO, 0, 0));
        log.write_number<2>(1);
        FNLOG_ASSERT((log.log_data_->content_[0] == '0'), "0");
        FNLOG_ASSERT((log.log_data_->content_[1] == '1'), "0");
    }
    
    if (true)
    {
        FNLOG_ASSERT(FNLog::short_path("a/b/c", 5) == 0, "0");
        FNLOG_ASSERT(FNLog::short_path("/a/b/c", 6) == 1, "0");
        FNLOG_ASSERT(FNLog::short_path("a/b/c/d", 7) == 2, "0");

    }

    return 0;
}
