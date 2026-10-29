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

    /* Max77620 PMIC registers */
    constexpr inline u8 Max77620PmicCnfgGlbl1Reg   = 0x00;
    constexpr inline u8 Max77620PmicCnfgGlbl2Reg   = 0x01;
    constexpr inline u8 Max77620PmicCnfgGlbl3Reg   = 0x02;
    constexpr inline u8 Max77620PmicCnfg132kReg    = 0x03;
    constexpr inline u8 Max77620PmicCnfgBbcReg     = 0x04;
    constexpr inline u8 Max77620PmicIrqTopReg      = 0x05;
    constexpr inline u8 Max77620PmicIntLbtReg      = 0x06;
    constexpr inline u8 Max77620PmicIrqSdReg       = 0x07;
    constexpr inline u8 Max77620PmicIrqLvl2L07Reg  = 0x08;
    constexpr inline u8 Max77620PmicIrqLvl2L8Reg   = 0x09;
    constexpr inline u8 Max77620PmicIrqLvl2GpioReg = 0x0A;
    constexpr inline u8 Max77620PmicOnOffIrqReg    = 0x0B;
    constexpr inline u8 Max77620PmicNvercReg       = 0x0C;
    constexpr inline u8 Max77620PmicIrqTopMReg     = 0x0D;
    constexpr inline u8 Max77620PmicIntEnLbtReg    = 0x0E;
    constexpr inline u8 Max77620PmicIrqMaskSdReg   = 0x0F;
    constexpr inline u8 Max77620PmicIrqMskL07Reg   = 0x10;
    constexpr inline u8 Max77620PmicIrqMskL8Reg    = 0x11;
    constexpr inline u8 Max77620PmicOnOffIrqMReg   = 0x12;
    constexpr inline u8 Max77620PmicStatLbtReg     = 0x13;
    constexpr inline u8 Max77620PmicStatSdReg      = 0x14;
    constexpr inline u8 Max77620PmicOnOffStatReg   = 0x15;

    /* Step-down regulators. */
    constexpr inline u8 Max77620PmicSd0Reg         = 0x16;
    constexpr inline u8 Max77620PmicSd1Reg         = 0x17;
    constexpr inline u8 Max77620PmicSd2Reg         = 0x18;
    constexpr inline u8 Max77620PmicSd3Reg         = 0x19;
    constexpr inline u8 Max77620PmicSd4Reg         = 0x1A;
    constexpr inline u8 Max77620PmicDvsSd0Reg      = 0x1B;
    constexpr inline u8 Max77620PmicDvsSd1Reg      = 0x1C;
    constexpr inline u8 Max77620PmicSd0CfgReg      = 0x1D;
    constexpr inline u8 Max77620PmicSd1CfgReg      = 0x1E;
    constexpr inline u8 Max77620PmicSd2CfgReg      = 0x1F;
    constexpr inline u8 Max77620PmicSd3CfgReg      = 0x20;
    constexpr inline u8 Max77620PmicSd4CfgReg      = 0x21;
    constexpr inline u8 Max77620PmicSdCfg2Reg      = 0x22;

    /* LDO regulators. */
    constexpr inline u8 Max77620PmicLdo0CfgReg     = 0x23;
    constexpr inline u8 Max77620PmicLdo0Cfg2Reg    = 0x24;
    constexpr inline u8 Max77620PmicLdo1CfgReg     = 0x25;
    constexpr inline u8 Max77620PmicLdo1Cfg2Reg    = 0x26;
    constexpr inline u8 Max77620PmicLdo2CfgReg     = 0x27;
    constexpr inline u8 Max77620PmicLdo2Cfg2Reg    = 0x28;
    constexpr inline u8 Max77620PmicLdo3CfgReg     = 0x29;
    constexpr inline u8 Max77620PmicLdo3Cfg2Reg    = 0x2A;
    constexpr inline u8 Max77620PmicLdo4CfgReg     = 0x2B;
    constexpr inline u8 Max77620PmicLdo4Cfg2Reg    = 0x2C;
    constexpr inline u8 Max77620PmicLdo5CfgReg     = 0x2D;
    constexpr inline u8 Max77620PmicLdo5Cfg2Reg    = 0x2E;
    constexpr inline u8 Max77620PmicLdo6CfgReg     = 0x2F;
    constexpr inline u8 Max77620PmicLdo6Cfg2Reg    = 0x30;
    constexpr inline u8 Max77620PmicLdo7CfgReg     = 0x31;
    constexpr inline u8 Max77620PmicLdo7Cfg2Reg    = 0x32;
    constexpr inline u8 Max77620PmicLdo8CfgReg     = 0x33;
    constexpr inline u8 Max77620PmicLdo8Cfg2Reg    = 0x34;
    constexpr inline u8 Max77620PmicLdoCfg3Reg     = 0x35;

    /* GPIO. */
    constexpr inline u8 Max77620PmicGpio0Reg       = 0x36;
    constexpr inline u8 Max77620PmicGpio1Reg       = 0x37;
    constexpr inline u8 Max77620PmicGpio2Reg       = 0x38;
    constexpr inline u8 Max77620PmicGpio3Reg       = 0x39;
    constexpr inline u8 Max77620PmicGpio4Reg       = 0x3A;
    constexpr inline u8 Max77620PmicGpio5Reg       = 0x3B;
    constexpr inline u8 Max77620PmicGpio6Reg       = 0x3C;
    constexpr inline u8 Max77620PmicGpio7Reg       = 0x3D;
    constexpr inline u8 Max77620PmicPueGpioReg     = 0x3E;
    constexpr inline u8 Max77620PmicPdeGpioReg     = 0x3F;
    constexpr inline u8 Max77620PmicAmeGpioReg     = 0x40;

    /* ON/OFF controller. */
    constexpr inline u8 Max77620PmicOnOffCnfg1Reg  = 0x41;
    constexpr inline u8 Max77620PmicOnOffCnfg2Reg  = 0x42;

    /* Flexible power sequencer. */
    constexpr inline u8 Max77620PmicFpsCfg0Reg     = 0x43;
    constexpr inline u8 Max77620PmicFpsCfg1Reg     = 0x44;
    constexpr inline u8 Max77620PmicFpsCfg2Reg     = 0x45;
    constexpr inline u8 Max77620PmicFpsLdo0Reg     = 0x46;
    constexpr inline u8 Max77620PmicFpsLdo1Reg     = 0x47;
    constexpr inline u8 Max77620PmicFpsLdo2Reg     = 0x48;
    constexpr inline u8 Max77620PmicFpsLdo3Reg     = 0x49;
    constexpr inline u8 Max77620PmicFpsLdo4Reg     = 0x4A;
    constexpr inline u8 Max77620PmicFpsLdo5Reg     = 0x4B;
    constexpr inline u8 Max77620PmicFpsLdo6Reg     = 0x4C;
    constexpr inline u8 Max77620PmicFpsLdo7Reg     = 0x4D;
    constexpr inline u8 Max77620PmicFpsLdo8Reg     = 0x4E;
    constexpr inline u8 Max77620PmicFpsSd0Reg      = 0x4F;
    constexpr inline u8 Max77620PmicFpsSd1Reg      = 0x50;
    constexpr inline u8 Max77620PmicFpsSd2Reg      = 0x51;
    constexpr inline u8 Max77620PmicFpsSd3Reg      = 0x52;
    constexpr inline u8 Max77620PmicFpsSd4Reg      = 0x53;
    constexpr inline u8 Max77620PmicFpsGpio1Reg    = 0x54;
    constexpr inline u8 Max77620PmicFpsGpio2Reg    = 0x55;
    constexpr inline u8 Max77620PmicFpsGpio3Reg    = 0x56;
    constexpr inline u8 Max77620PmicFpsRsoReg      = 0x57;

    /* Chip ID. */
    constexpr inline u8 Max77620PmicCid0Reg        = 0x58;
    constexpr inline u8 Max77620PmicCid1Reg        = 0x59;
    constexpr inline u8 Max77620PmicCid2Reg        = 0x5A;
    constexpr inline u8 Max77620PmicCid3Reg        = 0x5B;
    constexpr inline u8 Max77620PmicCid4Reg        = 0x5C;
    constexpr inline u8 Max77620PmicCid5Reg        = 0x5D;

    constexpr inline u8 Max77620PmicDvsSd4Reg      = 0x5E;

    /* IRQ masks. */
    constexpr inline u8 Max77620PmicIrqTopGlblMask  = BIT(7);
    constexpr inline u8 Max77620PmicIrqTopSdMask    = BIT(6);
    constexpr inline u8 Max77620PmicIrqTopLdoMask   = BIT(5);
    constexpr inline u8 Max77620PmicIrqTopGpioMask  = BIT(4);
    constexpr inline u8 Max77620PmicIrqTopRtcMask   = BIT(3);
    constexpr inline u8 Max77620PmicIrqTop32kMask   = BIT(2);
    constexpr inline u8 Max77620PmicIrqTopOnOffMask = BIT(1);

    /* ONOFFIRQ/ONOFFIRQM bits (power button, lid, AC adapter). */
    constexpr inline u8 Max77620PmicOnOffIrqMrWrnMask    = BIT(0); /* Manual reset warning. */
    constexpr inline u8 Max77620PmicOnOffIrqEn01SecMask  = BIT(1);
    constexpr inline u8 Max77620PmicOnOffIrqEn0FallMask  = BIT(2); /* Power button edge. */
    constexpr inline u8 Max77620PmicOnOffIrqEn0RiseMask  = BIT(3); /* Power button edge. */
    constexpr inline u8 Max77620PmicOnOffIrqLidFallMask  = BIT(4);
    constexpr inline u8 Max77620PmicOnOffIrqLidRiseMask  = BIT(5);
    constexpr inline u8 Max77620PmicOnOffIrqAcOkFallMask = BIT(6); /* AC adapter removed. */
    constexpr inline u8 Max77620PmicOnOffIrqAcOkRiseMask = BIT(7); /* AC adapter inserted. */

    /* Edges that signal the power button event. */
    constexpr inline u8 Max77620PmicOnOffIrqPowerButtonMask = Max77620PmicOnOffIrqEn0FallMask | Max77620PmicOnOffIrqEn0RiseMask;

    /* ONOFFSTAT bits. */
    constexpr inline u8 Max77620PmicOnOffStatLidMask  = BIT(0);
    constexpr inline u8 Max77620PmicOnOffStatAcOkMask = BIT(1);
    constexpr inline u8 Max77620PmicOnOffStatEn0Mask  = BIT(2); /* Power button held. */

    /* Interrupts left unmasked while asleep */
    constexpr inline u8 Max77620PmicIrqTopSleepWakeMask   = Max77620PmicIrqTopGlblMask | Max77620PmicIrqTopRtcMask | Max77620PmicIrqTopOnOffMask;
    constexpr inline u8 Max77620PmicOnOffIrqSleepWakeMask = Max77620PmicOnOffIrqEn0RiseMask | Max77620PmicOnOffIrqAcOkFallMask | Max77620PmicOnOffIrqAcOkRiseMask;
    constexpr inline u8 Max77620PmicIrqTopSleepMask       = static_cast<u8>(~Max77620PmicIrqTopSleepWakeMask);
    constexpr inline u8 Max77620PmicOnOffIrqSleepMask     = static_cast<u8>(~Max77620PmicOnOffIrqSleepWakeMask);

    /* Wakeup masks */
    constexpr inline u8 Max77620PmicWakeupReasonRtcMask      = BIT(2); /* IRQTOP RTC. */
    constexpr inline u8 Max77620PmicWakeupReasonEn0RiseMask  = BIT(3); /* ONOFFIRQ EN0 rise. */
    constexpr inline u8 Max77620PmicWakeupReasonAcOkFallMask = BIT(4); /* ONOFFIRQ AC-OK fall. */
    constexpr inline u8 Max77620PmicWakeupReasonAcOkRiseMask = BIT(5); /* ONOFFIRQ AC-OK rise. */

    /* INTLBT/INTENLBT bits (low battery, thermal alarms, global mask). */
    constexpr inline u8 Max77620PmicIrqLbmMask      = BIT(3);
    constexpr inline u8 Max77620PmicIrqTjAlrm1Mask  = BIT(2);
    constexpr inline u8 Max77620PmicIrqTjAlrm2Mask  = BIT(1);
    constexpr inline u8 Max77620PmicGlblMMask       = BIT(0);

    /* NVERC bits. (reset/shutdown cause) */
    /* Unsure if these are correct. */
    constexpr inline u8 Max77620PmicNvercShutdownPinMask  = BIT(0);
    constexpr inline u8 Max77620PmicNvercSysWatchdogMask  = BIT(1);
    constexpr inline u8 Max77620PmicNvercHardResetMask    = BIT(2);
    constexpr inline u8 Max77620PmicNvercJunctionTempMask = BIT(3);
    constexpr inline u8 Max77620PmicNvercBatteryLowMask   = BIT(4);
    constexpr inline u8 Max77620PmicNvercBatteryOvloMask  = BIT(5);
    constexpr inline u8 Max77620PmicNvercBatteryUvloMask  = BIT(6);
    constexpr inline u8 Max77620PmicNvercResetInputMask   = BIT(7);

    /* Hardware protection events + reset input? */
    constexpr inline u8 NvercEnvironmentShutdownMask = Max77620PmicNvercJunctionTempMask | Max77620PmicNvercBatteryOvloMask | Max77620PmicNvercBatteryUvloMask | Max77620PmicNvercResetInputMask;
    /* User/external requested power-off? */
    constexpr inline u8 NvercUserShutdownMask        = Max77620PmicNvercShutdownPinMask | Max77620PmicNvercHardResetMask;

    /* Thermal flag alarms. */
    constexpr inline u8 Max77620PmicThermalFlagAlarm1 = BIT(0);
    constexpr inline u8 Max77620PmicThermalFlagAlarm2 = BIT(1);

}
