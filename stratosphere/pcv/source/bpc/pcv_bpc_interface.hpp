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

#define AMS_PCV_I_BPC_MANAGER_INTERFACE_INFO(C, H)                                                                               \
    AMS_SF_METHOD_INFO(C, H,  0, void,   ShutdownSystem,          (),                                       ())                  \
    AMS_SF_METHOD_INFO(C, H,  1, void,   RebootSystem,            (),                                       ())                  \
    AMS_SF_METHOD_INFO(C, H,  2, Result, GetWakeupReason,         (sf::Out<u32> out_wakeup_reason),         (out_wakeup_reason)) \
    AMS_SF_METHOD_INFO(C, H,  3, Result, GetShutdownReason,       (sf::Out<u32> out_shutdown_reason),       (out_shutdown_reason)) \
    AMS_SF_METHOD_INFO(C, H,  4, Result, GetAcOk,                 (sf::Out<bool> out_ok),                   (out_ok))            \
    AMS_SF_METHOD_INFO(C, H,  5, Result, GetPowerEvent,           (sf::OutCopyHandle out_event, u32 target), (out_event, target)) \
    AMS_SF_METHOD_INFO(C, H,  7, Result, GetPowerButtonEvent,     (sf::OutCopyHandle out_event, u32 target), (out_event, target)) \
    AMS_SF_METHOD_INFO(C, H, 14, Result, GetPowerButton,          (sf::Out<bool> out_is_pushed),            (out_is_pushed))     \
    AMS_SF_METHOD_INFO(C, H, 15, void,   SetUpRtcResetOnShutdown, (),                                       ())

AMS_SF_DEFINE_INTERFACE(ams::pcv::bpc, IBpcManager, AMS_PCV_I_BPC_MANAGER_INTERFACE_INFO, 0x49627063)
