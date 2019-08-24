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
    enum LineType
    {
        LINE_NULL,
        LINE_ARRAY,
        LINE_BLANK,
        LINE_ERROR,
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
        RK_PRIORITY,
        RK_CATEGORY,
        RK_CATEGORY_EXTEND,
        RK_OUT_TYPE,
        RK_FILE,
        RK_PATH,
        RK_LIMIT_SIZE,
        RK_ROLLBACK,
        RK_UDP_ADDR,
    };
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
                if (end - begin > (int)sizeof("category") - 1)
                {
                    return RK_CATEGORY_EXTEND;
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
        case 'l':
            return RK_LIMIT_SIZE;
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

    inline ChannelType ParseChannelType(const char* begin, const char* end)
    {
        if (end <= begin)
        {
            return CHANNEL_MULTI;
        }
        switch (*begin)
        {
            case 'm': case 'M':
                return CHANNEL_MULTI;
            case 's': case 'S':
                return CHANNEL_SYNC;
            case 'r': case 'R':
                return CHANNEL_RING;
            case 'a':case 'A':
                return CHANNEL_MULTI;
        }
        return CHANNEL_MULTI;
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
        }
        return DEVICE_OUT_NULL;
    }

    inline void ParseAddres(const char* begin, const char* end, long long& ip, long long& port)
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
        Line line_;
        Logger::Channels channels_;
        int channel_size_;
        bool hot_update_;
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
            const char& ch = *ls.current_++;
            if (ls.line_.block_type_ == BLOCK_CLEAN && ch != '\0' && ch != '\r' && ch != '\n')
            {
                continue;
            }

            //preprocess
            if (ls.line_.block_type_ == BLOCK_KEY && (ch < 'a' || ch > 'z') && ch != '_')
            {
                ls.line_.block_type_ = BLOCK_PRE_SEP;
                ls.line_.key_end_ = &ch;
                ls.line_.key_ = ParseReserve(ls.line_.key_begin_, ls.line_.key_end_);
                if (ls.line_.key_ == RK_NULL)
                {
                    ls.line_.line_type_ = LINE_ERROR;
                    return ls.line_.line_type_;
                }
            }
            if (ls.line_.block_type_ == BLOCK_VAL)
            {
                switch (ch)
                {
                case '\0': case'\n':case '\r': case '#': case '\"':
                    ls.line_.block_type_ = BLOCK_CLEAN;
                    ls.line_.val_end_ = &ch;
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
                    ls.line_.line_type_ = LINE_ERROR;
                    return ls.line_.line_type_;
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
                return ls.line_.line_type_;
            case '\0':
                if (ls.line_.line_type_ != LINE_BLANK)
                {
                    ls.current_--;
                    return ls.line_.line_type_;
                }
                else
                {
                    ls.line_.line_type_ = LINE_EOF;
                    return ls.line_.line_type_;
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
                    ls.line_.line_type_ = LINE_ERROR;
                    return ls.line_.line_type_;
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
                    ls.line_.line_type_ = LINE_ERROR;
                    return ls.line_.line_type_;
                }
                break;
            default:
                if ((ch >= 'a' && ch <= 'z')
                    || (ch >= 'A' && ch <= 'Z')
                    || (ch >= '0' && ch <= '9')
                    || ch == '_' || ch == '-' || ch == ':' || ch == '/' || ch == '.' || ch == '$' || ch == '~')
                {
                    switch (ls.line_.block_type_)
                    {
                    case BLOCK_CLEAN: case BLOCK_KEY: case BLOCK_VAL:
                        break;
                    case BLOCK_BLANK: case BLOCK_PRE_KEY:
                        ls.line_.block_type_ = BLOCK_KEY;
                        ls.line_.key_begin_ = &ch;
                        break;
                    case BLOCK_PRE_VAL:
                        ls.line_.block_type_ = BLOCK_VAL;
                        ls.line_.val_begin_ = &ch;
                        break;
                    default:
                        ls.line_.line_type_ = LINE_ERROR;
                        return ls.line_.line_type_;
                    }
                    break;
                }
                else if (ls.line_.block_type_ != BLOCK_CLEAN)
                {
                    ls.line_.line_type_ = LINE_ERROR;
                    return ls.line_.line_type_;
                }
            }
        }
        ls.line_.line_type_ = LINE_ERROR;
        return ls.line_.line_type_;
    }

    inline int ParseDevice(LexState& ls, Device& device, int indent)
    {
        do
        {
            const char* current = ls.current_;
            int line_state = Lex(ls);
            if (line_state == LINE_ERROR)
            {
                return line_state;
            }
            if (line_state == LINE_EOF)
            {
                return LINE_EOF;
            }
            if (ls.line_.line_type_ == LINE_BLANK)
            {
                continue;
            }
            if (ls.line_.blank_ <= indent)
            {
                ls.current_ = current;
                ls.line_number_--;
                return 0;
            }
            switch (ls.line_.key_)
            {
            case RK_OUT_TYPE:
                device.out_type_ = ParseOutType(ls.line_.val_begin_, ls.line_.val_end_);
                break;
            case RK_DISABLE:
                device.config_fields_[DEVICE_CFG_ABLE].num_ = !ParseBool(ls.line_.val_begin_, ls.line_.val_end_); //"disable"
                break;
            case RK_PRIORITY:
                device.config_fields_[DEVICE_CFG_PRIORITY].num_ = ParsePriority(ls.line_.val_begin_, ls.line_.val_end_);
                break;
            case RK_CATEGORY:
                device.config_fields_[DEVICE_CFG_CATEGORY].num_ = atoll(ls.line_.val_begin_);
                break;
            case RK_CATEGORY_EXTEND:
                device.config_fields_[DEVICE_CFG_CATEGORY_EXTEND].num_ = atoll(ls.line_.val_begin_);
                break;
            case RK_LIMIT_SIZE:
                device.config_fields_[DEVICE_CFG_FILE_LIMIT_SIZE].num_ = atoll(ls.line_.val_begin_) * 1000*1000;
                break;
            case RK_ROLLBACK:
                device.config_fields_[DEVICE_CFG_FILE_ROLLBACK].num_ = atoll(ls.line_.val_begin_);
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
                if (ls.line_.val_end_ - ls.line_.val_begin_ < Device::MAX_NAME_LEN - 1
                    && ls.line_.val_end_ - ls.line_.val_begin_ >= 1)
                {
                    memcpy(device.out_file_, ls.line_.val_begin_, ls.line_.val_end_ - ls.line_.val_begin_);
                    device.out_file_[ls.line_.val_end_ - ls.line_.val_begin_] = '\0';
                }
                break;
            case RK_UDP_ADDR:
                ParseAddres(ls.line_.val_begin_, ls.line_.val_end_, device.config_fields_[DEVICE_CFG_UDP_IP].num_, device.config_fields_[DEVICE_CFG_UDP_PORT].num_);
                break;
            default:
                return LINE_ERROR;
            }
        } while (true);
        return 0;
    }
    inline int ParseChannel(LexState& ls, Channel& channel, int indent)
    {
        do
        {
            const char* current = ls.current_;
            int line_state = Lex(ls);
            if (line_state == LINE_ERROR)
            {
                return line_state;
            }
            if (line_state == LINE_EOF)
            {
                return LINE_EOF;
            }
            if (ls.line_.line_type_ == LINE_BLANK)
            {
                continue;
            }
            if (ls.line_.blank_ <= indent)
            {
                ls.current_ = current;
                ls.line_number_--;
                return 0;
            }
            switch (ls.line_.key_)
            {
            case RK_SYNC:
                channel.channel_type_ = ParseChannelType(ls.line_.val_begin_, ls.line_.val_end_);
                break;
            case RK_PRIORITY:
                channel.config_fields_[CHANNEL_CFG_PRIORITY].num_ = ParsePriority(ls.line_.val_begin_, ls.line_.val_end_);
                break;
            case RK_CATEGORY:
                channel.config_fields_[CHANNEL_CFG_CATEGORY].num_ = atoi(ls.line_.val_begin_);
                break;            
            case RK_CATEGORY_EXTEND:
                channel.config_fields_[CHANNEL_CFG_CATEGORY_EXTEND].num_ = atoi(ls.line_.val_begin_);
                break;
            case RK_DEVICE:
                if (ls.line_.line_type_ != LINE_ARRAY)
                {
                    ls.line_.line_type_ = LINE_ERROR;
                    return ls.line_.line_type_;
                }
                else
                {
                    int device_id = atoi(ls.line_.val_begin_);
                    if (channel.device_size_ >= Channel::MAX_DEVICE_SIZE || device_id != channel.device_size_)
                    {
                        ls.line_.line_type_ = LINE_ERROR;
                        return ls.line_.line_type_;
                    }

                    Device& device = channel.devices_[channel.device_size_++];
                    memset(&device, 0, sizeof(device));
                    device.device_id_ = device_id;
                    line_state = ParseDevice(ls, device, ls.line_.blank_);
                    if (line_state == LINE_EOF || line_state == LINE_ERROR)
                    {
                        return line_state;
                    }
                }
                break;
            default:
                return LINE_ERROR;
            }

        } while (true);
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
        memset(&ls.channels_, 0, sizeof(ls.channels_));
        ls.channel_size_ = 0;
        ls.hot_update_ = false;
        ls.current_ = ls.first_;
        ls.line_number_ = 1;
        do
        {
            int line_state = Lex(ls);
            if (line_state == LINE_ERROR)
            {
                return line_state;
            }
            if (line_state == LINE_EOF)
            {
                return 0;
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
            case RK_CHANNEL:
                if (ls.line_.line_type_ != LINE_ARRAY)
                {
                    ls.line_.line_type_ = LINE_ERROR;
                    return ls.line_.line_type_;
                }
                else
                {
                    int channel_id = atoi(ls.line_.val_begin_);
                    if (ls.channel_size_ >= Logger::MAX_CHANNEL_SIZE || ls.channel_size_ != channel_id)
                    {
                        ls.line_.line_type_ = LINE_ERROR;
                        return ls.line_.line_type_;
                    }

                    Channel& channel = ls.channels_[ls.channel_size_++];
                    memset(&channel, 0, sizeof(channel));
                    channel.channel_id_ = channel_id;
                    line_state = ParseChannel(ls, channel, ls.line_.blank_);
                    if (line_state == LINE_EOF)
                    {
                        return 0;
                    }
                    if (line_state == LINE_ERROR)
                    {
                        return ls.line_.line_type_;
                    }
                }
                break;
            default:
                return LINE_ERROR;
            }
        } while (true);
        return 0;
    }

}


#endif
