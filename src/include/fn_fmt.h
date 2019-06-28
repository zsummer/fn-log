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
#ifndef _FN_LOG_FMT_H_
#define _FN_LOG_FMT_H_

#include "fn_data.h"


namespace FNLog
{


#ifndef _WIN32
    struct PriorityRender
    {
        const char* const priority_name_;
        int priority_len_;
        const char* const scolor_;
    };
    static const PriorityRender PRIORITY_RENDER[] =
    {
        {"[TRACE]", sizeof("[TRACE]") - 1, "\e[0m",                     },
        {"[DEBUG]", sizeof("[DEBUG]") - 1, "\e[0m",                     },
        {"[INFO ]", sizeof("[INFO ]") - 1, "\e[34m\e[1m",/*hight blue*/},
        {"[WARN ]", sizeof("[WARN ]") - 1, "\e[33m", /*yellow*/         },
        {"[ERROR]", sizeof("[ERROR]") - 1, "\e[31m", /*red*/            },
        {"[ALARM]", sizeof("[ALARM]") - 1, "\e[32m", /*green*/          },
        {"[FATAL]", sizeof("[FATAL]") - 1, "\e[35m",                    },
    };
#else
    struct PriorityRender
    {
        const char* const priority_name_;
        int priority_len_;
        const WORD color_;
    };
    static const PriorityRender PRIORITY_RENDER[] =
    {
        {"[TRACE]", sizeof("[TRACE]") - 1,  FOREGROUND_INTENSITY,               },
        {"[DEBUG]", sizeof("[DEBUG]") - 1,  FOREGROUND_INTENSITY,               },
        {"[INFO ]", sizeof("[INFO ]") - 1,  FOREGROUND_BLUE | FOREGROUND_GREEN, },
        {"[WARN ]", sizeof("[WARN ]") - 1,  FOREGROUND_GREEN | FOREGROUND_RED,  },
        {"[ERROR]", sizeof("[ERROR]") - 1,  FOREGROUND_RED,                     },
        {"[ALARM]", sizeof("[ALARM]") - 1,  FOREGROUND_GREEN,                   },
        {"[FATAL]", sizeof("[FATAL]") - 1,  FOREGROUND_RED | FOREGROUND_BLUE,   },
    };
#endif



    static_assert(PRIORITY_TRACE == 0, "");
    static_assert(sizeof(PRIORITY_RENDER) / sizeof(PriorityRender) == PRIORITY_MAX, "");

    
    template<int WIDE>
    int write_bin_unsafe(char* dst, unsigned long long number);
    template<int WIDE>
    int write_dec_unsafe(char* dst, unsigned long long number);
    template<int WIDE>
    int write_hex_unsafe(char* dst, unsigned long long number);

    template<int WIDE>
    int write_bin_unsafe(char* dst, long long number)
    {
        return write_bin_unsafe<WIDE>(dst, (unsigned long long) number);
    }
    template<int WIDE>
    int write_hex_unsafe(char* dst, long long number)
    {
        return write_hex_unsafe<WIDE>(dst, (unsigned long long) number);
    }
    template<int WIDE>
    int write_dec_unsafe(char* dst, long long number)
    {
        if (number < 0)
        {
            *dst = '-';
            number = -number;
            return 1 + write_dec_unsafe<WIDE - 1>(dst + 1, (unsigned long long) number);
        }
        return write_dec_unsafe<WIDE>(dst, (unsigned long long) number);
    }


    template<int WIDE>
    int write_dec_unsafe(char* dst, unsigned long long number)
    {
        static const char* dec_lut =
            "00010203040506070809"
            "10111213141516171819"
            "20212223242526272829"
            "30313233343536373839"
            "40414243444546474849"
            "50515253545556575859"
            "60616263646566676869"
            "70717273747576777879"
            "80818283848586878889"
            "90919293949596979899";

        static const int buf_len = 30;
        char buf[buf_len];
        int write_index = buf_len;
        unsigned long long m1 = 0;
        unsigned long long m2 = 0;
        do
        {
            m1 = number / 100;
            m2 = number % 100;
            m2 += m2;
            number = m1;
            *(buf + write_index - 1) = dec_lut[m2 + 1];
            *(buf + write_index - 2) = dec_lut[m2];
            write_index -= 2;
        } while (number);
        if (buf[write_index] == '0')
        {
            write_index++;
        }
        while (buf_len - write_index < WIDE)
        {
            write_index--;
            buf[write_index] = '0';
        }
        memcpy(dst, buf + write_index, buf_len - write_index);
        return buf_len - write_index;
    }


    template<int WIDE>
    int write_hex_unsafe(char* dst, unsigned long long number)
    {
        static const char* lut =
            "0123456789ABCDEFGHI";
        static const char* hex_lut =
            "000102030405060708090A0B0C0D0E0F"
            "101112131415161718191A1B1C1D1E1F"
            "202122232425262728292A2B2C2D2E2F"
            "303132333435363738393A3B3C3D3E3F"
            "404142434445464748494A4B4C4D4E4F"
            "505152535455565758595A5B5C5D5E5F"
            "606162636465666768696A6B6C6D6E6F"
            "707172737475767778797A7B7C7D7E7F"
            "808182838485868788898A8B8C8D8E8F"
            "909192939495969798999A9B9C9D9E9F"
            "A0A1A2A3A4A5A6A7A8A9AAABACADAEAF"
            "B0B1B2B3B4B5B6B7B8B9BABBBCBDBEBF"
            "C0C1C2C3C4C5C6C7C8C9CACBCCCDCECF"
            "D0D1D2D3D4D5D6D7D8D9DADBDCDDDEDF"
            "E0E1E2E3E4E5E6E7E8E9EAEBECEDEEEF"
            "F0F1F2F3F4F5F6F7F8F9FAFBFCFDFEFF";



        int real_wide = 0;
#ifndef _WIN32
        real_wide = sizeof(number) * 8 - __builtin_clzll(number);
#else
        unsigned long win_index = 0;
        if (_BitScanReverse(&win_index, (unsigned long)(number >> 32)))
        {
            real_wide = win_index + 1 + 32;
        }
        else if(_BitScanReverse(&win_index, (unsigned long)(number & 0xffffffff)))
        {
            real_wide = win_index + 1;
        }
#endif 
        switch (real_wide)
        {
        case  1:case  2:case  3:case  4:real_wide = 1; break;
        case  5:case  6:case  7:case  8:real_wide = 2; break;
        case 9: case 10:case 11:case 12:real_wide = 3; break;
        case 13:case 14:case 15:case 16:real_wide = 4; break;
        case 17:case 18:case 19:case 20:real_wide = 5; break;
        case 21:case 22:case 23:case 24:real_wide = 6; break;
        case 25:case 26:case 27:case 28:real_wide = 7; break;
        case 29:case 30:case 31:case 32:real_wide = 8; break;
        case 33:case 34:case 35:case 36:real_wide = 9; break;
        case 37:case 38:case 39:case 40:real_wide = 10; break;
        case 41:case 42:case 43:case 44:real_wide = 11; break;
        case 45:case 46:case 47:case 48:real_wide = 12; break;
        case 49:case 50:case 51:case 52:real_wide = 13; break;
        case 53:case 54:case 55:case 56:real_wide = 14; break;
        case 57:case 58:case 59:case 60:real_wide = 15; break;
        case 61:case 62:case 63:case 64:real_wide = 16; break;
        }
        if (real_wide < WIDE)
        {
            real_wide = WIDE;
        }
        unsigned long long cur_wide = real_wide;
        while (number && cur_wide >= 2)
        {
            const unsigned long long m2 = (unsigned long long)((number % 256) * 2);
            number /= 256;
            *(dst + cur_wide - 1) = hex_lut[m2 + 1];
            *(dst + cur_wide - 2) = hex_lut[m2];
            cur_wide -= 2;
        } 
        if (number)
        {
            *dst = lut[number % 16];
            cur_wide --;
        }
        while (cur_wide-- != 0)
        {
            *(dst + cur_wide) = '0';
        }
        return real_wide;
    }


    template<int WIDE>
    int write_bin_unsafe(char* dst, unsigned long long number)
    {
        static const char* lut =
            "0123456789abcdefghijk";

        int real_wide = 0;
#ifndef _WIN32
        real_wide = sizeof(number) * 8 - __builtin_clzll(number);
#else
        unsigned long win_index = 0;
        _BitScanReverse64(&win_index, number);
        real_wide = (int)win_index + 1;
#endif 
        if (real_wide < WIDE)
        {
            real_wide = WIDE;
        }
        unsigned long long cur_wide = real_wide;
        do
        {
            const unsigned long long m2 = number & 1;
            number >>= 1;
            *(dst + cur_wide - 1) = lut[m2];
            cur_wide--;
        } while (number);
        return real_wide;
    }

    inline int write_double_unsafe(char* dst, double number)
    {
        int fp_class = std::fpclassify(number);
        switch (fp_class)
        {
        case FP_NAN:
            memcpy(dst, "nan", 3);
            return 3;
        case FP_INFINITE:
            memcpy(dst, "inf", 3);
            return 3;
        }


        double fabst = std::fabs(number);

        
        if (fabst < 0.0001 || fabst > 0xFFFFFFFFFFFFFFFULL)
        {
            gcvt(number, 16, dst);
            int len = (int)strlen(dst);
            return len;
        }

        if (number < 0.0)
        {
            double intpart = 0;
            unsigned long long fractpart = (unsigned long long)(modf(fabst, &intpart) * 10000);
            *dst = '-';
            int writed_len = 1 + write_dec_unsafe<0>(dst + 1, (unsigned long long)intpart);
            if (fractpart > 0)
            {
                *(dst + writed_len) = '.';
                return writed_len + 1 + write_dec_unsafe<4>(dst + writed_len + 1, (unsigned long long)fractpart);
            }
            return writed_len;
        }

        double intpart = 0;
        unsigned long long fractpart = (unsigned long long)(modf(fabst, &intpart) * 10000);
        int writed_len = write_dec_unsafe<0>(dst, (unsigned long long)intpart);
        if (fractpart > 0)
        {
            *(dst + writed_len) = '.';
            writed_len++;
            int wide = 4;
            int fp = (int)fractpart;
            while (fp % 10 == 0 && wide > 1)
            {
                wide--;
                fp /= 10;
            }
            writed_len += wide;
            fp = (int)fractpart;
            switch (wide)
            {
            case 1:
                *(dst + writed_len - 1) = "0123456789"[fp / 1000];
                break;
            case 2:
                *(dst + writed_len - 2) = "0123456789"[fp / 1000];
                *(dst + writed_len - 1) = "0123456789"[fp / 100 % 10];
                break;
            case 3:
                *(dst + writed_len - 3) = "0123456789"[fp / 1000];
                *(dst + writed_len - 2) = "0123456789"[fp / 100 % 10];
                *(dst + writed_len - 1) = "0123456789"[fp / 10 % 10];
                break;
            case 4:
                *(dst + writed_len - 4) = "0123456789"[fp / 1000];
                *(dst + writed_len - 3) = "0123456789"[fp / 100 % 10];
                *(dst + writed_len - 2) = "0123456789"[fp / 10 % 10];
                *(dst + writed_len - 1) = "0123456789"[fp % 10];
                break;
            default:
                break;
            }
        }
        return writed_len;
    }

    inline int write_float_unsafe(char* dst, float number)
    {
        if (std::isnan(number))
        {
            memcpy(dst, "nan", 3);
            return 3;
        }
        else if (std::isinf(number))
        {
            memcpy(dst, "inf", 3);
            return 3;
        }

        double fabst = std::fabs(number);


        if (fabst < 0.0001 || fabst > 0xFFFFFFFULL)
        {
            gcvt(number, 7, dst);
            int len = (int)strlen(dst);
            return len;
        }

        if (number < 0.0)
        {
            double intpart = 0;
            unsigned long long fractpart = (unsigned long long)(modf(fabst, &intpart) * 10000);
            *dst = '-';
            int writed_len = 1 + write_dec_unsafe<0>(dst + 1, (unsigned long long)intpart);
            if (fractpart > 0)
            {
                *(dst + writed_len) = '.';
                return writed_len + 1 + write_dec_unsafe<4>(dst + writed_len + 1, (unsigned long long)fractpart);
            }
            return writed_len;
        }

        double intpart = 0;
        unsigned long long fractpart = (unsigned long long)(modf(fabst, &intpart) * 10000);
        int writed_len = write_dec_unsafe<0>(dst, (unsigned long long)intpart);
        if (fractpart > 0)
        {
            *(dst + writed_len) = '.';
            writed_len++;
            int wide = 4;
            int fp = (int)fractpart;
            while (fp %10 == 0 && wide > 1)
            {
                wide--;
                fp /= 10;
            }
            writed_len += wide;
            fp = (int)fractpart;
            switch (wide)
            {
            case 1:
                *(dst + writed_len - 1) = "0123456789"[fp / 1000];
                break;
            case 2:
                *(dst + writed_len - 2) = "0123456789"[fp / 1000];
                *(dst + writed_len - 1) = "0123456789"[fp / 100 % 10];
                break;
            case 3:
                *(dst + writed_len - 3) = "0123456789"[fp / 1000];
                *(dst + writed_len - 2) = "0123456789"[fp / 100 % 10];
                *(dst + writed_len - 1) = "0123456789"[fp / 10 % 10];
                break;
            case 4:
                *(dst + writed_len - 4) = "0123456789"[fp / 1000];
                *(dst + writed_len - 3) = "0123456789"[fp / 100 % 10];
                *(dst + writed_len - 2) = "0123456789"[fp / 10 % 10];
                *(dst + writed_len - 1) = "0123456789"[fp % 10];
                break;
            default:
                break;
            }
        }
        return writed_len;
    }

    inline int write_date_unsafe(char* dst, long long timestamp, unsigned int precise)
    {
        static thread_local tm cache_date = { 0 };
        static thread_local long long cache_timestamp = 0;
        static const char date_fmt[] = "[20190412 13:05:35.417]";
        long long day_second = timestamp - cache_timestamp;
        if (day_second < 0 || day_second >= 24*60*60)
        {
            cache_date = FileHandler::time_to_tm((time_t)timestamp);
            struct tm daytm = cache_date;
            daytm.tm_hour = 0;
            daytm.tm_min = 0;
            daytm.tm_sec = 0;
            cache_timestamp = mktime(&daytm);
            day_second = timestamp - cache_timestamp;
        }
        int write_bytes = 0;

        *(dst + write_bytes++) = '[';

        write_bytes += write_dec_unsafe<4>(dst + write_bytes, (unsigned long long)cache_date.tm_year + 1900);
        write_bytes += write_dec_unsafe<2>(dst + write_bytes, (unsigned long long)cache_date.tm_mon + 1);
        write_bytes += write_dec_unsafe<2>(dst + write_bytes, (unsigned long long)cache_date.tm_mday);

        *(dst + write_bytes++) = ' ';

        write_bytes += write_dec_unsafe<2>(dst + write_bytes, (unsigned long long)day_second/3600);
        *(dst + write_bytes++) = ':';
        day_second %= 3600;
        write_bytes += write_dec_unsafe<2>(dst + write_bytes, (unsigned long long)day_second / 60);
        *(dst + write_bytes++) = ':';
        day_second %= 60;
        write_bytes += write_dec_unsafe<2>(dst + write_bytes, (unsigned long long)day_second);

        *(dst + write_bytes++) = '.';
        if (precise >= 1000)
        {
            precise = 999;
        }
        write_bytes += write_dec_unsafe<3>(dst + write_bytes, (unsigned long long)precise);

        *(dst + write_bytes++) = ']';

        if (write_bytes != sizeof(date_fmt) - 1)
        {
            return 0;
        }

        return write_bytes;
    }

    inline int write_log_priority_unsafe(char* dst, int priority)
    {
        priority = priority % PRIORITY_MAX;
        memcpy(dst, PRIORITY_RENDER[priority].priority_name_, PRIORITY_RENDER[priority].priority_len_);
        return PRIORITY_RENDER[priority].priority_len_;
    }

    inline int write_log_thread_unsafe(char* dst, unsigned int thread_id)
    {
        int write_bytes = 0;
        *(dst + write_bytes) = ' ';
        write_bytes++;
        *(dst + write_bytes) = '[';
        write_bytes++;
        write_bytes += write_dec_unsafe<0>(dst + write_bytes, (unsigned long long) thread_id);
        *(dst + write_bytes) = ']';
        write_bytes++;
        return write_bytes;
    }

    inline int write_pointer_unsafe(char* dst, const void* ptr)
    {
        if (ptr == nullptr)
        {
            memcpy(dst, "null", 4);
            return 4;
        }
        int write_bytes = 2;
        memcpy(dst, "0x", 2);
        write_bytes += write_hex_unsafe<0>(dst + write_bytes, (unsigned long long) ptr);
        return write_bytes;
    }

}


#endif
