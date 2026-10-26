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

namespace ams::pcv::rtc {

    namespace {
        constexpr i2c::TransactionOption StartStop = static_cast<i2c::TransactionOption>(i2c::TransactionOption_StartCondition | i2c::TransactionOption_StopCondition);
    }

    Result InitializeI2cSessions(i2c::I2cSession &pmic_session, i2c::I2cSession &rtc_session) {
        R_TRY(i2c::OpenSession(std::addressof(pmic_session), i2c::DeviceCode_Max77620Pmic));
        R_TRY(i2c::OpenSession(std::addressof(rtc_session),  i2c::DeviceCode_Max77620Rtc));

        return ResultSuccess();
    }

    Result I2cWrite(i2c::I2cSession &session, u8 reg, u8 value) {
        const u8 buf[2] = { reg, value };

        R_RETURN(i2c::Send(session, buf, sizeof(buf), StartStop));
    }

    Result I2cRead(i2c::I2cSession &session, u8 *dst, size_t dst_size, const u8 *cmd, size_t cmd_size) {
        AMS_ABORT_UNLESS(dst != nullptr && dst_size > 0);
        AMS_ABORT_UNLESS(cmd != nullptr && cmd_size > 0);

        u8 cmd_list[i2c::CommandListLengthMax];
        i2c::CommandListFormatter formatter(cmd_list, sizeof(cmd_list));

        R_ABORT_UNLESS(formatter.EnqueueSendCommand(i2c::TransactionOption_StartCondition, cmd, cmd_size));
        R_ABORT_UNLESS(formatter.EnqueueReceiveCommand(StartStop, dst_size));

        R_RETURN(i2c::ExecuteCommandList(dst, dst_size, session, cmd_list, formatter.GetCurrentLength()));
    }

    Result I2cReadUntilFlagClear(i2c::I2cSession &session, u8 &out, u8 cmd, u8 flag) {
        constexpr u32 MaxAttempts = 4;

        for (u32 i = 0; i < MaxAttempts; ++i) {
            svc::SleepThread(16000000);
            R_TRY(I2cRead(session, std::addressof(out), sizeof(out), std::addressof(cmd), sizeof(cmd)));

            if ((out & flag) == 0) {
                R_SUCCEED();
            }
        }

        R_THROW(ResultRtcAccessFailed());
    }

}
