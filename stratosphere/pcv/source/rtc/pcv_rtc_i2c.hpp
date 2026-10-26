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

    Result InitializeI2cSessions(i2c::I2cSession &pmic_session, i2c::I2cSession &rtc_session);
    Result I2cWrite(i2c::I2cSession &session, u8 reg, u8 value);
    Result I2cRead(i2c::I2cSession &session, u8 *dst, size_t dst_size, const u8 *cmd, size_t cmd_size);
    Result I2cReadUntilFlagClear(i2c::I2cSession &session, u8 &out, u8 cmd, u8 flag);

}