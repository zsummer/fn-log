
/*
* Copyright (C) 2019 YaweiZhang <yawei.zhang@foxmail.com>.
* All rights reserved
* This file is part of the fn-log, used MIT License.
*/


#pragma once
#ifndef _FN_LOG_PARSE_H_
#define _FN_LOG_PARSE_H_

#include "fn_data.h"


namespace FNLog
{
    enum ParseErrorCode
    {
        PEC_NONE = E_SUCCESS,
        PEC_ERROR = E_BASE_ERRNO_MAX,
        PEC_NONSUPPORT_SYNTAX,
        PEC_ILLEGAL_KEY,
        PEC_ILLEGAL_VAR_NAME,
        PEC_ILLEGAL_VAR_VALUE,
        PEC_NOT_CLOSURE,
        PEC_ILLEGAL_ADDR_IP,
        PEC_ILLEGAL_ADDR_PORT,
        PEC_DEFINED_TARGET_TOO_LONG,

        PEC_UNDEFINED_DEVICE_KEY,
        PEC_UNDEFINED_DEVICE_TYPE,
        PEC_UNDEFINED_CHANNEL_KEY,
        PEC_UNDEFINED_GLOBAL_KEY,

    };


    inline std::string DebugErrno(int error_code)
    {
        switch (error_code)
        {
        case E_SUCCESS:
            return "success";
        case E_LOGGER_IN_USE:
            return "logger alread in use";
        case E_LOGGER_NOT_INIT:
            return "logger not init";
        case E_LOGGER_NOT_RUNNING:
            return "logger not running";
        case E_INVALID_CONFIG_PATH:
            return "invalid config path";
        case E_INVALID_CHANNEL_SIZE:
            return "invalid channel size";
        case E_INVALID_CHANNEL_SYNC:
            return "invalid channel sync";
        case E_INVALID_CHANNEL_STATE:
            return "invalid channel state";
        case E_CHANNEL_THREAD_FAILED:
            return "create thread faield";
        case E_CHANNEL_NOT_SEQUENCE:
            return "channel index need sequence.";

        case E_INVALID_DEVICE_SIZE:
            return "invalid device size";
        case E_DEVICE_NOT_SEQUENCE:
            return "device index need sequence.";

        case E_SHMGET_PROBE_ERROR:
        case E_SHMGET_CREATE_ERROR:
        case E_SHMAT_ERROR:
        case E_SHM_VERSION_WRONG:
        case E_VERSION_MISMATCH:
            return "shm error";
        
        case E_DISABLE_HOTUPDATE:
        case E_NO_CONFIG_PATH:
        case E_CONFIG_NO_CHANGE:
        case E_OUT_RINGBUFFER:
            return "some warn";

        case PEC_ERROR:
            return "syntax error";
        case PEC_NONSUPPORT_SYNTAX:
            return "unsupport syntax";
        case PEC_ILLEGAL_KEY:
            return "unknown key";

        case PEC_ILLEGAL_VAR_NAME:
            return "name or key invalid";
        case PEC_NOT_CLOSURE:
            return "not closure line ";
        case PEC_ILLEGAL_ADDR_IP:
        case PEC_ILLEGAL_ADDR_PORT:
            return "udp addr error";
        case PEC_DEFINED_TARGET_TOO_LONG:
            return "var/macro name len must longger than/equal new name.";

        case PEC_UNDEFINED_DEVICE_KEY:
        case PEC_UNDEFINED_DEVICE_TYPE:
        case PEC_UNDEFINED_CHANNEL_KEY:
        case PEC_UNDEFINED_GLOBAL_KEY:
            return "undefined type";





        default:
            break;
        }
        return "unknown error.";
    }


    enum BlockType
    {
        BLOCK_NONE,
        BLOCK_KEY,
        BLOCK_PRE_SEP,
        BLOCK_PRE_VAL,
        BLOCK_VAL,
        BLOCK_CLEAN,
    };

    enum ReseveKey
    {
        RK_NULL,
        RK_SHM_KEY,
        RK_CHANNEL,
        RK_DEFINE, //the symbol name len must equal or great than new name;   like tag0 10, tag1 100;  will error by "tag0 100000" 
        RK_VARIABLE,
        RK_DEVICE,
        RK_SYNC,
        RK_DISABLE,
        RK_HOT_UPDATE,
        RK_LOGGER_NAME,
        PK_LOGGER_DESC,
        RK_PRIORITY,
        RK_CATEGORY,
        RK_CATEGORY_EXTEND,
        RK_CATEGORY_WLIST, //bitset list 
        RK_CATEGORY_BLIST,
        RK_CATEGORY_WMASK, 
        RK_CATEGORY_BMASK, 
        RK_IDENTIFY,
        RK_IDENTIFY_EXTEND,
        RK_IDENTIFY_WLIST, //bitset list 
        RK_IDENTIFY_BLIST,
        RK_IDENTIFY_WMASK,
        RK_IDENTIFY_BMASK, 
        RK_IN_TYPE,
        RK_OUT_TYPE,
        RK_FILE,
        RK_PATH,
        RK_LIMIT_SIZE,
        RK_ROLLBACK,
        RK_ROLLDAILY,
        RK_ROLLHOURLY,
        RK_FILE_STUFF_UP,
        RK_UDP_ADDR,
    };

#if __GNUG__ && __GNUC__ >= 6
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wclass-memaccess"
#endif

    struct Line
    {
        int blank_;
        int chars_;
        int block_type_;
        char block_stack_;
        int key_;
        const char* key_begin_;
        const char* key_end_;
        const char* val_begin_;
        const char* val_end_;
    };

    struct LexState
    {
        int line_no_;
        const char* first_;
        const char* current_;
        const char* end_;
        Line line_;
        SHMLogger::Channels channels_;
        int channel_size_;
        long long shm_key_;
        bool hot_update_;
        char desc_[Logger::MAX_LOGGER_DESC_LEN];
        int desc_len_;
        char name_[Logger::MAX_LOGGER_NAME_LEN];
        int name_len_;
    };

    inline bool IsBlank(char ch)
    {
        //not std::isblank
        switch (ch)
        {
        case ' ':
        case '\t': 
        case '\v':
        case '\f':
            return true;
        default:
            break;
        }
        return false;
    }
    inline bool IsSoftLineBound(char ch)
    {
        //not std::isblank
        switch (ch)
        {
        case '\0':
        case '\r':
        case '\n':
        case '#':
            return true;
        default:
            break;
        }
        return false;
    }
    inline bool IsLineBound(char ch)
    {
        //not std::isblank
        switch (ch)
        {
        case '\0':
        case '\r':
        case '\n':
            return true;
        default:
            break;
        }
        return false;
    }

    inline bool IsValidName(char ch)
    {
        if (ch >= 'a' && ch <= 'z')
        {
            return true;
        }
        if (ch >= 'A' && ch <= 'Z')
        {
            return true;
        }
        if (ch >= '0' && ch <= '9')
        {
            return true;
        }
        if (ch == '_')
        {
            return true;
        }
        return false;
    }

    inline bool IsValidNameFirst(char ch)
    {
        if (ch >= 'a' && ch <= 'z')
        {
            return true;
        }
        if (ch >= 'A' && ch <= 'Z')
        {
            return true;
        }
        if (ch == '_')
        {
            return true;
        }
        return false;
    }

    inline ReseveKey ParseReserve(const char* begin, const char* end)
    {
        if (end - begin < 2)
        {
            return RK_NULL;
        }
        switch (*begin)
        {
        case 'c':
            if (*(begin + 1) == 'h')
            {
                return RK_CHANNEL;
            }
            else if (*(begin + 1) == 'a')
            {
                if (end - begin > (int)sizeof("category_ex") - 1)
                {
                    if (*(begin + 9) == 'e')  //category_extend
                    {
                        return RK_CATEGORY_EXTEND;
                    }
                    else if (*(begin + 9) == 'w')
                    {
                        if (*(begin + 10) == 'l')  //category_wlist  
                        {
                            return RK_CATEGORY_WLIST;
                        }
                        else if (*(begin + 10) == 'm')
                        {
                            return RK_CATEGORY_WMASK;
                        }
                    }
                    else if (*(begin + 9) == 'b')
                    {
                        if (*(begin + 10) == 'l')  //category_blist; black list   
                        {
                            return RK_CATEGORY_BLIST;
                        }
                        else if (*(begin + 10) == 'm')
                        {
                            return RK_CATEGORY_BMASK;
                        }
                    }
                }
                else
                {
                    return RK_CATEGORY;
                }
            }
            break;
        case 'd':
            if (end - begin < 3)
            {
                return RK_NULL;
            }
            if (*(begin + 2) == 'f')
            {
                return RK_DEFINE;
            }
            else if (*(begin+1) == 'e')
            {
                return RK_DEVICE;
            }
            else if (*(begin + 1) == 'i')
            {
                return RK_DISABLE;
            }
            break;
        case  'f':
            return RK_FILE;
        case 'h':
            return RK_HOT_UPDATE;
        case 'i':
            if (*(begin+1) == 'n')
            {
                return RK_IN_TYPE;
            }
            else if (end - begin > (int)sizeof("identify_ex") - 1)
            {
                if (*(begin + 9) == 'e')
                {
                    return RK_IDENTIFY_EXTEND;
                }
                else if (*(begin + 9) == 'w')
                {
                    if (*(begin + 10) == 'l')  //identify_wlist  
                    {
                        return RK_IDENTIFY_WLIST;
                    }
                    else if (*(begin + 10) == 'm')
                    {
                        return RK_IDENTIFY_WMASK;
                    }
                }
                else if (*(begin + 9) == 'b')
                {
                    if (*(begin + 10) == 'l')  //identify_blist;  black list  
                    {
                        return RK_IDENTIFY_BLIST;
                    }
                    else if (*(begin + 10) == 'm')
                    {
                        return RK_IDENTIFY_BMASK;
                    }
                }
            }
            else
            {
                return RK_IDENTIFY;
            }
            break;
        case 'l':
            if (*(begin + 1) == 'i')
            {
                return RK_LIMIT_SIZE;
            }
            else if (end - begin > 8)
            {
                if (*(begin + 7) == 'n')
                {
                    return RK_LOGGER_NAME;
                }
                if (*(begin + 7) == 'd')
                {
                    return PK_LOGGER_DESC;
                }
            }
            break;
        case 'p':
            if (*(begin + 1) == 'r')
            {
                return RK_PRIORITY;
            }
            else if (*(begin + 1) == 'a')
            {
                return RK_PATH;
            }
            break;
        case 'r':
            if (end - begin > (int)sizeof("rollb"))
            {
                if (*(begin + 4) == 'b')
                {
                    return RK_ROLLBACK;
                }
                else if (*(begin + 4) == 'd')
                {
                    return RK_ROLLDAILY;
                }
                else if (*(begin + 4) == 'h')
                {
                    return RK_ROLLHOURLY;
                }
            }
            break;
        case 'o':
            return RK_OUT_TYPE;
        case 's':
            if (*(begin+1) == 'y')
            {
                return RK_SYNC;
            }
            else if (*(begin + 1) == 't')
            {
                return RK_FILE_STUFF_UP;
            }
            return RK_SHM_KEY;
        case 'u':
            return RK_UDP_ADDR;
        case 'v':
            return RK_VARIABLE;
        default:
            break;
        }
        return RK_NULL;
    }

    inline LogPriority ParsePriority(const char* begin, const char* end)
    {
        if (end <= begin)
        {
            return PRIORITY_TRACE;
        }
        switch (*begin)
        {
        case 't':case 'T':
        case 'n':case 'N':
            return PRIORITY_TRACE;
        case 'd':case 'D':
            return PRIORITY_DEBUG;
        case 'i':case 'I':
            return PRIORITY_INFO;
        case 'w':case 'W':
            return PRIORITY_WARN;
        case 'e':case 'E':
            return PRIORITY_ERROR;
        case 'a':case 'A':
            return PRIORITY_ALARM;
        case 'f':case 'F':
            return PRIORITY_FATAL;
        }
        return PRIORITY_TRACE;
    }

    inline bool ParseBool(const char* begin, const char* end)
    {
        if (end <= begin)
        {
            return false;
        }
        if (*begin == '0' || *begin == 'f' || *begin == 'F')
        {
            return false;
        }
        return true;
    }

    inline long long ParseNumber(const char* begin, const char* end)
    {
        if (end <= begin)
        {
            return 0;
        }

        if (end - begin > 40 )
        {
            return 0;
        }

        char buff[50];
        memcpy(buff, begin, end - begin);
        buff[end - begin] = '\0';
        return strtoll(buff, NULL, 0);
    }

    inline bool ParseString(const char* begin, const char* end, char * buffer, int buffer_len, int& write_len)
    {
        write_len = 0;
        if (end <= begin)
        {
            return false;
        }
        write_len = buffer_len - 1;
        if (end - begin < write_len)
        {
            write_len = (int)(end - begin);
        }
        memcpy(buffer, begin, write_len);
        buffer[write_len] = '\0';
        return true;
    }
    inline ChannelType ParseChannelType(const char* begin, const char* end)
    {
        if (end <= begin || *begin != 's')
        {
            return CHANNEL_ASYNC;
        }
        return CHANNEL_SYNC;
    }

    inline DeviceInType ParseInType(const char* begin, const char* end)
    {
        if (end <= begin)
        {
            return DEVICE_IN_NULL;
        }
        switch (*begin)
        {
        case 'u': case 'U':
            return DEVICE_IN_UDP;
        }
        return DEVICE_IN_NULL;
    }

    inline DeviceOutType ParseOutType(const char* begin, const char* end)
    {
        if (end <= begin)
        {
            return DEVICE_OUT_NULL;
        }
        switch (*begin)
        {
        case 'e': case 'E':
            return DEVICE_OUT_EMPTY;
        case 'f': case 'F':
            return DEVICE_OUT_FILE;
        case 'n': case 'N':
            return DEVICE_OUT_NULL;
        case 'u': case 'U':
            return DEVICE_OUT_UDP;
        case 's':case 'S':
            return DEVICE_OUT_SCREEN;
        case 'v':case 'V':
            return DEVICE_OUT_VIRTUAL;
        }
        return DEVICE_OUT_NULL;
    }

    inline std::pair<long long, const char*> ParseAddresIP(const char* begin, const char* end, bool parse_dn)
    {
        if (end <= begin)
        {
            return std::make_pair(0, end);
        }
        //only support ipv4 
        const char* ip_begin = begin;
        while (IsBlank(*ip_begin) && ip_begin != end)
        {
            ip_begin++;
        }
        const char* ip_end = ip_begin;
        bool is_dn = false;
        while (!(IsBlank(*ip_end) || *ip_end == ':') && ip_end != end)
        {
            if (!(*ip_end >= '0' && *ip_end <= '9') && *ip_end != '.')
            {
                is_dn = true;
            }
            ip_end++;
        }

        if (ip_end - ip_begin <= 0)
        {
            return std::make_pair(0, end);
        }
        std::string dn(ip_begin, ip_end - ip_begin);

        if (is_dn) //syn to get    
        {
            if (false)
            {
                struct addrinfo* res = nullptr;
                struct addrinfo hints;
                memset(&hints, 0, sizeof(hints));
                hints.ai_family = AF_UNSPEC;
                hints.ai_socktype = SOCK_STREAM;
                hints.ai_flags = AI_PASSIVE;
                
                if (getaddrinfo(dn.c_str(), "", &hints, &res) == 0)
                {
                    char buf[100] = { 0 };
                    if (res->ai_family == AF_INET)
                    {
                        inet_ntop(res->ai_family, &(((sockaddr_in*)res->ai_addr)->sin_addr), buf, 100);
                    }
                    else if (res->ai_family == AF_INET6)
                    {
                        inet_ntop(res->ai_family, &(((sockaddr_in6*)res->ai_addr)->sin6_addr), buf, 100);
                    }
                    return std::make_pair((long long)inet_addr(buf), ip_end);
                }
            }
            if (parse_dn)
            {
                struct hostent* rhost = gethostbyname(dn.c_str());
                if (rhost == nullptr)
                {
                    return std::make_pair(0, end);
                }
                if (rhost->h_addrtype == AF_INET)
                {
                    int i = 0;
                    struct in_addr addr;
                    while (rhost->h_addr_list[i] != 0) 
                    {
                        addr.s_addr = *(u_long*)rhost->h_addr_list[i++];
                        return std::make_pair((long long)inet_addr(inet_ntoa(addr)), ip_end);
                        //printf("%s", inet_ntoa(addr));
                    }
                }
                
            }
            return std::make_pair(0, end);
        }


        return std::make_pair((long long)inet_addr(dn.c_str()), ip_end);
    }


    inline void ParseAddres(const char* begin, const char* end, bool parse_dn, long long & ip, long long& port)
    {
        ip = 0;
        port = 0;
        if (end <= begin)
        {
            return;
        }
        auto result_ip = ParseAddresIP(begin, end, parse_dn);
        const char* port_begin = result_ip.second;
        while (port_begin != end && (*port_begin < '1' || *port_begin > '9')  )
        {
            port_begin++;
        }
        if (port_begin >= end)
        {
            return;
        }
        if (end - port_begin >= 40)
        {
            return;
        }
        char buff[50];
        memcpy(buff, port_begin, end - port_begin);
        buff[end - port_begin] = '\0';
        port = htons((unsigned short)atoi(buff));
        ip = result_ip.first;
        return;
    }

    inline unsigned long long ParseBitArray(const char* begin, const char* end)
    {
        unsigned long long bitmap = 0;
        if (end <= begin)
        {
            return bitmap;
        }
        const char* offset = begin;
        while (offset < end)
        {
            if (*offset >= '0' && *offset <= '9')
            {
                int bit_offset = atoi(offset);
                bitmap |= (1ULL << bit_offset);
                while (offset < end && (*offset >= '0' && *offset <= '9'))
                {
                    offset++;
                }
                continue;
            }
            offset++;
        }
        return bitmap;
    }
    inline int PredefinedInplace(std::string& text, std::string::size_type text_offset, std::string key, std::string val, bool pre_bound, bool suffix_bound)
    {
        if (val.length() > key.length())
        {
            return PEC_DEFINED_TARGET_TOO_LONG;
        }

        //fixed len
        while (val.length() < key.length())
        {
            val.push_back(' ');
        }

        //replace 
        while (true)
        {
            text_offset = text.find(key, text_offset);
            if (text_offset == std::string::npos)
            {
                //finish 
                break;
            }
            bool bound_ok = true;
            while (bound_ok && pre_bound)
            {
                if (text_offset == 0)
                {
                    break;
                }
                char ch = text[text_offset - 1];
                if (IsValidName(ch))
                {
                    bound_ok = false;
                    text_offset++;
                    break;
                }
                break;
            }

            while (bound_ok && suffix_bound)
            {
                if (text_offset + val.length() == text.length())
                {
                    break;
                }
                char ch = text[text_offset + val.length()];
                if (IsValidName(ch))
                {
                    bound_ok = false;
                    text_offset++;
                    break;
                }
                break;
            }

            if (bound_ok)
            {
                memcpy(&text[text_offset], val.c_str(), val.length());
            }
        };
        return 0;
    }
    inline int PredefinedMacro(LexState& ls, std::string& text)
    {
        std::string line(ls.line_.val_begin_, ls.line_.val_end_ - ls.line_.val_begin_);
        std::string symbol;
        std::string::size_type sep = line.find(' ');
        if (sep == std::string::npos)
        {
            return PEC_NOT_CLOSURE;
        }
        symbol = line.substr(0, sep);
        if (symbol.empty())
        {
            return PEC_NOT_CLOSURE;
        }

        while (sep < line.length())
        {
            if (IsBlank(line[sep]))
            {
                sep++;
                continue;
            }
            break;
        }
        std::string content = line.substr(sep);

        int ret = PredefinedInplace(text, ls.line_.val_end_ - text.c_str(), symbol, content, true, true);
        if (ret != PEC_NONE)
        {
            return ret;
        }
        return PEC_NONE;
    }

    inline int PredefinedVar(LexState& ls, std::string& text)
    {
        std::string line(ls.line_.val_begin_, ls.line_.val_end_ - ls.line_.val_begin_);
        std::string::size_type offset = 0;
        std::string var; 
        std::string val;
        while (offset < line.length())
        {
            if (line.at(offset) == ' ' || line.at(offset) == '\t')
            {
                offset++;
                continue;
            }

            std::string::size_type dot = line.find(',', offset);
            if (dot == std::string::npos)
            {
                dot = line.length();
            }

            std::string::size_type sep = line.find('=', offset);
            if (sep >= dot)
            {
                offset = dot + 1;
                return PEC_ILLEGAL_VAR_VALUE;
            }
            var = line.substr(offset, sep - offset);
            while (!var.empty() && IsBlank(var.back()))
            {
                var.pop_back();
            }

            if (var.empty())
            {
                //has dot but wrong
                offset = dot + 1;
                //continue;
                return PEC_DEFINED_TARGET_TOO_LONG;
            }

            if (!IsValidNameFirst(var[0]))
            {
                //has dot but wrong
                offset = dot + 1;
                //continue;
                return PEC_ILLEGAL_VAR_NAME;
            }

            sep++;
            while (sep < dot &&  IsBlank(line.at(offset)))
            {
                sep++;
            }
            val = line.substr(sep, dot - sep);
            while (!val.empty() && IsBlank(val.back()))
            {
                val.pop_back();
            }



            std::string::size_type text_offset = ls.line_.val_end_ - text.c_str();

            int ret = PredefinedInplace(text, text_offset, std::string("${") + var + "}", val, false, false);
            if (ret != 0)
            {
                offset = dot + 1;
                return ret;
            }
            ret = PredefinedInplace(text, text_offset, std::string("$") + var, val, false, true);
            if (ret != 0)
            {
                offset = dot + 1;
                return ret;
            }

            offset = dot + 1;
        }
  
        return PEC_NONE;
    }

    inline void InitState(LexState& state)
    {
        //static_assert(std::is_trivial<LexState>::value, "");
        memset(&state, 0, sizeof(state));
    }

    inline int Lex(LexState& ls)
    {
        memset(&ls.line_, 0, sizeof(ls.line_));
        if (ls.current_ >= ls.end_)
        {
            if (ls.line_no_ == 0)
            {
                ls.line_no_ = 1;
            }
            return PEC_NONE;
        }
        while (true)
        {
            char ch = *ls.current_++;
            ls.line_.chars_++;

            bool is_blank = IsBlank(ch);
            bool is_end_char = IsSoftLineBound(ch);

            if (ls.line_.block_type_ == BLOCK_CLEAN && !IsLineBound(ch))
            {
                continue;
            }

            if (is_end_char)
            {
                if (ls.line_.block_type_ == BLOCK_VAL)
                {
                    if (ls.line_.block_stack_ != '\0')
                    {
                        return PEC_NOT_CLOSURE;
                    }

                    ls.line_.block_type_ = BLOCK_CLEAN;
                    ls.line_.val_end_ = ls.current_ - 1;
                }

                //no value 
                if (ls.line_.block_type_ == BLOCK_PRE_VAL)
                {
                    ls.line_.block_type_ = BLOCK_CLEAN;
                    ls.line_.val_begin_ = ls.current_ - 1;
                    ls.line_.val_end_ = ls.current_ - 1;
                }

                if (ls.line_.block_type_ != BLOCK_NONE && ls.line_.block_type_ != BLOCK_CLEAN)
                {
                    return PEC_NOT_CLOSURE;
                }

                while (ls.line_.val_end_ > ls.line_.val_begin_)
                {
                    char suffix = *(ls.line_.val_end_ - 1);
                    if (IsBlank(suffix))
                    {
                        ls.line_.val_end_--;
                        continue;
                    }
                    break;
                }


                if (ch == '\r' || ch == '\n')
                {
                    if ((*ls.current_ == '\r' || *ls.current_ == '\n') && *ls.current_ != ch)
                    {
                        ls.current_++;//jump win rt  
                    }
                    ls.line_no_++;
                    return PEC_NONE;
                }

                if (ch == '\0')
                {
                    ls.current_--; //safe; set current referer to '\0'   
                    ls.line_no_++; //last line compensate that the line no aways ref valid lex line no.  
                    return PEC_NONE;
                }

                if (ch == '#')
                {
                    ls.line_.block_type_ = BLOCK_CLEAN;
                    continue;
                }
            }


            if (ls.line_.block_type_ == BLOCK_NONE)
            {
                if (is_blank)
                {
                    if (ls.line_.blank_ == ls.line_.chars_ - 1)
                    {
                        ls.line_.blank_++;
                    }
                    continue;
                }
                //ignore array char 
                if (ch == '-')
                {
                    continue;
                }

                //first char 
                if (ch < 'a' || ch > 'z')
                {
                    return PEC_ILLEGAL_KEY;
                }
                ls.line_.block_type_ = BLOCK_KEY;
                ls.line_.key_begin_ = ls.current_ - 1;
            }

            //key must in [a-z_0-9]
            if (ls.line_.block_type_ == BLOCK_KEY)
            {
                if ((ch >= 'a' && ch <= 'z') || ch == '_' || (ch >= '0' && ch <= '9'))
                {
                    continue;
                }
                ls.line_.block_type_ = BLOCK_PRE_SEP;
                ls.line_.key_end_ = ls.current_ - 1;
                ls.line_.key_ = ParseReserve(ls.line_.key_begin_, ls.line_.key_end_);
                if (ls.line_.key_ == RK_NULL)
                {
                    return PEC_ILLEGAL_KEY;
                }

            }

            
            if (ls.line_.block_type_ == BLOCK_PRE_SEP)
            {
                if (ch == ':')
                {
                    ls.line_.block_type_ = BLOCK_PRE_VAL;
                    continue;
                }
                //not support yaml '-' array 
                if (ch != ' ' && ch != '\t')
                {
                    return PEC_NONSUPPORT_SYNTAX;
                }
                continue;
            }

            if (ls.line_.block_type_ == BLOCK_PRE_VAL)
            {
                if (IsBlank(ch))
                {
                    continue;
                }
                if (ch == '\"')
                {
                    ls.line_.block_stack_ = '\"';
                    ls.line_.block_type_ = BLOCK_VAL;
                    ls.line_.val_begin_ = ls.current_;
                    continue;
                }
                if (ch == '[')
                {
                    ls.line_.block_stack_ = ']';
                    ls.line_.block_type_ = BLOCK_VAL;
                    ls.line_.val_begin_ = ls.current_;
                    continue;
                }
                if (ch == '{')
                {
                    ls.line_.block_stack_ = '}';
                    ls.line_.block_type_ = BLOCK_VAL;
                    ls.line_.val_begin_ = ls.current_;
                    continue;
                }
                ls.line_.block_stack_ = '\0';
                ls.line_.block_type_ = BLOCK_VAL;
                ls.line_.val_begin_ = ls.current_ - 1;
            }


            if (ls.line_.block_type_ == BLOCK_VAL)
            {
                if (ls.line_.val_begin_ == ls.current_ - 1)
                {
                    //trim blank begin "{[  
                    if (is_blank)
                    {
                        ls.line_.val_begin_ = ls.current_;
                        continue;
                    }
                }

                if (ls.line_.block_stack_ != '\0' && ch == ls.line_.block_stack_)
                {
                    ls.line_.block_type_ = BLOCK_CLEAN;
                    ls.line_.val_end_ = ls.current_ - 1;
                    continue;
                }



                continue;
            }

        }
        return PEC_ERROR;
    }

    inline int ParseDevice(LexState& ls, Device& device, int indent)
    {
        do
        {
            const char* current = ls.current_;
            int ret = Lex(ls);
            if (ret != PEC_NONE)
            {
                ls.current_ = current;
                return ret;
            }

            if (ls.line_.key_end_ - ls.line_.key_begin_ == 0)
            {
                if (ls.current_ >= ls.end_)
                {
                    //eof 
                    return ret;
                }
                //blank line 
                continue;
            }
            
            if (ls.line_.blank_ <= indent)
            {
                ls.current_ = current;
                return 0;
            }

            switch (ls.line_.key_)
            {
            case RK_IN_TYPE:
                device.in_type_ = ParseInType(ls.line_.val_begin_, ls.line_.val_end_);
                break;
            case RK_OUT_TYPE:
                device.out_type_ = ParseOutType(ls.line_.val_begin_, ls.line_.val_end_);
                if (device.out_type_ == DEVICE_OUT_NULL)
                {
                    return PEC_UNDEFINED_DEVICE_TYPE;
                }
                break;
            case RK_DISABLE:
                device.config_fields_[DEVICE_CFG_ABLE] = !ParseBool(ls.line_.val_begin_, ls.line_.val_end_); //"disable"
                break;
            case RK_PRIORITY:
                device.config_fields_[DEVICE_CFG_PRIORITY] = ParsePriority(ls.line_.val_begin_, ls.line_.val_end_);
                break;
            case RK_CATEGORY:
                device.config_fields_[DEVICE_CFG_CATEGORY] = atoll(ls.line_.val_begin_);
                break;
            case RK_CATEGORY_EXTEND:
                device.config_fields_[DEVICE_CFG_CATEGORY_EXTEND] = atoll(ls.line_.val_begin_);
                break;
            case RK_CATEGORY_WLIST:
                device.config_fields_[DEVICE_CFG_CATEGORY_MASK] = ParseBitArray(ls.line_.val_begin_, ls.line_.val_end_);
                break;
            case RK_CATEGORY_BLIST:
                device.config_fields_[DEVICE_CFG_CATEGORY_MASK] = ~ParseBitArray(ls.line_.val_begin_, ls.line_.val_end_);
                break;
            case RK_CATEGORY_WMASK:
                device.config_fields_[DEVICE_CFG_CATEGORY_MASK] = atoll(ls.line_.val_begin_);
                break;
            case RK_CATEGORY_BMASK:
                device.config_fields_[DEVICE_CFG_CATEGORY_MASK] = ~atoll(ls.line_.val_begin_);
                break;
            case RK_IDENTIFY:
                device.config_fields_[DEVICE_CFG_IDENTIFY] = atoll(ls.line_.val_begin_);
                break;
            case RK_IDENTIFY_EXTEND:
                device.config_fields_[DEVICE_CFG_IDENTIFY_EXTEND] = atoll(ls.line_.val_begin_);
                break;
            case RK_IDENTIFY_WLIST:
                device.config_fields_[DEVICE_CFG_IDENTIFY_MASK] = ParseBitArray(ls.line_.val_begin_, ls.line_.val_end_);
                break;
            case RK_IDENTIFY_BLIST:
                device.config_fields_[DEVICE_CFG_IDENTIFY_MASK] = ~ParseBitArray(ls.line_.val_begin_, ls.line_.val_end_);
                break;
            case RK_IDENTIFY_WMASK:
                device.config_fields_[DEVICE_CFG_IDENTIFY_MASK] = atoll(ls.line_.val_begin_);
                break;
            case RK_IDENTIFY_BMASK:
                device.config_fields_[DEVICE_CFG_IDENTIFY_MASK] = ~atoll(ls.line_.val_begin_);
                break;
            case RK_LIMIT_SIZE:
                device.config_fields_[DEVICE_CFG_FILE_LIMIT_SIZE] = atoll(ls.line_.val_begin_) * 1000*1000;
                break;
            case RK_ROLLBACK:
                device.config_fields_[DEVICE_CFG_FILE_ROLLBACK] = atoll(ls.line_.val_begin_);
                break;
            case RK_ROLLDAILY:
                device.config_fields_[DEVICE_CFG_FILE_ROLLDAILY] = ParseBool(ls.line_.val_begin_, ls.line_.val_end_);
                break;
            case RK_ROLLHOURLY:
                device.config_fields_[DEVICE_CFG_FILE_ROLLHOURLY] = ParseBool(ls.line_.val_begin_, ls.line_.val_end_);
                break;
            case RK_FILE_STUFF_UP:
                device.config_fields_[DEVICE_CFG_FILE_STUFF_UP] = ParseBool(ls.line_.val_begin_, ls.line_.val_end_);  
                break;
            case RK_PATH:
                if (ls.line_.val_end_ - ls.line_.val_begin_ < Device::MAX_PATH_LEN - 1
                    && ls.line_.val_end_ - ls.line_.val_begin_ >= 1)
                {
                    memcpy(device.out_path_, ls.line_.val_begin_, ls.line_.val_end_ - ls.line_.val_begin_);
                    device.out_path_[ls.line_.val_end_ - ls.line_.val_begin_] = '\0';
                }
                break;
            case RK_FILE:
                if (ls.line_.val_end_ - ls.line_.val_begin_ < Device::MAX_LOGGER_NAME_LEN - 1
                    && ls.line_.val_end_ - ls.line_.val_begin_ >= 1)
                {
                    memcpy(device.out_file_, ls.line_.val_begin_, ls.line_.val_end_ - ls.line_.val_begin_);
                    device.out_file_[ls.line_.val_end_ - ls.line_.val_begin_] = '\0';
                }
                break;
            case RK_UDP_ADDR:
                if (true) 
                {
                    long long ip = 0;
                    long long port = 0;
                    bool parse_dn = device.config_fields_[DEVICE_CFG_ABLE] != 0;
                    ParseAddres(ls.line_.val_begin_, ls.line_.val_end_, parse_dn, ip, port);
                    device.config_fields_[DEVICE_CFG_UDP_IP] = ip;
                    device.config_fields_[DEVICE_CFG_UDP_PORT] = port;
                }
                if (device.config_fields_[DEVICE_CFG_ABLE])
                {
                    if (device.in_type_ == DEVICE_IN_NULL && device.config_fields_[DEVICE_CFG_UDP_IP] == 0)
                    {
                        return PEC_ILLEGAL_ADDR_IP;
                    }

                    if (device.config_fields_[DEVICE_CFG_UDP_PORT] == 0)
                    {
                        return PEC_ILLEGAL_ADDR_PORT;
                    }
                }


                break;
            default:
                return PEC_UNDEFINED_DEVICE_KEY;
            }
        } while (ls.current_ < ls.end_);
        return 0;
    }
    inline int ParseChannel(LexState& ls, Channel& channel, int indent)
    {
        do
        {
            const char* current = ls.current_;
            int ret = Lex(ls);
            if (ret != PEC_NONE)
            {
                ls.current_ = current;
                return ret;
            }
            if (ls.line_.key_end_ - ls.line_.key_begin_ == 0)
            {
                if (ls.current_ >= ls.end_)
                {
                    return ret;
                }
                continue;
            }

            if (ls.line_.blank_ <= indent)
            {
                ls.current_ = current;
                return 0;
            }

            switch (ls.line_.key_)
            {
            case RK_SYNC:
                channel.channel_type_ = ParseChannelType(ls.line_.val_begin_, ls.line_.val_end_);
                break;
            case RK_PRIORITY:
                channel.config_fields_[CHANNEL_CFG_PRIORITY] = ParsePriority(ls.line_.val_begin_, ls.line_.val_end_);
                break;
            case RK_CATEGORY:
                channel.config_fields_[CHANNEL_CFG_CATEGORY] = atoi(ls.line_.val_begin_);
                break;            
            case RK_CATEGORY_EXTEND:
                channel.config_fields_[CHANNEL_CFG_CATEGORY_EXTEND] = atoi(ls.line_.val_begin_);
                break;
            case RK_CATEGORY_WLIST:
                channel.config_fields_[CHANNEL_CFG_CATEGORY_MASK] = ParseBitArray(ls.line_.val_begin_, ls.line_.val_end_);
                break;
            case RK_CATEGORY_BLIST:
                channel.config_fields_[CHANNEL_CFG_CATEGORY_MASK] = ~ParseBitArray(ls.line_.val_begin_, ls.line_.val_end_);
                break;
            case RK_CATEGORY_WMASK:
                channel.config_fields_[CHANNEL_CFG_CATEGORY_MASK] = atoll(ls.line_.val_begin_);
                break;
            case RK_CATEGORY_BMASK:
                channel.config_fields_[CHANNEL_CFG_CATEGORY_MASK] = ~atoll(ls.line_.val_begin_);
                break;
            case RK_IDENTIFY:
                channel.config_fields_[CHANNEL_CFG_IDENTIFY] = atoi(ls.line_.val_begin_);
                break;
            case RK_IDENTIFY_EXTEND:
                channel.config_fields_[CHANNEL_CFG_IDENTIFY_EXTEND] = atoi(ls.line_.val_begin_);
                break;
            case RK_IDENTIFY_WLIST:
                channel.config_fields_[CHANNEL_CFG_IDENTIFY_MASK] = ParseBitArray(ls.line_.val_begin_, ls.line_.val_end_);
                break;
            case RK_IDENTIFY_BLIST:
                channel.config_fields_[CHANNEL_CFG_IDENTIFY_MASK] = ~ParseBitArray(ls.line_.val_begin_, ls.line_.val_end_);
                break;
            case RK_IDENTIFY_WMASK:
                channel.config_fields_[CHANNEL_CFG_IDENTIFY_MASK] = atoll(ls.line_.val_begin_);
                break;
            case RK_IDENTIFY_BMASK:
                channel.config_fields_[CHANNEL_CFG_IDENTIFY_MASK] = ~atoll(ls.line_.val_begin_);
                break;

            case RK_DEVICE:
                {
                    int device_id = atoi(ls.line_.val_begin_);
                    if (channel.device_size_ >= Channel::MAX_DEVICE_SIZE)
                    {
                        return E_INVALID_DEVICE_SIZE;
                    }
                    if (device_id != channel.device_size_)
                    {
                        return E_DEVICE_NOT_SEQUENCE;
                    }
                    Device& device = channel.devices_[channel.device_size_++];
                    memset(&device, 0, sizeof(device));
                    device.device_id_ = device_id;
                    ret = ParseDevice(ls, device, ls.line_.blank_);
                    if (device.out_type_ == DEVICE_OUT_VIRTUAL)
                    {
                        channel.virtual_device_id_ = device.device_id_;
                    }
                    if (ret != PEC_NONE)
                    {
                        return ret;
                    }
                }
                break;
            default:
                return PEC_UNDEFINED_CHANNEL_KEY;
            }

        } while (ls.current_ < ls.end_);
        return 0;
    }

    inline int ParseLogger(LexState& ls, std::string& text)
    {
        //UTF8 BOM 
        const char* first = &text[0];
        if (text.size() >= 3)
        {
            if ((unsigned char)text[0] == 0xEF && (unsigned char)text[1] == 0xBB && (unsigned char)text[2] == 0xBF)
            {
                first += 3;
            }
        }
        ls.first_ = first;
        ls.end_ = first + text.length();
        memset(&ls.channels_, 0, sizeof(ls.channels_));
        ls.channel_size_ = 0;
        ls.hot_update_ = false;
        ls.current_ = ls.first_;
        ls.line_no_ = 0;
        ls.desc_len_ = 0;
        ls.name_len_ = 0;
        do
        {
            const char* current = ls.current_;
            int ret = Lex(ls);
            if (ret != PEC_NONE)
            {
                ls.current_ = current;
                return ret;
            }
            if (ls.line_.key_end_ - ls.line_.key_begin_ == 0)
            {
                if (ls.current_ >= ls.end_)
                {
                    return ret;
                }
                continue;
            }

            switch (ls.line_.key_)
            {
            case RK_HOT_UPDATE:
                ls.hot_update_ = ParseBool(ls.line_.val_begin_, ls.line_.val_end_);//"disable"
                break;
            case RK_DEFINE:
                //do nothing  
                ret = PredefinedMacro(ls, text);
                if (ret != PEC_NONE)
                {
                    return ret;
                }
                break;
            case RK_VARIABLE:
                //do nothing  
                ret = PredefinedVar(ls, text);
                if (ret != PEC_NONE)
                {
                    return ret;
                }
                break;
            case RK_LOGGER_NAME:
                ParseString(ls.line_.val_begin_, ls.line_.val_end_, ls.name_, Logger::MAX_LOGGER_NAME_LEN, ls.name_len_);
                break;
            case PK_LOGGER_DESC:
                ParseString(ls.line_.val_begin_, ls.line_.val_end_, ls.desc_, Logger::MAX_LOGGER_DESC_LEN, ls.desc_len_);
                break;
            case RK_SHM_KEY:
                ls.shm_key_ = ParseNumber(ls.line_.val_begin_, ls.line_.val_end_);
                break;
            case RK_CHANNEL:
                {
                    int channel_id = atoi(ls.line_.val_begin_);
                    if (ls.channel_size_ >= Logger::MAX_CHANNEL_SIZE)
                    {
                        return E_INVALID_CHANNEL_SIZE;
                    }
                    if (ls.channel_size_ != channel_id)
                    {
                        return E_CHANNEL_NOT_SEQUENCE;
                    }
                    Channel& channel = ls.channels_[ls.channel_size_++];
                    memset(&channel, 0, sizeof(channel));
                    channel.channel_id_ = channel_id;
                    ret = ParseChannel(ls, channel, ls.line_.blank_);
                    if (ret != 0)
                    {
                        return ret;
                    }
                }
                break;
            default:
                return PEC_UNDEFINED_GLOBAL_KEY;
            }
        } while (ls.current_ < ls.end_);
        if (ls.channel_size_ == 0)
        {
            return E_INVALID_CHANNEL_SIZE;
        }
        return PEC_NONE;
    }

#if __GNUG__ && __GNUC__ >= 6
#pragma GCC diagnostic pop
#endif

}


#endif
