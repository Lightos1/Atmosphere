/*
 * Copyright (c) Atmosphère-NX
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once
#include <stratosphere.hpp>

#define AMS_PCV_I_RTC_MANAGER_INTERFACE_INFO(C, H)                                                                                                       \
    AMS_SF_METHOD_INFO(C, H, 0,  Result, GetTimeInSeconds,   (sf::Out<u64> out_seconds, u32 device_code),            (out_seconds, device_code))           \
    AMS_SF_METHOD_INFO(C, H, 1,  Result, SetTimeInSeconds,   (u32 device_code, u64 in_seconds),                      (device_code, in_seconds))            \
    AMS_SF_METHOD_INFO(C, H, 2,  Result, SetResetOnShutdown, (u32 device_code, bool do_reset),                       (device_code, do_reset))              \
    AMS_SF_METHOD_INFO(C, H, 3,  Result, GetResetDetected,   (sf::Out<u8> out_reset_detected, u32 device_code),      (out_reset_detected, device_code))    \
    AMS_SF_METHOD_INFO(C, H, 4,  Result, ClearResetDetected, (u32 device_code),                                      (device_code))                        \
    AMS_SF_METHOD_INFO(C, H, 10, Result, EnableAlarm,        (u32 device_code, u32 alarm_id, u64 in_seconds),        (device_code, alarm_id, in_seconds))  \
    AMS_SF_METHOD_INFO(C, H, 11, Result, DisableAlarm,       (u32 device_code, u32 alarm_id),                        (device_code, alarm_id))

AMS_SF_DEFINE_INTERFACE(ams::pcv::rtc, IRtcManager, AMS_PCV_I_RTC_MANAGER_INTERFACE_INFO, 0x49525443)
