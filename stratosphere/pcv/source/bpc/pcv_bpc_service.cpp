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
#include "pcv_bpc_service.hpp"
#include "pcv_bpc.hpp"

namespace ams::pcv::bpc  {

    void BpcService::ShutdownSystem() {
        GetPowerManager().ShutdownSystem();
    }

    void BpcService::RebootSystem() {
        GetPowerManager().RebootSystem();
    }

    Result BpcService::GetWakeupReason(sf::Out<u32> out_wakeup_reason) {
        GetPowerManager().GetWakeupReason(out_wakeup_reason.GetPointer());
        R_SUCCEED();
    }

    Result BpcService::GetShutdownReason(sf::Out<u32> out_shutdown_reason) {
        GetPowerManager().GetShutdownReason(out_shutdown_reason.GetPointer());
        R_SUCCEED();
    }

    Result BpcService::GetAcOk(sf::Out<bool> out_ok) {
        R_RETURN(GetPowerManager().GetAcOk(out_ok.GetPointer()));
    }

    Result BpcService::GetPowerEvent(sf::OutCopyHandle out_event, u32 target) {
        os::SystemEventType *event = nullptr;
        R_TRY(GetPowerManager().GetPowerEvent(std::addressof(event), target));

        out_event.SetValue(os::GetReadableHandleOfSystemEvent(event), false);
        R_SUCCEED();
    }

    Result BpcService::GetPowerButtonEvent(sf::OutCopyHandle out_event, u32 target) {
        os::SystemEventType *event = nullptr;
        R_TRY(GetPowerManager().GetPowerButtonEvent(std::addressof(event), target));

        out_event.SetValue(os::GetReadableHandleOfSystemEvent(event), false);
        R_SUCCEED();
    }

    Result BpcService::GetPowerButton(sf::Out<bool> out_is_pushed) {
        R_RETURN(GetPowerManager().GetPowerButton(out_is_pushed.GetPointer()));
    }

    void BpcService::SetUpRtcResetOnShutdown() {
        GetPowerManager().SetUpRtcResetOnShutdown();
    }

}
