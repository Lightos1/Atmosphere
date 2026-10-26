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

#include <stratosphere.hpp>
#include "pcv_rtc_service.hpp"
#include "pcv_rtc_driver.hpp"

namespace ams::pcv::rtc {

    /* The device code is being ignored here for one reason: it's completely reduntant. */
    /* Nintendo uses linked lists to register potentially multiple rtc devices. */
    /* The device code is then gotten to determine which node to use. */
    /* But they only ever register a single device, making this completely reduntant and just a hassle to implement. */
    Result RtcService::GetTimeInSeconds(sf::Out<u64> out, u32 device_code) {
        AMS_UNUSED(device_code);

        R_RETURN(GetRtcDevice().GetTimeInSeconds(out.GetPointer()));
    }

    Result RtcService::SetTimeInSeconds(u32 device_code, u64 seconds) {
        AMS_UNUSED(device_code);

        R_RETURN(GetRtcDevice().SetTimeInSeconds(seconds));
    }

    Result RtcService::SetResetOnShutdown(u32 device_code, bool do_reset) {
        AMS_UNUSED(device_code);

        R_RETURN(GetRtcDevice().SetResetOnShutdown(do_reset));
    }

    Result RtcService::GetResetDetected(sf::Out<u8> out, u32 device_code) {
        AMS_UNUSED(device_code);

        R_RETURN(GetRtcDevice().GetResetDetected(out.GetPointer()));
    }

    Result RtcService::ClearResetDetected(u32 device_code) {
        AMS_UNUSED(device_code);

        R_RETURN(GetRtcDevice().ClearResetDetected());
    }

    Result RtcService::EnableAlarm(u32 device_code, u32 alarm_id, u64 alarm_time) {
        AMS_UNUSED(device_code);

        R_RETURN(GetRtcDevice().EnableAlarm(alarm_id, alarm_time));
    }

    Result RtcService::DisableAlarm(u32 device_code, u32 alarm_id) {
        AMS_UNUSED(device_code);

        R_RETURN(GetRtcDevice().DisableAlarm(alarm_id));
    }

}
