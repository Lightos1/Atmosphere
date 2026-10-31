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
#include "pcv_bpc_interface.hpp"

namespace ams::pcv::bpc {

    class BpcService {
        public:
            void ShutdownSystem();
            void RebootSystem();
            Result GetWakeupReason(sf::Out<u32> out_wakeup_reason);
            Result GetShutdownReason(sf::Out<u32> out_shutdown_reason);
            Result GetAcOk(sf::Out<bool> out_ok);
            Result GetPowerEvent(sf::OutCopyHandle out_event, u32 target);
            Result GetPowerButtonEvent(sf::OutCopyHandle out_event, u32 target);
            Result GetPowerButton(sf::Out<bool> out_is_pushed);
            void SetUpRtcResetOnShutdown();

    };
    static_assert(IsIBpcManager<BpcService>);

}
