
/*
* Copyright (C) 2019 YaweiZhang <yawei.zhang@foxmail.com>.
* All rights reserved
* This file is part of the fn-log, used MIT License.
*/


#pragma once
#ifndef _FN_LOG_OUT_UDP_DEVICE_H_
#define _FN_LOG_OUT_UDP_DEVICE_H_

#include "fn_data.h"
#include "fn_fmt.h"

namespace FNLog
{

    inline void EnterProcOutUDPDevice(Logger& logger, int channel_id, int device_id, LogData& log)
    {
        auto& udp = logger.udp_handles_[channel_id * Channel::MAX_DEVICE_SIZE + device_id];
        Device& device = logger.shm_->channels_[channel_id].devices_[device_id];

        if (!udp.is_open())
        {
            udp.open();
        }
        if (!udp.is_open())
        {
            AtomicAddL(device, DEVICE_LOG_TOTAL_LOSE_LINE);
            return;
        }
        
        long long ip = AtomicLoadC(device, DEVICE_CFG_UDP_IP);
        long long port = AtomicLoadC(device, DEVICE_CFG_UDP_PORT);
        int ret = udp.write((unsigned long)ip, (unsigned short)port, log.content_, log.content_len_);
        if (ret <= 0)
        {
            AtomicAddL(device, DEVICE_LOG_TOTAL_LOSE_LINE);
        }
        AtomicAddL(device, DEVICE_LOG_TOTAL_WRITE_LINE);
        AtomicAddLV(device, DEVICE_LOG_TOTAL_WRITE_BYTE, log.content_len_);
        AtomicAddLV(device, DEVICE_LOG_PRIORITY + log.priority_, log.content_len_);
    }
}


#endif
