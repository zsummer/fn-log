/*
 *
 * MIT License
 *
 * Copyright (C) 2021 YaweiZhang <yawei.zhang@foxmail.com>.
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
#ifndef _FN_LOG_OUT_VIRTUAL_DEVICE_H_
#define _FN_LOG_OUT_VIRTUAL_DEVICE_H_

#include "fn_data.h"
#include "fn_fmt.h"

namespace FNLog
{
    using VirtualDevicePtr = void (*)(const LogData& log);

    inline VirtualDevicePtr& RefVirtualDevice()
    {
        static VirtualDevicePtr g_virtual_device_ptr = NULL;
        return g_virtual_device_ptr;
    }
    inline void SetVirtualDevice(VirtualDevicePtr vdp)
    {
        RefVirtualDevice() = vdp;
    }

    //the virtual device like log hook;  this virtual device call at the log create time(thread) not at log write thread .    
    //can used translate log  
    inline void EnterProcOutVirtualDevice(Logger& logger, int channel_id, int device_id, LogData& log)
    {
        if (RefVirtualDevice())
        {
            Channel& channel = logger.shm_->channels_[channel_id];
            Device::ConfigFields& fields = channel.devices_[device_id].config_fields_;
            long long field_begin_category = fields[FNLog::DEVICE_CFG_CATEGORY];
            long long field_category_count = fields[FNLog::DEVICE_CFG_CATEGORY_EXTEND];
            unsigned long long field_category_filter = (unsigned long long)fields[FNLog::DEVICE_CFG_CATEGORY_FILTER];
            long long field_begin_identify = fields[FNLog::DEVICE_CFG_IDENTIFY];
            long long field_identify_count = fields[FNLog::DEVICE_CFG_IDENTIFY_EXTEND];
            unsigned long long field_identify_filter = (unsigned long long)fields[FNLog::DEVICE_CFG_IDENTIFY_FILTER];

            if (field_category_count > 0 && (log.category_ < field_begin_category || log.category_ >= field_begin_category + field_category_count))
            {
                return;
            }

            if (field_identify_count > 0 && (log.identify_ < field_begin_identify || log.identify_ >= field_begin_identify + field_identify_count))
            {
                return;
            }
            if (field_category_filter && (field_category_filter & ((1ULL) << (unsigned int)log.category_)) == 0)
            {
                return;
            }
            if (field_identify_filter && (field_identify_filter & ((1ULL) << (unsigned int)log.identify_)) == 0)
            {
                return;
            }

            int content_len_ = FN_MIN(log.content_len_, LogData::LOG_SIZE - 1);
            log.content_[content_len_] = '\0'; //virtual device hook maybe direct used content like c-string 

            Device& device = logger.shm_->channels_[channel_id].devices_[device_id];
            AtomicAddL(device, DEVICE_LOG_TOTAL_WRITE_LINE);
            AtomicAddLV(device, DEVICE_LOG_TOTAL_WRITE_BYTE, log.content_len_);
            (*RefVirtualDevice())(log);
        }
    }
}


#endif
