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

namespace ams::pcv::rtc {

    constexpr inline u8 Max77620RtcControlReg = 0x3;
    constexpr inline u8 Max77620RtcUpdate0Reg = 0x4;
    constexpr inline u8 Max77620RtcReadUpdate = 1 << 4;

    constexpr inline u8 Max77620Rtc24h        = BIT(1);

    constexpr inline u8 Max77620RegCnfg1_32K       = 0x03;
    constexpr inline u8 Max77620Cnfg1_PwrMd32_Mask = 0x3;
    constexpr inline u8 Max77620Cnfg1_32K_Out0_En  = BIT(2);
    constexpr inline u8 Max77620Cnfg1_32K_LoadMask = 0x30;
    constexpr inline u8 Max77620Cnfg1_32K_OK       = BIT(7);

}
