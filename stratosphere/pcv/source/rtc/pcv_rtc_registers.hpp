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

    /* MAX77620 RTC registers. */
    constexpr inline u8 Max77620RtcIntReg      = 0x00;
    constexpr inline u8 Max77620RtcIntMReg     = 0x01;
    constexpr inline u8 Max77620RtcControlMReg = 0x02;
    constexpr inline u8 Max77620RtcControlReg  = 0x03;
    constexpr inline u8 Max77620RtcUpdate0Reg  = 0x04;
    constexpr inline u8 Max77620RtcUpdate1Reg  = 0x05;
    constexpr inline u8 Max77620RtcSmplReg     = 0x06;

    /* Time registers */
    constexpr inline u8 Max77620RtcSecReg        = 0x07;
    constexpr inline u8 Max77620RtcMinReg        = 0x08;
    constexpr inline u8 Max77620RtcHourReg       = 0x09;
    constexpr inline u8 Max77620RtcWeekdayReg    = 0x0A;
    constexpr inline u8 Max77620RtcMonthReg      = 0x0B;
    constexpr inline u8 Max77620RtcYearReg       = 0x0C;
    constexpr inline u8 Max77620RtcDayReg        = 0x0D;
    constexpr inline int Max77620RtcTimeRegCount = 7;

    /* Alarm registers */
    constexpr inline u8 Max77620RtcAlarm1Reg  = 0x0E;
    constexpr inline u8 Max77620RtcAlarm2Reg  = 0x15;
    constexpr inline u8 Max77620RtcAlarm1Mask = BIT(1);
    constexpr inline u8 Max77620RtcAlarm2Mask = BIT(2);

    /* Alarm field value masks. */
    constexpr inline u8 Max77620RtcSecValueMask     = 0x7F;
    constexpr inline u8 Max77620RtcMinValueMask     = 0x7F;
    constexpr inline u8 Max77620RtcHourValueMask    = 0x3F;
    constexpr inline u8 Max77620RtcWeekdayValueMask = 0x7F;
    constexpr inline u8 Max77620RtcMonthValueMask   = 0x1F;
    constexpr inline u8 Max77620RtcYearValueMask    = 0x7F;
    constexpr inline u8 Max77620RtcDayValueMask     = 0x3F;

    /* RTCCNTL bits. */
    constexpr inline u8 Max77620RtcBinFormat = BIT(0);
    constexpr inline u8 Max77620Rtc24h       = BIT(1);

    constexpr inline u8 Max77620RtcControlBcd12h = 0;

    /* RTCCNTLM bits. */
    constexpr inline u8 Max77620RtcControlMBinFormat = BIT(0);
    constexpr inline u8 Max77620RtcControlM24h       = BIT(1);
    constexpr inline u8 Max77620RtcControlMUnmaskAll = 0;

    /* RTCINTM values. */
    constexpr inline u8 Max77620RtcIntMaskAll = 0xFF;

    /* RTCUPDATE0 bits. */
    constexpr inline u8 Max77620RtcWriteUpdate      = BIT(0);
    constexpr inline u8 Max77620RtcReadUpdate       = BIT(4);
    constexpr inline u8 Max77620RtcUpdate1ReadReady = 0x02;

    /* Enable bits. */
    constexpr inline u8 Max77620RtcAlarmEnMask = BIT(7);
    constexpr inline u8 Max77620RtcHourPmMask  = BIT(6);

    constexpr inline u8 Max77620RegCnfg1_32K       = 0x03;
    constexpr inline u8 Max77620Cnfg1_PwrMd32_Mask = 0x3;
    constexpr inline u8 Max77620Cnfg1_32K_Out0_En  = BIT(2);
    constexpr inline u8 Max77620Cnfg1_32K_LoadMask = 0x30;
    constexpr inline u8 Max77620Cnfg1_32K_OK       = BIT(7);

    /* Interrupt masks (unknown) */
    constexpr inline u8 Max77620RtcIrqBit0Mask = BIT(0);
    constexpr inline u8 Max77620RtcIrqBit1Mask = BIT(1);
    constexpr inline u8 Max77620RtcIrqBit2Mask = BIT(2);
    constexpr inline u8 Max77620RtcIrqBit4Mask = BIT(4);

    /* Interrupt flags (unknown) */
    constexpr inline u32 Max77620RtcFlagFromBit1 = BIT(0);
    constexpr inline u32 Max77620RtcFlagFromBit4 = BIT(1);
    constexpr inline u32 Max77620RtcFlagFromBit0 = BIT(2);
    constexpr inline u32 Max77620RtcFlagFromBit2 = BIT(3);

}
