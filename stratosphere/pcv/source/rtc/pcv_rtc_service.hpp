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
#include "pcv_rtc_interface.hpp"

namespace ams::pcv::rtc {

    class RtcService {
        public:
            Result GetTimeInSeconds(sf::Out<u64> out, u32 device_code);
            Result SetTimeInSeconds(u32 device_code, u64 seconds);
            Result SetResetOnShutdown(u32 device_code, bool do_reset);
            Result GetResetDetected(sf::Out<u8> out, u32 device_code);
            Result ClearResetDetected(u32 device_code);
            Result EnableAlarm(u32 device_code, u32 alarm_id, u64 alarm_time);
            Result DisableAlarm(u32 device_code, u32 alarm_id);
    };
    static_assert(IsIRtcManager<RtcService>);

}