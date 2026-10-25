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

namespace ams::pcv::rtc {

    namespace {

        constinit i2c::I2cSession pmic_session;
        constinit i2c::I2cSession rtc_session;

        bool GetExternalRtcResetFlag() {
            bool flag = false;
            R_ABORT_UNLESS(::setsysGetExternalRtcResetFlag(std::addressof(flag)));

            return flag;
        }

        void SetExternalRtcResetFlag(bool flag) {
            R_ABORT_UNLESS(::setsysSetExternalRtcResetFlag(flag));
        }

        Result WaitReadUpdateCleared() {
            constexpr u32 MaxAttempts = 4;
            u8 out = 0;
            u8 cmd = Max77620RtcUpdate0Reg;

            for (u32 i = 0; i < MaxAttempts; ++i) {
                svc::SleepThread(16000000);
                R_TRY(I2cRead(rtc_session, std::addressof(out), sizeof(out), std::addressof(cmd), sizeof(cmd)));

                /* Check if the flag is cleared. */
                if ((out & Max77620RtcReadUpdate) == 0) {
                    break;
                }
            }

            R_UNLESS((out & Max77620RtcReadUpdate) == 0, ResultRtcReadUpdateTimeout());

            return ResultSuccess();
        }

    }

    Result Initialize() {
        /* Check if we can trust the time */
        if (!GetExternalRtcResetFlag()) {
            /* Initialize pmic and rtc i2c sessions */
            R_ABORT_UNLESS(InitializeI2cSessions(pmic_session, rtc_session));

            /* Send a read update */
            R_ABORT_UNLESS(I2cWrite(rtc_session, Max77620RtcUpdate0Reg, Max77620RtcReadUpdate));

            /* Wait until the flag gets cleared. */
            R_ABORT_UNLESS(WaitReadUpdateCleared());

            /* Read the control register. */
            u8 rtc_control_reg = 0;
            {
                u8 cmd = Max77620RtcControlReg;
                R_ABORT_UNLESS(I2cRead(rtc_session, std::addressof(rtc_control_reg), sizeof(rtc_control_reg), std::addressof(cmd), sizeof(cmd)));
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

        /* Get the pmic config of the 32khz oscillator. */
        u8 oscil_config = 0;
        {
            u8 cmd = Max77620RegCnfg1_32K;
            R_ABORT_UNLESS(I2cRead(pmic_session, std::addressof(oscil_config), sizeof(oscil_config), std::addressof(cmd), sizeof(cmd)));
        }

        /* Set the rate to 32khz. */
        oscil_config &= ~(Max77620Cnfg1_PwrMd32_Mask | Max77620Cnfg1_32K_LoadMask);
        oscil_config |= Max77620Cnfg1_32K_LoadMask;

        R_ABORT_UNLESS(I2cWrite(pmic_session, Max77620RegCnfg1_32K, oscil_config));
        svc::SleepThread(500000);

        /* todo pmic and alarm stuff. */

        return ResultSuccess();
    }

}
