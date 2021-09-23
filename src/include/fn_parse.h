/*
 *
 * MIT License
 *
 * Copyright (C) 2019 YaweiZhang <yawei.zhang@foxmail.com>.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * ===============================================================================
 *
 * (end of COPYRIGHT)
 */


 /*
  * AUTHORS:  YaweiZhang <yawei.zhang@foxmail.com>
  * VERSION:  1.0.0
  * PURPOSE:  fn-log is a cpp-based logging utility.
  * CREATION: 2019.4.20
  * RELEASED: 2019.6.27
  * QQGROUP:  524700770
  */


#pragma once
#ifndef _FN_LOG_PARSE_H_
#define _FN_LOG_PARSE_H_

#include "fn_data.h"


namespace FNLog
{
    enum ParseErrorCode
    {
        PEC_NONE, 
        PEC_ERROR,
        PEC_ILLEGAL_CHARACTER,
        PEC_ILLEGAL_KEY,
        PEC_NOT_CLOSURE,
        PEC_ILLEGAL_ADDR_IP,
        PEC_ILLEGAL_ADDR_PORT,

        PEC_UNDEFINED_DEVICE_KEY,
        PEC_UNDEFINED_DEVICE_TYPE,
        PEC_UNDEFINED_CHANNEL_KEY,
        PEC_UNDEFINED_GLOBAL_KEY,

        PEC_DEVICE_NOT_ARRAY,
        PEC_DEVICE_INDEX_OUT_MAX,
        PEC_DEVICE_INDEX_NOT_SEQUENCE,
 

        PEC_CHANNEL_NOT_ARRAY,
        PEC_CHANNEL_INDEX_OUT_MAX,
        PEC_CHANNEL_INDEX_NOT_SEQUENCE,
        PEC_NO_ANY_CHANNEL,
    };

    enum LineType
    {
        LINE_NULL,
        LINE_ARRAY,
        LINE_BLANK,
        LINE_EOF,
    };
    enum BlockType
    {
        BLOCK_BLANK,
        BLOCK_PRE_KEY,
        BLOCK_KEY,
        BLOCK_PRE_SEP,
        BLOCK_PRE_VAL,
        BLOCK_VAL,
        BLOCK_CLEAN,
    };
    enum ReseveKey
    {
        RK_NULL,
        RK_CHANNEL,
        RK_DEVICE,
        RK_SYNC,
        RK_DISABLE,
        RK_HOT_UPDATE,
        RK_LOGGER_NAME,
        PK_LOGGER_DESC,
        RK_PRIORITY,
        RK_CATEGORY,
        RK_CATEGORY_EXTEND,
        RK_CATEGORY_FILTER,
        RK_IDENTIFY,
        RK_IDENTIFY_EXTEND,
        RK_IDENTIFY_FILTER,
        RK_OUT_TYPE,
        RK_FILE,
        RK_PATH,
        RK_LIMIT_SIZE,
        RK_ROLLBACK,
        RK_UDP_ADDR,
    };

#if __GNUG__ && __GNUC__ >= 6
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wclass-memaccess"
#endif


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
                if (end - begin > (int)sizeof("category_e") - 1)
                {
                    return *(begin + 9) == 'e' ? RK_CATEGORY_EXTEND : RK_CATEGORY_FILTER;
                }
                else
                {
                    return RK_CATEGORY;
                }
            }
            break;
        case 'd':
            if (*(begin+1) == 'e')
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
            if (end - begin > (int)sizeof("identify_e") - 1)
            {
                return *(begin + 9) == 'e' ? RK_IDENTIFY_EXTEND : RK_IDENTIFY_FILTER;
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
            return RK_ROLLBACK;
        case 'o':
            return RK_OUT_TYPE;
        case 's':
            return RK_SYNC;
        case 'u':
            return RK_UDP_ADDR;
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
        if (*begin == '0' || *begin == 'f')
        {
            return false;
        }
        return true;
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
    
    inline DeviceOutType ParseOutType(const char* begin, const char* end)
    {
        if (end <= begin)
        {
            return DEVICE_OUT_NULL;
        }
        switch (*begin)
        {
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

    inline void ParseAddres(const char* begin, const char* end, std::atomic_llong& ip, std::atomic_llong& port)
    {
        ip = 0;
        port = 0;
        if (end <= begin)
        {
            return;
        }
        const char* ip_begin = begin;
        while ((*ip_begin < '1' || *ip_begin > '9') && ip_begin != end)
        {
            ip_begin++;
        }
        const char* ip_end = ip_begin;
        while (((*ip_end >= '0' && *ip_end <= '9') || *ip_end == '.' ) && ip_end != end)
        {
            ip_end++;
        }
        if (ip_end <= ip_begin)
        {
            return;
        }

        const char* port_begin = ip_end;
        while ((*port_begin < '1' || *port_begin > '9') && port_begin != end)
        {
            port_begin++;
        }
        if (end <= port_begin)
        {
            return;
        }
        std::string str(ip_begin, ip_end - ip_begin);
        ip = inet_addr(str.c_str());
        str.assign(port_begin, end - port_begin);
        port = htons(atoi(str.c_str()));
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
    struct Line
    {
        int blank_;
        int line_type_;
        int block_type_;
        int key_;
        const char* key_begin_;
        const char* key_end_;
        const char* val_begin_;
        const char* val_end_;
    };

    struct LexState
    {
        int line_number_;
        const char* first_;
        const char* current_;
        const char* end_;
        Line line_;
        SHMLogger::Channels channels_;
        int channel_size_;
        bool hot_update_;
        char desc_[Logger::MAX_LOGGER_DESC_LEN];
        int desc_len_;
        char name_[Logger::MAX_LOGGER_NAME_LEN];
        int name_len_;
    };

    inline void InitState(LexState& state)
    {
        //static_assert(std::is_trivial<LexState>::value, "");
        memset(&state, 0, sizeof(state));
    }

    inline int Lex(LexState& ls)
    {
        memset(&ls.line_, 0, sizeof(ls.line_));
        while (true)
        {
            char ch = *ls.current_++;
            if (ls.line_.block_type_ == BLOCK_CLEAN && ch != '\0' && ch != '\r' && ch != '\n')
            {
                continue;
            }

            //preprocess
            if (ls.line_.block_type_ == BLOCK_KEY && (ch < 'a' || ch > 'z') && ch != '_')
            {
                ls.line_.block_type_ = BLOCK_PRE_SEP;
                ls.line_.key_end_ = ls.current_ - 1;
                ls.line_.key_ = ParseReserve(ls.line_.key_begin_, ls.line_.key_end_);
                if (ls.line_.key_ == RK_NULL)
                {
                    return PEC_ILLEGAL_KEY;
                }
            }
            if (ls.line_.block_type_ == BLOCK_VAL)
            {
                switch (ch)
                {
                case '\0': case'\n':case '\r': case '#': case '\"':
                    ls.line_.block_type_ = BLOCK_CLEAN;
                    ls.line_.val_end_ = ls.current_ - 1;
                    break;
                }
            }

            //end of line check
            switch (ch)
            {
            case '\0': case '\n':case '\r': case '#':
                if (ls.line_.block_type_ == BLOCK_BLANK)
                {
                    ls.line_.block_type_ = BLOCK_CLEAN;
                    ls.line_.line_type_ = LINE_BLANK;
                }
                if (ls.line_.block_type_ != BLOCK_CLEAN)
                {
                    return PEC_NOT_CLOSURE;
                }
                break;
            }

            //process
            switch (ch)
            {
            case ' ': case '\f': case '\t': case '\v': case '\"':
                if (ls.line_.block_type_ == BLOCK_BLANK)
                {
                    ls.line_.blank_++;
                    break;
                }
                break;
            case '\n':case '\r':
                ls.line_number_++;
                if ((*ls.current_ == '\r' || *ls.current_ == '\n') && *ls.current_ != ch)
                {
                    ls.current_++; //skip '\n\r' or '\r\n'
                }
                return PEC_NONE;
            case '\0':
                if (ls.line_.line_type_ != LINE_BLANK)
                {
                    ls.current_--;
                    return PEC_NONE;
                }
                else
                {
                    ls.line_.line_type_ = LINE_EOF;
                    return PEC_NONE;
                }
                
            case '-':
                if (ls.line_.block_type_ == BLOCK_BLANK)
                {
                    ls.line_.block_type_ = BLOCK_PRE_KEY;
                    ls.line_.line_type_ = LINE_ARRAY;
                    break;
                }
                else if (ls.line_.block_type_ != BLOCK_VAL)
                {
                    return PEC_ILLEGAL_CHARACTER;
                }
                break;
            case ':':
                if (ls.line_.block_type_ == BLOCK_PRE_SEP)
                {
                    ls.line_.block_type_ = BLOCK_PRE_VAL;
                    break;
                }
                else if (ls.line_.block_type_ != BLOCK_VAL)
                {
                    return PEC_ILLEGAL_CHARACTER;
                }
                break;
            default:
                if ((ch >= 'a' && ch <= 'z')
                    || (ch >= 'A' && ch <= 'Z')
                    || (ch >= '0' && ch <= '9')
                    || ch == '_' || ch == '-' || ch == ':' || ch == '/' || ch == '.' || ch == ',' || ch == '$' || ch == '~')
                {
                    switch (ls.line_.block_type_)
                    {
                    case BLOCK_CLEAN: case BLOCK_KEY: case BLOCK_VAL:
                        break;
                    case BLOCK_BLANK: case BLOCK_PRE_KEY:
                        ls.line_.block_type_ = BLOCK_KEY;
                        ls.line_.key_begin_ = ls.current_ - 1;
                        break;
                    case BLOCK_PRE_VAL:
                        ls.line_.block_type_ = BLOCK_VAL;
                        ls.line_.val_begin_ = ls.current_ - 1;
                        break;
                    default:
                        return PEC_ILLEGAL_CHARACTER;
                    }
                    break;
                }
                else if (ls.line_.block_type_ != BLOCK_CLEAN)
                {
                    return PEC_ILLEGAL_CHARACTER;
                }
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
                ls.line_number_--;
                return ret;
            }
            if (ls.line_.line_type_ == LINE_EOF)
            {
                return ret;
            }
            if (ls.line_.line_type_ == LINE_BLANK)
            {
                continue;
            }
            
            if (ls.line_.blank_ <= indent)
            {
                ls.current_ = current;
                ls.line_number_--;
                ls.line_.line_type_ = LINE_BLANK;
                return 0;
            }

            switch (ls.line_.key_)
            {
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
            case RK_CATEGORY_FILTER:
                device.config_fields_[DEVICE_CFG_CATEGORY_FILTER] = ParseBitArray(ls.line_.val_begin_, ls.line_.val_end_);
            case RK_IDENTIFY:
                device.config_fields_[DEVICE_CFG_IDENTIFY] = atoll(ls.line_.val_begin_);
                break;
            case RK_IDENTIFY_EXTEND:
                device.config_fields_[DEVICE_CFG_IDENTIFY_EXTEND] = atoll(ls.line_.val_begin_);
                break;
            case RK_IDENTIFY_FILTER:
                device.config_fields_[DEVICE_CFG_IDENTIFY_FILTER] = ParseBitArray(ls.line_.val_begin_, ls.line_.val_end_);
                break;
            case RK_LIMIT_SIZE:
                device.config_fields_[DEVICE_CFG_FILE_LIMIT_SIZE] = atoll(ls.line_.val_begin_) * 1000*1000;
                break;
            case RK_ROLLBACK:
                device.config_fields_[DEVICE_CFG_FILE_ROLLBACK] = atoll(ls.line_.val_begin_);
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
                ParseAddres(ls.line_.val_begin_, ls.line_.val_end_, device.config_fields_[DEVICE_CFG_UDP_IP], device.config_fields_[DEVICE_CFG_UDP_PORT]);
                if (device.config_fields_[DEVICE_CFG_UDP_IP] == 0)
                {
                    return PEC_ILLEGAL_ADDR_IP;
                }
                if (device.config_fields_[DEVICE_CFG_UDP_PORT] == 0)
                {
                    return PEC_ILLEGAL_ADDR_PORT;
                }
                break;
            default:
                return PEC_UNDEFINED_DEVICE_KEY;
            }
        } while (ls.line_.line_type_ != LINE_EOF);
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
                ls.line_number_--;
                return ret;
            }
            if (ls.line_.line_type_ == LINE_EOF)
            {
                return ret;
            }
            if (ls.line_.line_type_ == LINE_BLANK)
            {
                continue;
            }

            if (ls.line_.blank_ <= indent)
            {
                ls.current_ = current;
                ls.line_number_--;
                ls.line_.line_type_ = LINE_BLANK;
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
            case RK_CATEGORY_FILTER:
                channel.config_fields_[CHANNEL_CFG_CATEGORY_FILTER] = ParseBitArray(ls.line_.val_begin_, ls.line_.val_end_);
                break;
            case RK_IDENTIFY:
                channel.config_fields_[CHANNEL_CFG_IDENTIFY] = atoi(ls.line_.val_begin_);
                break;
            case RK_IDENTIFY_EXTEND:
                channel.config_fields_[CHANNEL_CFG_IDENTIFY_EXTEND] = atoi(ls.line_.val_begin_);
                break;
            case RK_IDENTIFY_FILTER:
                channel.config_fields_[CHANNEL_CFG_IDENTIFY_FILTER] = ParseBitArray(ls.line_.val_begin_, ls.line_.val_end_);
                break;
            case RK_DEVICE:
                if (ls.line_.line_type_ != LINE_ARRAY)
                {
                    return PEC_DEVICE_NOT_ARRAY;
                }
                else
                {
                    int device_id = atoi(ls.line_.val_begin_);
                    if (channel.device_size_ >= Channel::MAX_DEVICE_SIZE)
                    {
                        return PEC_DEVICE_INDEX_OUT_MAX;
                    }
                    if (device_id != channel.device_size_)
                    {
                        return PEC_DEVICE_INDEX_NOT_SEQUENCE;
                    }
                    Device& device = channel.devices_[channel.device_size_++];
                    memset(&device, 0, sizeof(device));
                    device.device_id_ = device_id;
                    ret = ParseDevice(ls, device, ls.line_.blank_);
                    if (ret != PEC_NONE || ls.line_.line_type_ == LINE_EOF)
                    {
                        return ret;
                    }
                }
                break;
            default:
                return PEC_UNDEFINED_CHANNEL_KEY;
            }

        } while (ls.line_.line_type_ != LINE_EOF);
        return 0;
    }
    inline int ParseLogger(LexState& ls, const std::string& text)
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
        ls.line_.line_type_ = LINE_NULL;
        ls.line_number_ = 1;
        ls.desc_len_ = 0;
        ls.name_len_ = 0;
        do
        {
            const char* current = ls.current_;
            int ret = Lex(ls);
            if (ret != PEC_NONE)
            {
                ls.current_ = current;
                ls.line_number_--;
                return ret;
            }
            if (ls.line_.line_type_ == LINE_EOF)
            {
                break;
            }
            if (ls.line_.line_type_ == LINE_BLANK)
            {
                continue;
            }

            switch (ls.line_.key_)
            {
            case RK_HOT_UPDATE:
                ls.hot_update_ = ParseBool(ls.line_.val_begin_, ls.line_.val_end_);//"disable"
                break;
            case RK_LOGGER_NAME:
                ParseString(ls.line_.val_begin_, ls.line_.val_end_, ls.name_, Logger::MAX_LOGGER_NAME_LEN, ls.name_len_);
                break;
            case PK_LOGGER_DESC:
                ParseString(ls.line_.val_begin_, ls.line_.val_end_, ls.desc_, Logger::MAX_LOGGER_DESC_LEN, ls.desc_len_);
                break;
            case RK_CHANNEL:
                if (ls.line_.line_type_ != LINE_ARRAY)
                {
                    return PEC_CHANNEL_NOT_ARRAY;
                }
                else
                {
                    int channel_id = atoi(ls.line_.val_begin_);
                    if (ls.channel_size_ >= Logger::MAX_CHANNEL_SIZE)
                    {
                        return PEC_CHANNEL_INDEX_OUT_MAX;
                    }
                    if (ls.channel_size_ != channel_id)
                    {
                        return PEC_CHANNEL_INDEX_NOT_SEQUENCE;
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
        } while (ls.line_.line_type_ != LINE_EOF);
        if (ls.channel_size_ == 0)
        {
            return PEC_NO_ANY_CHANNEL;
        }
        return PEC_NONE;
    }

#if __GNUG__ && __GNUC__ >= 6
#pragma GCC diagnostic pop
#endif

}


#endif
