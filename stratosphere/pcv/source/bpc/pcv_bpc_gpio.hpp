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

namespace ams::pcv::bpc::gpio {

    struct Max77620GpioEntry {
        u32 index;
        const char *name;

        u8 gpio_index;
        u8 gpio_cfg_reg;

        u8 bit_mask;

        u8 unk13;
        u8 unk14;
        u8 unk15;

        u8 opened;
        u8 fps_configured;
        u8 fps_reg;

        u32 fps_pu_period;
        u32 fps_pd_period;
        u32 fps_src;

        i2c::I2cSession *session;
    };

    void InitializeDevices();

}
