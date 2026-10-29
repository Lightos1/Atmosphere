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
#include "pcv_i2c.hpp"

namespace ams::pcv {

    namespace {

        constexpr i2c::TransactionOption StartStop = static_cast<i2c::TransactionOption>(i2c::TransactionOption_StartCondition | i2c::TransactionOption_StopCondition);
        constexpr size_t I2cBufferSize = 32;

    }

    Result I2cWrite(i2c::I2cSession &session, u8 reg, u8 value) {
        const u8 buf[2] = { reg, value };

        R_RETURN(i2c::Send(session, buf, sizeof(buf), StartStop));
    }

    Result I2cRead(i2c::I2cSession &session, u8 reg, u8 &out) {
        u8 cmd_list[i2c::CommandListLengthMax];
        i2c::CommandListFormatter formatter(cmd_list, sizeof(cmd_list));

        R_ABORT_UNLESS(formatter.EnqueueSendCommand(i2c::TransactionOption_StartCondition, std::addressof(reg), sizeof(reg)));
        R_ABORT_UNLESS(formatter.EnqueueReceiveCommand(StartStop, sizeof(out)));

        R_RETURN(i2c::ExecuteCommandList(std::addressof(out), sizeof(out), session, cmd_list, formatter.GetCurrentLength()));
    }

}
