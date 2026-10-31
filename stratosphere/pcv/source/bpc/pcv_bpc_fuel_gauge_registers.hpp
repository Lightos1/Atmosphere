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

namespace ams::pcv::bpc {

    /* MAX17050 registers. */
    constexpr inline u8 Max17050SocRepReg         = 0x06;
    constexpr inline u8 Max17050ConfigReg         = 0x1D;
    constexpr inline u8 Max17050ShdnTimerReg      = 0x3F;
    constexpr inline u16 Max17050ShdnTimerThr7    = 7 << 13;
    constexpr inline u16 Max17050ConfigI2cSh      = 0x0040;
    constexpr inline u8 Max17050HighChargePercent = 80;

}
