
/*
* Copyright (C) 2019 YaweiZhang <yawei.zhang@foxmail.com>.
* All rights reserved
* This file is part of the fn-log, used MIT License.
*/


#pragma once
#ifndef _FN_LOG_OUT_EMPTY_DEVICE_H_
#define _FN_LOG_OUT_EMPTY_DEVICE_H_

#include "fn_data.h"
#include "fn_fmt.h"

namespace FNLog
{

    inline void EnterProcOutEmptyDevice(Logger& logger, int channel_id, int device_id, LogData& log)
    {
        Device& device = logger.shm_->channels_[channel_id].devices_[device_id];
        AtomicAddL(device, DEVICE_LOG_TOTAL_WRITE_LINE);
        AtomicAddLV(device, DEVICE_LOG_TOTAL_WRITE_BYTE, log.content_len_);
        AtomicAddLV(device, DEVICE_LOG_PRIORITY + log.priority_, log.content_len_);
    }

}


#endif
