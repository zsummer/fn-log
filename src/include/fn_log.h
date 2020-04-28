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
#ifndef _FN_LOG_LOG_H_
#define _FN_LOG_LOG_H_

#include "fn_data.h"
#include "fn_parse.h"
#include "fn_load.h"
#include "fn_core.h"
#include "fn_stream.h"
#include "fn_macro.h"

namespace FNLog   
{

    //inline void EnableAllChannel(Logger& logger, bool enable);
    inline void EnableAllFileDevice(Logger& logger, bool enable) { BatchSetDeviceConfig(logger, DEVICE_OUT_FILE, DEVICE_CFG_ABLE, enable); }
    inline void EnableAllScreenDevice(Logger& logger, bool enable) { BatchSetDeviceConfig(logger, DEVICE_OUT_SCREEN, DEVICE_CFG_ABLE, enable); }
    inline void EnableAllUDPDevice(Logger& logger, bool enable) { BatchSetDeviceConfig(logger, DEVICE_OUT_UDP, DEVICE_CFG_ABLE, enable); }

    inline void SetAllChannelPriority(Logger& logger, LogPriority priority) { BatchSetChannelConfig(logger, CHANNEL_CFG_PRIORITY, priority); }
    inline void SetAllFilePriority(Logger& logger, LogPriority priority) { BatchSetDeviceConfig(logger, DEVICE_OUT_FILE, DEVICE_CFG_PRIORITY, priority); }
    inline void SetAllScreenPriority(Logger& logger, LogPriority priority) { BatchSetDeviceConfig(logger, DEVICE_OUT_SCREEN, DEVICE_CFG_PRIORITY, priority); }
    inline void SetAllUDPPriority(Logger& logger, LogPriority priority) { BatchSetDeviceConfig(logger, DEVICE_OUT_UDP, DEVICE_CFG_PRIORITY, priority); }

#define BatchSetChannelCategoryMacro(begin, count) \
    BatchSetChannelConfig(logger, CHANNEL_CFG_CATEGORY, begin);\
    BatchSetChannelConfig(logger, CHANNEL_CFG_CATEGORY_EXTEND, count);
#define BatchSetDeviceCategoryMacro(out_type, begin, count) \
    BatchSetDeviceConfig(logger, out_type, DEVICE_CFG_CATEGORY, begin); \
    BatchSetDeviceConfig(logger, out_type, DEVICE_CFG_CATEGORY_EXTEND, count);


    inline void SetAllChannelCategory(Logger& logger, int begin, int count) { BatchSetChannelCategoryMacro(begin, count);}
    inline void SetAllFilePriority(Logger& logger, int begin, int count) { BatchSetDeviceCategoryMacro(DEVICE_OUT_FILE, begin, count); }
    inline void SetAllScreenCategory(Logger& logger, int begin, int count) { BatchSetDeviceCategoryMacro(DEVICE_OUT_SCREEN, begin, count); }
    inline void SetAllUDPCategory(Logger& logger, int begin, int count) { BatchSetDeviceCategoryMacro(DEVICE_OUT_UDP, begin, count); }

    inline void SetAllFileLimitSize(Logger& logger, int limit) { BatchSetDeviceConfig(logger, DEVICE_OUT_FILE, DEVICE_CFG_FILE_LIMIT_SIZE, limit); }
    inline void SetAllFileRollbackCount(Logger& logger, int rb_count) { BatchSetDeviceConfig(logger, DEVICE_OUT_FILE, DEVICE_CFG_FILE_ROLLBACK, rb_count); }

}

#endif
