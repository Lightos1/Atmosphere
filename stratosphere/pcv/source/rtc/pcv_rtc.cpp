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
#include "pcv_rtc_registers.hpp"
#include "pcv_rtc_i2c.hpp"
#include "pcv_rtc_device.hpp"
#include "pcv_rtc_reset_flag.hpp"

namespace ams::pcv::rtc {

    namespace {

        constinit i2c::I2cSession g_pmic_session;
        constinit i2c::I2cSession g_rtc_session;

        constinit RtcDevice g_rtc_device(g_rtc_session);

        void CheckExternalRtcReset() {
            /* Check if rtc was reset. */
            if (GetExternalRtcResetFlag()) {
                return;
            }

            /* Send a read update */
            R_ABORT_UNLESS(I2cWrite(g_rtc_session, Max77620RtcUpdate0Reg, Max77620RtcReadUpdate));

            /* Wait until the flag gets cleared. */
            u8 out_value = 0;
            R_ABORT_UNLESS(I2cReadUntilFlagClear(g_rtc_session, Max77620RtcUpdate0Reg, out_value, Max77620RtcReadUpdate));

            /* Read the control register. */
            u8 rtc_control_reg = 0;
            {
                R_ABORT_UNLESS(I2cRead(g_rtc_session, Max77620RtcControlReg, rtc_control_reg));
            }

            /* Nintendo assumes that the control reg is set to MAX77620_RTC_24H. */
            /* If it's set to anything else, they assume it must have been reset. */
            /* This is silly, should we do something different? */
            if ((rtc_control_reg & Max77620Rtc24h) != Max77620Rtc24h) {
                SetExternalRtcResetFlag(true);

                /* They then get the flag and ignore the result. Why. */
                GetExternalRtcResetFlag();
            }
        }

        void ConfigureOscillator() {
            /* Get the pmic config of the 32khz oscillator. */
            u8 oscil_config = 0;
            {
                R_ABORT_UNLESS(I2cRead(g_pmic_session, Max77620RegCnfg1_32K, oscil_config));
            }

            /* Set the rate to 32khz. */
            oscil_config &= ~(Max77620Cnfg1_PwrMd32_Mask | Max77620Cnfg1_32K_LoadMask);
            oscil_config |= Max77620Cnfg1_32K_LoadMask;

            /* Write the config */
            R_ABORT_UNLESS(I2cWrite(g_pmic_session, Max77620RegCnfg1_32K, oscil_config));
            svc::SleepThread(500000);
        }

    }

    Result Initialize() {
        /* Initialize pmic and rtc i2c sessions */
        R_ABORT_UNLESS(i2c::OpenSession(std::addressof(g_pmic_session), i2c::DeviceCode_Max77620Pmic));
        R_ABORT_UNLESS(i2c::OpenSession(std::addressof(g_rtc_session),  i2c::DeviceCode_Max77620Rtc));

        /* Check if we can trust the time */
        CheckExternalRtcReset();

        /* Configure the 32khz oscillator. */
        ConfigureOscillator();

        /* Initialize the rtc device. */
        R_ABORT_UNLESS(g_rtc_device.Initialize());

        /* Disable both alarms. */
        R_ABORT_UNLESS(g_rtc_device.DisableAlarm(0));
        R_ABORT_UNLESS(g_rtc_device.DisableAlarm(1));

        /* NOTE: Nintendo does more linked list linking crap here. */
        /* But this is *kinda* not needed. */

        return ResultSuccess();
    }

    RtcDevice &GetRtcDevice() {
        return g_rtc_device;
    }

}
