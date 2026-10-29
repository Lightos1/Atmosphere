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
#include "pcv_rtc_i2c.hpp"

namespace ams::pcv::rtc {

    Result I2cReadUntilFlagClear(i2c::I2cSession &session, u8 reg, u8 &out, u8 flag) {
        constexpr u32 MaxAttempts = 4;

        for (u32 i = 0; i < MaxAttempts; ++i) {
            svc::SleepThread(16000000);
            R_TRY(I2cRead(session, reg, out));

            if ((out & flag) == 0) {
                R_SUCCEED();
            }
        }

        R_THROW(ResultRtcAccessFailed());
    }

}
