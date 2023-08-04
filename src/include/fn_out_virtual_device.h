
/*
* Copyright (C) 2019 YaweiZhang <yawei.zhang@foxmail.com>.
* All rights reserved
* This file is part of the fn-log, used MIT License.
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
            unsigned long long field_category_mask = (unsigned long long)fields[FNLog::DEVICE_CFG_CATEGORY_MASK];
            long long field_begin_identify = fields[FNLog::DEVICE_CFG_IDENTIFY];
            long long field_identify_count = fields[FNLog::DEVICE_CFG_IDENTIFY_EXTEND];
            unsigned long long field_identify_mask = (unsigned long long)fields[FNLog::DEVICE_CFG_IDENTIFY_MASK];

            if (field_category_count > 0 && (log.category_ < field_begin_category || log.category_ >= field_begin_category + field_category_count))
            {
                return;
            }

            if (field_identify_count > 0 && (log.identify_ < field_begin_identify || log.identify_ >= field_begin_identify + field_identify_count))
            {
                return;
            }
            if (field_category_mask && (field_category_mask & ((1ULL) << (unsigned int)log.category_)) == 0)
            {
                return;
            }
            if (field_identify_mask && (field_identify_mask & ((1ULL) << (unsigned int)log.identify_)) == 0)
            {
                return;
            }

            int content_len_ = FN_MIN(log.content_len_, LogData::LOG_SIZE - 1);
            log.content_[content_len_] = '\0'; //virtual device hook maybe direct used content like c-string 

            Device& device = logger.shm_->channels_[channel_id].devices_[device_id];
            AtomicAddL(device, DEVICE_LOG_TOTAL_WRITE_LINE);
            AtomicAddLV(device, DEVICE_LOG_TOTAL_WRITE_BYTE, log.content_len_);
            AtomicAddLV(device, DEVICE_LOG_PRIORITY + log.priority_, log.content_len_);
            (*RefVirtualDevice())(log);
        }
    }
}


#endif
